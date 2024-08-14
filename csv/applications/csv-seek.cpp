#include <iostream>
#include <fstream>
#include "../../application/command_line_options.h"
#include "../../csv/traits.h"
#include "../../name_value/parser.h"
#include "../../visiting/traits.h"
#include "../../csv/stream.h"
#include "../../csv/traits.h"

/// @todo : Handle field name for scrubbing - should i name it selection or target or record or offset or index or grab or seek or cursor or bookmark or needle or marker...?
///         I'm thinking 'offset'
/// @todo: Implement index operation as well as percentage operation. I only have percentage implemented.

static void usage( bool verbose = false )
{
    std::cerr << std::endl;
    std::cerr << "seek through a stream to grab selected records" << std::endl;
    std::cerr << "" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: csv-seek <operation> [<options>] <stream>" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --binary,-b=<format>: data is packets of fixed size given by <format>" << std::endl;
    std::cerr << "                          alternatively use --size" << std::endl;
    std::cerr << "    --size,-s=<size>: data is packets of fixed size, otherwise data is expected" << std::endl;
    std::cerr << "                      line-wise. Alternatively use --binary" << std::endl;
    std::cerr << "    --scrub,-s:          Input is read as a percentage of the data" << std::endl;
    std::cerr << "    --index,-i:          grab the record at index <n> through the data" << std::endl;
    std::cerr << std::endl;
    std::cerr << "csv options" << std::endl;
    std::cerr << comma::csv::options::usage( verbose ) << std::endl;
    std::cerr << "examples setup" << std::endl;
    std::cerr << "      The following examples assume you have some data to work on." << std::endl;
    std::cerr << "      Run this to create some data to look at!" << std::endl;
    std::cerr << "      for i in $( seq 0 1 100  ); do echo $i,$i,$i; done | csv-to-bin 3f > data.bin" << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "      Sample the records at 50% and 10% through the data:" << std::endl;
    std::cerr << "      ( echo 0.5; echo 0.1 ) | csv-seek scrub \"data.bin;binary=3f\" | csv-from-bin 3f" << std::endl;
    std::cerr << "" << std::endl;
    std::cerr << "      Sample the 10th record" << std::endl;
    std::cerr << "      ( echo 0.5; echo 0.1 ) | csv-seek scrub \"data.bin;binary=3f\" | csv-from-bin 3f" << std::endl;
    std::cerr << "" << std::endl;
    std::cerr << "      Scrub through a point cloud (note this example requires snark):" << std::endl;
    std::cerr << "      csv-sliders \"percentage;min=0;max=1\" --on-change --frequency 100 | csv-seek scrub --fields=percent \"data.bin;binary=3f\" | view-points \"-;binary=3f;size=1\" << std::endl;" << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

std::streampos jump_to_record(const std::string& file_path, double percentage, size_t record_size, std::vector<char>& record_data) 
{
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file.");
    }

    std::streamsize file_size = file.tellg();
    std::streampos target_offset = static_cast<std::streampos>(file_size * percentage);
    std::streampos adjusted_offset = (target_offset / record_size) * record_size;

    file.seekg(adjusted_offset);
    record_data.resize(record_size);
    file.read(record_data.data(), record_size);

    if (file.gcount() != record_size) {
        throw std::runtime_error("Unable to read a full record at the adjusted offset.");
    }

    return adjusted_offset;
}

namespace comma { namespace csv {
struct config_t
{
    std::string filename;
    std::string format;
};

struct input_t
{
    double offset;
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
        v.apply( "offset", p.offset );
    }

    template < typename K, typename V > static void visit( const K&, const comma::csv::input_t& p, V& v )
    {
        v.apply( "offset", p.offset );
    }
};

} } // namespace comma { namespace visiting {

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        std::vector< std::string > unnamed = options.unnamed( "", "-[^;].*" );
        comma::csv::options csv( options );

        std::cerr << "unnamed.size(): " << unnamed.size() << std::endl;
        COMMA_ASSERT( unnamed.size() > 0, "expected operation" );
        COMMA_ASSERT( unnamed.size() > 1, "expected file" );
        COMMA_ASSERT( unnamed.size() < 3, "Does not work on multiple streams (yet (shouuld it?))" );

        std::string operation = unnamed[0];
        COMMA_ASSERT( operation=="scrub" || operation=="index", "expected operation to be scrub or index" );
        
        comma::name_value::parser csv_options_parser( "filename", ';', '=', false );
        auto stream = csv_options_parser.get< comma::csv::config_t >( unnamed[1] );
        auto stream_csv = csv_options_parser.get< comma::csv::options >( unnamed[1] );
        std::string filename = stream.filename;
        COMMA_ASSERT( filename!="-", "expected filename. file scrubbing does not work on streams." );
        COMMA_ASSERT( stream_csv.binary(), "expected binary file" );

        if( operation=="scrub" ) { std::cerr << "configuring scrub operation... todo" << std::endl; }
        else if( operation=="index" ) { std::cerr << "configuring index operation... todo" << std::endl; }

        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        COMMA_ASSERT( file.is_open(), "unable to open file" );

        std::streamsize file_size = file.tellg();
        std::streampos record_size = stream_csv.format().size();

        comma::csv::input_stream< comma::csv::input_t > istream( std::cin, csv );
        while( std::cin.good() && !std::cin.eof() )
        {
            const comma::csv::input_t* p = istream.read();
            if( !p ) { break; }

            std::streampos target_offset = static_cast<std::streampos>(file_size * p->offset);
            std::streampos adjusted_offset = (target_offset / record_size) * record_size;

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
