// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2023-2024 Vsevolod Vlaskine

/// @author dewey nguyen

#include <deque>
#include <iostream>
#include <vector>
#include "../../base/types.h"
#include "../../application/command_line_options.h"
#include "../options.h"
#include "../stream.h"
#include "../format.h"

using namespace comma;

namespace comma { namespace csv { namespace applications { namespace shape { namespace operations { namespace sliding_window { static std::string usage( bool verbose ); } } } } } }

static void usage( bool verbose=false )
{
    std::cerr << "Perform reshaping operations on input data" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations" << std::endl;
    std::cerr << "    concatenate:    group input records for concatenation into output records." << std::endl;
    std::cerr << "                    the user can choose non-overlapping or overlapping grouping (sliding window) mode." << std::endl;
    std::cerr << "    loop:           same as concatenate, but with an additional last record:" << std::endl;
    std::cerr << "                    last input record concatenated with the first record (hence, 'loop')" << std::endl;
    std::cerr << "                    this mode always uses the sliding window for overlapping groups" << std::endl;
    std::cerr << "    repeat:         repeat input given number of times, e.g. csv-shape repeat --size 5" << std::endl;
    std::cerr << "    sliding-window: todo" << std::endl;
    std::cerr << "    split:          csv-only: split line at n-th field, e.g. csv-shape split --size 4" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Usage: cat data.csv | csv-shape <operation> [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --expected-records; output the expected records for given --size and --step, and exit" << std::endl;
    std::cerr << "    --size,-n=<num>; number of input records in each grouping, range: 2 and above" << std::endl;
    std::cerr << "    --step=<num>; default=1; relative offset of the records to be concatenated" << std::endl;
    std::cerr << "    --verbose,-v: more output to stderr, shows examples with --help,-h" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations options" << std::endl;
    std::cerr << "    concatenate" << std::endl;
    std::cerr << "       --bidirectional; output records in both directions (e.g. a,b; b,a)" << std::endl;
    std::cerr << "       --reverse; output records in reverse order (e.g. b,a)" << std::endl;
    std::cerr << "       --sliding-window,-w; use a sliding window to group input records, see examples" << std::endl;
    std::cerr << "    loop" << std::endl;
    std::cerr << "       --bidirectional; output records in both directions (e.g. a,b; b,a)" << std::endl;
    std::cerr << "       --reverse; output records in reverse order (e.g. b,a)" << std::endl;
    std::cerr << "    split" << std::endl;
    std::cerr << "       --repeat; e.g: echo 0,1,2,3,4,5 | csv-shape split -n 2 will output: 0,1 and 2,3,4,5" << std::endl;
    std::cerr << "                      echo 0,1,2,3,4,5 | csv-shape split -n 2 --repeat will output: 0,1, then 2,3, then 4,5" << std::endl;
    std::cerr << "       --size,-n=<n>; e.g. echo 0,1,2,3,4,5| csv-shape split -n 3 will output" << std::endl;
    std::cerr << "                      two lines: 0,1,2 and 3,4,5" << std::endl;
    std::cerr << comma::csv::applications::shape::operations::sliding_window::usage( verbose ) << std::endl;
    std::cerr << std::endl;
    std::cerr << "csv options" << std::endl;
    std::cerr << comma::csv::options::usage( verbose ) << std::endl;
    std::cerr << "examples" << std::endl;
    if( verbose )
    {
        std::cerr << "examples" << std::endl;
        std::cerr << "   concatenate" << std::endl;
        std::cerr << "      non-overlapping groups:" << std::endl;
        std::cerr << "          concatenate each group of 5 input records into one output record." << std::endl;
        std::cerr << "          input records 1 to 5 create the first output record, input records 6-10 create the second output record, and so forth." << std::endl;
        std::cerr << "              seq 1 15 | csv-shape concatenate -n 5" << std::endl;
        std::cerr << "      overlapping groups:" << std::endl;
        std::cerr << "          move a sliding window of size 5 along the input records, every time the sliding window moves, make an output record from window" << std::endl;
        std::cerr << "          input records 1 to 5 create the first output record, input records 2 to 6 create the second record, input records 3 to 7 create the third record, and so forth" << std::endl;
        std::cerr << "              seq 1 10 | csv-shape concatenate -n 5 --sliding-window" << std::endl;
        std::cerr << std::endl;
    }
    else
    {
        std::cerr << "    run csv-shape --help --verbose for more..." << std::endl;
    }
    exit( 0 );
}

// There is nothing to do in this case - binary data
static void _binary_pass_through(const comma::csv::format& f, bool flush=false)
{
    std::vector< char > buffer( f.size(), '\0' );
    while( std::cin.good() && !std::cin.eof() )
    {
        if( std::cin.read( &buffer[0], buffer.size() ) )
        {
            std::cout.write( &buffer[0], buffer.size() );
            if( flush ) { std::cout.flush(); }
        }
    }
}

static bool is_binary;

class _concatenate
{
public:
    _concatenate()
        : use_sliding_window_(false)
        , bidirectional_(false)
        , reverse_(false)
        , looping_(false)
        , size_(0)
        , count_(0)
        , block_(0)
        , step_(1)
        , expected_records_(0)
        {}

    int run( const comma::command_line_options& options, const comma::csv::options& csv)
    {
        std::vector< std::string > unnamed = options.unnamed( "--size,-n,--sliding-window,-w,--step,--verbose,-v", "-.*" );
        std::string operation = unnamed[0];
        looping_ = ( operation == "loop");
        use_sliding_window_ = ( looping_ || options.exists("--sliding-window,-w") );
        reverse_ = options.exists("--reverse");
        bidirectional_ = options.exists("--bidirectional");
        if( !use_sliding_window_ && is_binary ) { _binary_pass_through( csv.format(), csv.flush ); return 0; };
        size_ = looping_ ? options.value("--size,-n", 2) : options.value< comma::uint32 >("--size,-n");
        step_ = options.value( "--step",1 );
        if( size_ < 2 ) { std::cerr <<  comma::verbose.app_name() << ": expected --size,-n= value to be greater than 1" << std::endl; return 1; }
        expected_records_ = step_ * ( size_ - 1 ) + 1;
        if( options.exists("--expected-records") ) { std::cout << expected_records_ << std::endl; return 0; };
        comma::csv::input_stream< input_t > istream( std::cin, csv );
        std::deque< std::string > deque;
        std::deque< std::string > first;
        bool has_block_ = csv.has_field( "block" );
        while( istream.ready() || ( std::cin.good() && !std::cin.eof() ) )
        {
            const input_t* p = istream.read();
            if( !p ) { break; }
            if ( !deque.empty() && has_block_ && (block_ != p->block) )
            {
                if (!verify(deque)) { return 1; }
                count_ = 0;
                if ( looping_ ) { output_loop(deque, first, csv); }
                deque.clear();
            }
            block_ = p->block;
            deque.push_back( istream.last() );
            if ( looping_ && first.size() + 1 < expected_records_ )  { first.push_back(istream.last()); }
            ++count_;
            if( deque.size() >= step_ * ( size_ - 1 ) + 1 )
            {
                output_records(deque, csv);
                if( !use_sliding_window_ ) { deque.clear(); } else { deque.pop_front(); }
            }
        }
        if ( !verify(deque)) { return 1; }
        if ( looping_ ) { output_loop(deque, first, csv); }
        return 0;
    }

    struct input_t { comma::uint32 block = 0; };

private:
    bool use_sliding_window_;
    bool bidirectional_;
    bool reverse_;
    bool looping_;
    comma::uint32 size_;
    comma::uint32 count_;
    comma::uint32 block_;
    comma::uint32 step_;
    comma::uint32 expected_records_;

    bool verify( const std::deque<std::string>& deque )
    {
        if( use_sliding_window_ && count_ < step_ * ( size_ - 1 ) + 1 )
        {
            std::cerr << comma::verbose.app_name() << ": --size,-n=" << size_ << ", --step=" << step_ << ", expected records count (" << step_ * ( size_ - 1 ) + 1
                      << ") is bigger than total number of input records: " << count_ << std::endl;
            return false;
        }
        if ( !use_sliding_window_ && !deque.empty() )
        {
            std::cerr << comma::verbose.app_name() << ": error, leftover tail input record found: " << deque.size() << " lines." << std::endl;
            return false;
        }
        return true;
    }

    void output_records(const std::deque<std::string>& deque, const comma::csv::options& csv)
    {
        auto delimiter = std::string();
        if (!reverse_)
        {
            std::cout.write( &( deque.front()[ 0 ] ), deque.front().size() );
            auto is = deque.cbegin(); while( ( is + 1 ) != deque.cend() ) { if(!is_binary){ std::cout << csv.delimiter; } is += step_; std::cout.write( &(*is)[0], is->size() ); }
            if(!is_binary){ std::cout << std::endl; }
            if (csv.flush) { std::cout.flush(); }
        }
        if (bidirectional_ || reverse_ )
        {
            std::cout.write( &( deque.back()[0] ), deque.back().size() );
            auto is = deque.crbegin(); while( ( is + 1 ) != deque.crend() ) { if(!is_binary){ std::cout << csv.delimiter; } is += step_; std::cout.write( &(*is)[0], is->size() ); }
            if(!is_binary){ std::cout << std::endl; }
            if (csv.flush) { std::cout.flush(); }
        }
    }

    void output_loop( std::deque<std::string>& deque, std::deque<std::string>& first, const comma::csv::options& csv)
    {
        while ( first.size() > 0 )
        {
            deque.push_back(first.front());
            output_records(deque, csv);
            deque.pop_front();
            first.pop_front();
        }
    }
};

namespace comma { namespace visiting {

template <> struct traits< _concatenate::input_t >
{
    template < typename K, typename V > static void visit( const K&, const _concatenate::input_t& p, V& v ) { v.apply("block", p.block); }
    template < typename K, typename V > static void visit( const K&, _concatenate::input_t& p, V& v ) { v.apply("block", p.block); }
};

} } // namespace comma { namespace visiting {

static int _repeat( const comma::command_line_options& options, const comma::csv::options& csv )
{
    unsigned int size = options.value< unsigned int >( "--size,-n" );
    if( csv.binary() )
    {
        typedef _concatenate::input_t input_t; // quick and dirty
        comma::csv::input_stream< input_t > is( std::cin, csv ); // quick and dirty, will be slow on ascii
        while( is.ready() || ( std::cin.good() && !std::cin.eof() ) )
        {
            const input_t* p = is.read();
            if( !p ) { break; }
            for( unsigned int i = 0; i < size; ++i ) { std::cout.write( is.binary().last(), csv.format().size() ); }
            if( csv.flush ) { std::cout.flush(); }
        }
    }
    else
    {
        while( std::cin.good() && !std::cin.eof() )
        {
            std::string line;
            std::getline( std::cin, line );
            if( comma::strip( line ).empty() ) { continue; }
            for( unsigned int i = 0; i < size; ++i ) { std::cout << line << std::endl; }
        }
    }
    return 0;
}

static int _split( const comma::command_line_options& options, const comma::csv::options& csv )
{
    if( csv.binary() ) { _binary_pass_through( csv.format(), csv.flush ); return 0; };
    unsigned int size = options.value< unsigned int >( "--size,-n" );
    bool repeat = options.exists( "--repeat" );
    while( std::cin.good() && !std::cin.eof() )
    {
        std::string line;
        std::getline( std::cin, line );
        const auto& s = comma::strip( line );
        if( s.empty() ) { continue; }
        unsigned int p{0};
        for( unsigned int i{0}, c{0}; i < s.size(); ++i )
        {
            if( s[i] == csv.delimiter ) { ++c; }
            if( c < size ) { continue; }
            std::cout << s.substr( p, i - p ) << std::endl;
            p = i + 1;
            c = 0;
            if( !repeat ) { break; }
        }
        if( p < s.size() ) { std::cout << s.substr( p ) << std::endl; }
    }
    return 0;
}

namespace comma { namespace csv { namespace applications { namespace shape { namespace operations {

namespace sliding_window {

std::string usage( bool )
{
    return R"(    sliding-window
        --incremental; output first block incrementally: first record, then first and second, etc
        --size=<n>; number of input records in each grouping
        --step=<n>; default=1; sliding window step)";
}

static int run( const comma::command_line_options& options, comma::csv::options& csv )
{
    const unsigned int size = options.value< unsigned int >( "--size" );
    const unsigned int stride = options.value< unsigned int >( "--step,--stride", 1 );
    COMMA_ASSERT( stride <= size, "stride greater than size currently not supported, just ask; got: size: " << size << " stride: " << stride );
    std::deque< std::string > deque;
    std::string record = csv.binary() ? std::string( csv.format().size(), 0 ) : std::string();
    bool incremental = options.exists( "--incremental" );
    auto output_record = [&]( const std::string& record )
    {
        std::cout.write( &record[0], record.size() );
        if( !csv.binary() ) { std::cout << std::endl; }
    };
    auto output_all = [&]() { for( const auto& r: deque ) { output_record( r ); } };
    while( std::cin.good() && !std::cin.eof() )
    {
        if( csv.binary() )
        {
            std::cin.read( &record[0], record.size() );
            if( std::cin.gcount() == 0 ) { break; }
            COMMA_ASSERT( std::cin.gcount() == int( record.size() ), "expected " << record.size() << " bytes; got only: " << std::cin.gcount() );
        }
        else
        {
            std::getline( std::cin, record );
            if( comma::strip( record ).empty() ) { continue; }
        }
        deque.push_back( record );
        if( deque.size() > ( size + stride ) )
        {
            for( unsigned int i = 0; i < stride; ++i ) { deque.pop_front(); } // quick and dirty
            output_all();
        }
        else
        {
            if( deque.size() > size ) { continue; }
            if( incremental ) { output_all(); } else { output_record( record ); }
        }
        if( csv.flush ) { std::cout.flush(); }
    }
    return 0;
}

} // namespace sliding_window {

} } } } } // namespace comma { namespace csv { namespace applications { namespace shape { namespace operations {

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        std::vector< std::string > unnamed = options.unnamed( "--expected-records,--incremental,--repeat,--sliding-window,-w,--verbose,-v", "-.*" );
        comma::csv::options csv( options );
        csv.full_xpath = false;
        if( csv.fields.empty() ) { csv.fields="a"; }
        is_binary = csv.binary();
        if( unnamed.empty() ) { std::cerr << comma::verbose.app_name() << ": please specify operations" << std::endl; exit( 1 ); }
        std::string operation = unnamed[0];
        if( operation == "concatenate" || operation == "loop" ) { return _concatenate().run( options, csv ); }
        if( operation == "repeat" ) { return _repeat( options, csv ); }
        if( operation == "sliding-window" ) { return comma::csv::applications::shape::operations::sliding_window::run( options, csv ); }
        if( operation == "split" ) { return _split( options, csv ); }
        comma::say() << ": expected operation; got: '" << operation << "'" << std::endl;
    }
    catch( std::exception& ex ) { std::cerr << "csv-shape: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-shape: unknown exception" << std::endl; }
    return 1;
}
