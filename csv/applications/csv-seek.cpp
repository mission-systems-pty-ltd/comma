// Copyright (c) 2024 Mission Systems

/// @author Aspen Eyers

#include <cstdint>
#include <iostream>
#include <fstream>
#include "../../application/command_line_options.h"
#include "../../csv/traits.h"
#include "../../name_value/parser.h"
#include "../../visiting/traits.h"
#include "../../csv/stream.h"
#include "../../csv/traits.h"


static void usage( bool verbose = false )
{
    std::cerr << R"(
seek through a stream to grab selected records
usage: csv-seek <options> [<stream>]
options
    --permissive:          permissive mode: output empty record on error

    --size,-s=<size>:      [todo] data is packets of fixed size, otherwise data is expected
                           line-wise. Alternatively use --binary
csv options
)";
    std::cerr << comma::csv::options::usage( verbose ) << std::endl;
    std::cerr << "examples" << std::endl;
    if( verbose ) { std::cerr << R"(    examples setup
        basics
            make data file
                csv-paste 'line-number;binary=ui' --head 100 > data.bin

            sample the records at 50% and 10% through the data:
                ( echo 0.5; echo 0.1 ) | csv-seek --fields=ratio "data.bin;binary=f" | csv-from-bin f

            sample the 10th record
                echo 10 | csv-seek "data.bin;binary=12f" | csv-from-bin f

        colour hue (you would need snark installed with graphics and imaging enabled)
            make data file
                ( csv-paste value=255 value=0 line-number --head 256; \
                  csv-paste 'line-number;begin=255;step=-1' line-number value=255 --head 256; \
                  csv-paste value=0 value=255 'line-number;begin=255;step=-1' --head 256; \
                  csv-paste line-number value=255 value=0 --head 256; \
                  csv-paste value=255 'line-number;begin=255;step=-1' value=0 --head 256 ) \
                      | csv-to-bin 3ub \
                      > colour-wheel.bin
            sample colour hue
                csv-sliders 'hue;min=0;max=1;step=0.0001' \
                            --frequency 10 \
                            --window-geometry=0,0,400,60 \
                            --title='examples: hue selection' \
                    | csv-seek --permissive --fields ratio 'colour-wheel.bin;binary=3ub' --flush \
                    | cv-cat --input 'rows=1;cols=1;no-header;type=3ub' \
                             'resize=400;view=,examples: hue selection,,0,130;null'
)";
    }
    else
    {
        std::cerr << "    see --help --verbose for more help" << std::endl;
    }
    exit( 0 );
}

namespace comma { namespace csv {
struct config_t
{
    std::string filename;
    std::string format;
};

struct input_t
{
    double ratio{0};
    std::uint32_t index{0};
    std::uint32_t block{0}; // todo in some vague future

    std::uint64_t get_index( std::size_t filesize, std::size_t record_size, bool use_ratio ) const { return use_ratio ? static_cast<std::uint64_t>(filesize * ratio) : index*record_size; }
};

}} // namespace comma { namespace csv {

namespace comma { namespace visiting {

template <> struct traits< comma::csv::config_t >
{
    template < typename K, typename V > static void visit( const K&, comma::csv::config_t& p, V& v )
    {
        v.apply( "filename", p.filename );
        v.apply( "format", p.format );
    }

    template < typename K, typename V > static void visit( const K&, const comma::csv::config_t& p, V& v )
    {
        v.apply( "filename", p.filename );
        v.apply( "format", p.format );
    }
};

template <> struct traits< comma::csv::input_t >
{
    template < typename K, typename V > static void visit( const K&, comma::csv::input_t& p, V& v )
    {
        v.apply( "ratio", p.ratio );
        v.apply( "index", p.index );
        v.apply( "block", p.block );
    }

    template < typename K, typename V > static void visit( const K&, const comma::csv::input_t& p, V& v )
    {
        v.apply( "ratio", p.ratio );
        v.apply( "index", p.index );
        v.apply( "block", p.block );
    }
};

} } // namespace comma { namespace visiting {

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        std::vector< std::string > unnamed = options.unnamed( "--flush,-v,--verbose,--permissive,-p,--size", "-.*" );
        comma::csv::options csv( options, "index" );
        bool permissive = options.exists( "--permissive,-p" );
        COMMA_ASSERT_BRIEF( csv.has_field( "ratio" ) != csv.has_field( "index" ), "please specify either 'ratio' or 'index' (but not both) in --fields" );

        COMMA_ASSERT_BRIEF( unnamed.size() > 0, "expected file (or stream, todo)" );
        COMMA_ASSERT_BRIEF( unnamed.size() < 2, "Does not work on multiple streams (yet (shouuld it?))" );

        comma::name_value::parser csv_options_parser( "filename", ';', '=', false );
        auto stream = csv_options_parser.get< comma::csv::config_t >( unnamed[0] );
        auto stream_csv = csv_options_parser.get< comma::csv::options >( unnamed[0] );
        std::string filename = stream.filename;
        COMMA_ASSERT_BRIEF( filename!="-", "expected filename. file scrubbing does not work on streams." );
        COMMA_ASSERT_BRIEF( stream_csv.binary(), "expected binary file" );

        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        COMMA_ASSERT_BRIEF( file.is_open(), "unable to open file" );

        std::streamsize file_size = file.tellg();
        std::streampos record_size = stream_csv.format().size();

        comma::csv::input_stream< comma::csv::input_t > istream( std::cin, csv );
        while( std::cin.good() && !std::cin.eof() )
        {
            const comma::csv::input_t* p = istream.read();
            if( !p ) { break; }

            std::streampos index = p->get_index( file_size, record_size, csv.has_field( "ratio" ) );
            std::streampos adjusted_offset = (index / record_size) * record_size;

            if (adjusted_offset >= file_size) 
            { 
                comma::saymore() <<  "index out of bounds" << std::endl; 
                if( permissive ) { continue; }
                return 1;
            }
            std::vector<char> record_data;
            file.seekg(adjusted_offset);
            record_data.resize(record_size);
            file.read(record_data.data(), record_size);
            std::cout.write(record_data.data(), record_data.size());
            if( csv.flush ) { std::cout.flush(); }
        }

        file.close();
        return 0;
    }
    catch( std::exception& ex ) { comma::say() << ex.what() << std::endl; }
    catch( ... ) { comma::say() << "unknown exception" << std::endl; }
    return 1;
}
