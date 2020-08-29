// This file is part of comma, a generic and flexible library
// Copyright (c) 2011 The University of Sydney
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Sydney nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
// HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


/// @author vsevolod vlaskine

#include <deque>
#include <iostream>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include "../../application/command_line_options.h"
#include "../../application/contact_info.h"
#include "../../application/signal_flag.h"
#include "../../base/types.h"
#include "../../csv/stream.h"
#include "../../io/stream.h"
#include "../../csv/traits.h"
#include "../../io/select.h"
#include "../../name_value/parser.h"
#include "../../string/string.h"
#include "../../visiting/traits.h"

static void bash_completion( unsigned const ac, char const * const * av )
{
    static const char* completion_options =
        " --help --verbose"
        " --by-lower --by-upper --nearest --realtime"
        " --binary --delimiter --fields"
        " --bound --do-not-append --select --timestamp-only"
        " --buffer --discard-bounding"
        ;
    std::cout << completion_options << std::endl;
    exit( 0 );
}

static void usage( bool verbose )
{
    std::cerr << std::endl;
    std::cerr << "join timestamped data from stdin with corresponding timestamped data from the" << std::endl;
    std::cerr << "second input" << std::endl;
    std::cerr << std::endl;
    std::cerr << "timestamps are expected to be fully ordered" << std::endl;
    std::cerr << std::endl;
    std::cerr << "note: on windows only files are supported as bounding data" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat a.csv | csv-time-join <how> [<options>] bounding.csv [-] > joined.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "<how>" << std::endl;
    std::cerr << "    --by-lower: join by lower timestamp (default)" << std::endl;
    std::cerr << "    --by-upper: join by upper timestamp" << std::endl;
    std::cerr << "    --nearest:  join by nearest timestamp" << std::endl;
    std::cerr << "                if 'block' given in --fields, output the whole block" << std::endl;
    std::cerr << "    --realtime: (streams only) output input immediately joined with current" << std::endl;
    std::cerr << "                latest bounding timestamp. The joined bounding timestamp may" << std::endl;
    std::cerr << "                be less than or greater than the timestamp from stdin." << std::endl;
    std::cerr << "                No timestamp comparisons are made before outputting a record." << std::endl;
    std::cerr << std::endl;
    std::cerr << "<input/output options>" << std::endl;
    std::cerr << "    -: if csv-time-join - b.csv, concatenate output as: <stdin><b.csv>" << std::endl;
    std::cerr << "       if csv-time-join b.csv -, concatenate output as: <b.csv><stdin>" << std::endl;
    std::cerr << "       default: csv-time-join - b.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    --help,-h:                  this help" << std::endl;
    std::cerr << "    --verbose,-v:               more output" << std::endl;
    std::cerr << "    --binary,-b <format>:       binary format" << std::endl;
    std::cerr << "    --delimiter,-d <delimiter>: ascii only; default ','" << std::endl;
    std::cerr << "    --fields,-f <fields>:       input fields; default: t" << std::endl;
    std::cerr << "    --bound=[<seconds>]:        output only points within given bound" << std::endl;
    std::cerr << "    --do-not-append,--select:   do not append any field from the second input" << std::endl;
    std::cerr << "    --timestamp-only:           append only timestamp from the second input" << std::endl;
    std::cerr << "    --buffer=[<records>]:       bounding data buffer size; default: infinite" << std::endl;
    std::cerr << "    --discard-bounding:         discard bounding data if buffer size reached;" << std::endl;
    std::cerr << "                                default is to block until stdin catches up" << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    first field on stdin is timestamp, the first field of filter is timestamp" << std::endl;
    std::cerr << "        - default:" << std::endl;
    std::cerr << "            cat a.csv | csv-time-join b.csv" << std::endl;
    std::cerr << "        - explicit:" << std::endl;
    std::cerr << "            cat a.csv | csv-time-join --fields=t \"b.csv;fields=t\"" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    3rd field on stdin is timestamp, the 2nd field of filter is timestamp" << std::endl;
    std::cerr << "        cat a.csv | csv-time-join --fields=,,t \"b.csv;fields=,t\"" << std::endl;
    std::cerr << std::endl;
    if( verbose )
    {
        std::cerr << "    echo \"20170101T115955,a\" >  a.csv" << std::endl;
        std::cerr << "    echo \"20170101T120001,b\" >> a.csv" << std::endl;
        std::cerr << "    echo \"20170101T120002,c\" >> a.csv" << std::endl;
        std::cerr << "    echo \"20170101T120007,d\" >> a.csv" << std::endl;
        std::cerr << "    echo \"20170101T120012,e\" >> a.csv" << std::endl;
        std::cerr << "    echo \"20170101T120015,f\" >> a.csv" << std::endl;
        std::cerr << "    echo \"20170101T120000,y\" >  b.csv" << std::endl;
        std::cerr << "    echo \"20170101T120010,z\" >> b.csv" << std::endl;
        std::cerr << std::endl;
        std::cerr << "    cat a.csv | csv-time-join b.csv" << std::endl;
        std::cerr << "    cat a.csv | csv-time-join b.csv --by-upper" << std::endl;
        std::cerr << "    cat a.csv | csv-time-join b.csv --nearest" << std::endl;
        std::cerr << "    cat a.csv | csv-time-join b.csv --nearest --bound=2" << std::endl;
        std::cerr << "    cat a.csv | csv-time-join b.csv --nearest --bound=2 --select" << std::endl;
        std::cerr << "    cat a.csv | csv-time-join b.csv --nearest --bound=2 --timestamp-only" << std::endl;
        std::cerr << std::endl;
        std::cerr << "    ( sleep 1; cat a.csv ) | csv-play |" << std::endl;
        std::cerr << "        csv-time-join --realtime <( cat b.csv | csv-play )" << std::endl;
}
    else
    {
        std::cerr << "    try --help --verbose for more examples" << std::endl;
    }
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

struct Point
{
    boost::optional<boost::posix_time::ptime> timestamp;
    Point() {}
    Point( const boost::posix_time::ptime& timestamp ) : timestamp( timestamp ) {}
};

namespace comma { namespace visiting {

template <> struct traits< Point >
{
    template < typename K, typename V > static void visit( const K&, const Point& p, V& v ) { v.apply( "t", p.timestamp ); }
    template < typename K, typename V > static void visit( const K&, Point& p, V& v ) { v.apply( "t", p.timestamp ); }
};
    
} } // namespace comma { namespace visiting {

enum class how { by_lower, by_upper, nearest, realtime };
how method = how::by_lower;
bool timestamp_only;
bool select_only;
comma::csv::options stdin_csv;
comma::csv::options bounding_csv;
boost::optional< boost::posix_time::time_duration > bound;
typedef std::pair< boost::posix_time::ptime, std::string > timestring_t;

boost::posix_time::ptime get_time( const Point& p ) { return p.timestamp ? *p.timestamp : boost::posix_time::microsec_clock::universal_time(); }

static void output_bounding( std::ostream& os, const timestring_t& bounding, bool stdin_first )
{
    if( !select_only )
    {
        if( stdin_csv.binary() )
        {
            if( timestamp_only )
            {
                static const unsigned int time_size = comma::csv::format::traits< boost::posix_time::ptime, comma::csv::format::time >::size;
                static char timestamp[ time_size ];
                comma::csv::format::traits< boost::posix_time::ptime, comma::csv::format::time >::to_bin( bounding.first, timestamp );
                os.write( (char*)&timestamp, time_size );
            }
            else
            {
                os.write( &bounding.second[0], bounding.second.size() );
            }
        }
        else
        {
            if( stdin_first ) { os << stdin_csv.delimiter; }
            os << ( timestamp_only ? boost::posix_time::to_iso_string( bounding.first ) : bounding.second );
            if( !stdin_first ) { os << stdin_csv.delimiter; }
        }
    }
}

static void output_input( std::ostream& os, const timestring_t& input )
{
    if( stdin_csv.binary() ) { os.write( &input.second[0], stdin_csv.format().size() ); } else { os << input.second; }
}

static void output( const timestring_t& input, const timestring_t& bounding, bool stdin_first )
{
    if( bounding.first.is_infinity() ) { return; }
    if( bound && ( input.first - bounding.first > bound || bounding.first - input.first > bound )) { return; }
    if( stdin_first )
    {
        output_input( std::cout, input );
        output_bounding( std::cout, bounding, stdin_first );
    }
    else
    {
        output_bounding( std::cout, bounding, stdin_first );
        output_input( std::cout, input );
    }
    if( !stdin_csv.binary() ) { std::cout << '\n'; }
    std::cout.flush();
}

int main( int ac, char** av )
{
    try
    {
        comma::signal_flag is_shutdown(comma::signal_flag::hard);
        comma::command_line_options options( ac, av, usage );
        if( options.exists( "--bash-completion" )) bash_completion( ac, av );
        options.assert_mutually_exclusive( "--by-lower,--by-upper,--nearest,--realtime" );
        if( options.exists( "--by-upper" )) { method = how::by_upper; }
        if( options.exists( "--nearest" )) { method = how::nearest; }
        if( options.exists( "--realtime" )) { method = how::realtime; }
        timestamp_only = options.exists( "--timestamp-only,--time-only" );
        select_only = options.exists( "--do-not-append,--select" );
        if( select_only && timestamp_only ) { std::cerr << "csv-time-join: --timestamp-only specified with --select, ignoring --timestamp-only" << std::endl; }
        bool discard_bounding = options.exists( "--discard-bounding" );
        boost::optional< unsigned int > buffer_size = options.optional< unsigned int >( "--buffer" );
        if( options.exists( "--bound" ) ) { bound = boost::posix_time::microseconds( static_cast<unsigned int>(options.value< double >( "--bound" ) * 1000000 )); }
        stdin_csv = comma::csv::options( options, "t" );
        std::vector< std::string > unnamed = options.unnamed(
            "--by-lower,--by-upper,--nearest,--realtime,--select,--do-not-append,--timestamp-only,--time-only,--discard-bounding",
            "--binary,-b,--delimiter,-d,--fields,-f,--bound,--buffer,--verbose,-v" );
        std::string properties;
        bool stdin_first = true;
        switch( unnamed.size() )
        {
            case 0:
                std::cerr << "csv-time-join: please specify bounding source" << std::endl;
                return 1;
            case 1:
                properties = unnamed[0];
                break;
            case 2:
                if( unnamed[0] == "-" ) { properties = unnamed[1]; }
                else if( unnamed[1] == "-" ) { properties = unnamed[0]; stdin_first = false; }
                else { std::cerr << "csv-time-join: expected either '- <bounding>' or '<bounding> -'; got : " << comma::join( unnamed, ' ' ) << std::endl; return 1; }
                break;
            default:
                std::cerr << "csv-time-join: expected either '- <bounding>' or '<bounding> -'; got : " << comma::join( unnamed, ' ' ) << std::endl;
                return 1;
        }
        comma::name_value::parser parser( "filename" );
        bounding_csv = parser.get< comma::csv::options >( properties );
        if( bounding_csv.fields.empty() ) { bounding_csv.fields = "t"; }

        comma::csv::input_stream< Point > stdin_stream( std::cin, stdin_csv );
        #ifdef WIN32
        if( stdin_csv.binary() ) { _setmode( _fileno( stdout ), _O_BINARY ); }
        #endif // #ifdef WIN32

        comma::io::istream bounding_istream( comma::split( properties, ';' )[0], bounding_csv.binary() ? comma::io::mode::binary : comma::io::mode::ascii );
        comma::csv::input_stream< Point > bounding_stream( *bounding_istream, bounding_csv );

        #ifndef WIN32
        comma::io::select select;
        comma::io::select bounding_stream_select;
        select.read().add( 0 );
        select.read().add( bounding_istream.fd() );
        bounding_stream_select.read().add( bounding_istream.fd() );
        #endif // #ifndef WIN32

        const Point* p = NULL;
        if( method == how::realtime )
        {
            #ifdef WIN32
            COMMA_THROW( comma::exception, "--realtime mode not supported in WIN32" );
            #else
            bool end_of_input = false;
            bool end_of_bounds = false;
            boost::optional< timestring_t > joined_line;
            while( !is_shutdown && !end_of_input )
            {
                if( !bounding_stream.ready() && !stdin_stream.ready() ) { select.wait(boost::posix_time::milliseconds(1)); }
                if( !is_shutdown && !end_of_input && ( stdin_stream.ready() || ( select.check() && select.read().ready( comma::io::stdin_fd ) ) ) )
                {
                    p = stdin_stream.read();
                    if( p )
                    {
                        timestring_t input_line = std::make_pair( get_time( *p ), stdin_stream.last() );
                        if( joined_line ) { output( input_line, *joined_line, stdin_first ); }
                    }
                    else
                    {
                        comma::verbose << "end of input stream" << std::endl;
                        end_of_input = true;
                    }
                }
                if( !is_shutdown && !end_of_bounds && ( bounding_stream.ready() || ( select.check() && select.read().ready( bounding_istream.fd() ) ) ) )
                {
                    p = bounding_stream.read();
                    if( p )
                    {
                        joined_line = std::make_pair( get_time( *p ), bounding_stream.last() );
                    }
                    else
                    {
                        comma::verbose << "end of bounding stream" << std::endl;
                        end_of_bounds = true;
                    }
                }
            }
            if( is_shutdown ) { comma::verbose << "got a signal" << std::endl; return 0; }
            #endif // #ifdef WIN32
        }
        else
        {
            std::deque< timestring_t > bounding_queue;
            bool next = true;
            bool bounding_data_available;
            bool upper_bound_added = false;
            bounding_queue.push_back( std::make_pair( boost::posix_time::neg_infin, "" ) ); // add a fake entry for an lower bound to allow stdin before first bound to match
            while( stdin_stream.ready() || ( std::cin.good() && !std::cin.eof() ) )
            {
                if( !std::cin.good() ) { select.read().remove( 0 ); }
                if( !bounding_istream->good() ) { select.read().remove( bounding_istream.fd() ); }
                bounding_data_available = bounding_stream.ready() || ( bounding_istream->good() && !bounding_istream->eof() );
                #ifdef WIN32
                bool bounding_stream_ready = true;
                bool stdin_stream_ready = true;
                #else // #ifdef WIN32
                //check so we do not block
                bool bounding_stream_ready = bounding_stream.ready();
                bool stdin_stream_ready = stdin_stream.ready();
                if( next )
                {
                    if( !bounding_stream_ready || !stdin_stream_ready )
                    {
                        if( !bounding_stream_ready && !stdin_stream_ready ) { select.wait( boost::posix_time::milliseconds( 10 ) ); }
                        else { select.check(); }
                        if( select.read().ready( bounding_istream.fd() )) { bounding_stream_ready = true; }
                        if( select.read().ready(0) ) { stdin_stream_ready = true; }
                    }
                }
                else
                {
                    if( !bounding_stream_ready )
                    {
                        bounding_stream_select.wait( boost::posix_time::milliseconds( 10 ) );
                        if( bounding_stream_select.read().ready( bounding_istream.fd() )) { bounding_stream_ready=true; }
                    }
                }
                #endif //#ifdef WIN32
                //keep storing available bounding data
                if( bounding_stream_ready )
                {
                    if( !buffer_size || bounding_queue.size() < *buffer_size || discard_bounding )
                    {
                        const Point* q = bounding_stream.read();
                        if( q ) { bounding_queue.push_back( std::make_pair( get_time( *q ), bounding_stream.last() )); }
                        else { bounding_data_available = false; }
                    }
                    if( buffer_size && bounding_queue.size() > *buffer_size && discard_bounding ) { bounding_queue.pop_front(); }
                }
                if( !upper_bound_added && bounding_istream->eof() )
                {
                    // add a fake entry for an upper bound to allow stdin data above last bound to match
                    bounding_queue.push_back( std::make_pair( boost::posix_time::pos_infin, "" ));
                    upper_bound_added = true;
                }
                //if we are done with the last bounded point get next
                if( next )
                {
                    if( !stdin_stream_ready ) { continue; }
                    p = stdin_stream.read();
                    if( !p ) { break; }
                }
                boost::posix_time::ptime t = get_time( *p );
                //get bound
                for( ; bounding_queue.size() >= 2 && t >= bounding_queue[1].first; bounding_queue.pop_front() );
                if( bounding_queue.size() < 2 )
                {
                    //bound not found
                    //do we have more data?
                    if( !bounding_data_available ) { break; }
                    next = false;
                    continue;
                }
                //bound available
                if( method == how::by_lower && t < bounding_queue.front().first )
                {
                    next = true;
                    continue;
                }
                bool is_first = ( method == how::by_lower )
                    || ( method == how::nearest && ( t - bounding_queue[0].first ) < ( bounding_queue[1].first - t ));
                const timestring_t& chosen_bound = is_first ? bounding_queue[0] : bounding_queue[1];;
                timestring_t input_line = std::make_pair( t, stdin_stream.last() );
                output( input_line, chosen_bound, stdin_first );
                next = true;
            }
        }
        return 0;     
    }
    catch( std::exception& ex ) { std::cerr << "csv-time-join: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-time-join: unknown exception" << std::endl; }
    return 1;
}
