// This file is part of comma, a generic and flexible library
// Copyright (c) 2016 The University of Sydney
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

/// @author dave jennings

#include <functional>
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include "../../application/command_line_options.h"
#include "../../application/signal_flag.h"
#include "../../csv/options.h"
#include "../../csv/stream.h"
#include "../../io/select.h"
#include "../../io/stream.h"

static void bash_completion( unsigned const ac, char const * const * av )
{
    static const char* completion_options =
        " --help -h --verbose -v"
        " --timeout --period --pace"
        " --append-fields --append -a"
        " --output-fields --output-format"
        " --ignore-eof --ignore --yes"
        ;
    std::cout << completion_options << std::endl;
    exit( 0 );
}

void usage( bool verbose = false )
{
    std::cerr << std::endl;
    std::cerr << "pass stdin to stdout, repeating the last record after a period of inactivity" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: csv-repeat [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --help,-h: help; --help --verbose: more help" << std::endl;
    std::cerr << "    --append-fields,--append,-a=[<fields>]: add extra fields to output" << std::endl;
    std::cerr << "    --ignore-eof,--ignoreeof,--yes: if --period specified, ignore end of stdin, keep repeating" << std::endl;
    std::cerr << "                                    warning: currently, if you kill csv-repeat --yes with a signal" << std::endl;
    std::cerr << "                                             it will not exit until period timer expires; this may" << std::endl;
    std::cerr << "                                             be a problem for a large period timeout" << std::endl;
    std::cerr << "    --output-fields: print output fields and exit" << std::endl;
    std::cerr << "    --output-format: print output format and exit" << std::endl;
    std::cerr << "    --pace: output with a given --period, even if the input records are coming in at a higher pace" << std::endl;
    std::cerr << "            warning: currently is very simplistic; see todo comments in the code to make it more robust" << std::endl;
    std::cerr << "    --period=[<seconds>]: period of repeated record" << std::endl;
    std::cerr << "    --timeout,-t=[<seconds>]: timeout before repeating the last record; if not specified, timeout is set to --period" << std::endl;
    std::cerr << "    --timestamped: use input timestamp for repeating; currently, would do blocking read" << std::endl;
    std::cerr << "                   convenient for filling holes in data in offline processing" << std::endl;
    std::cerr << "      --timestamped options" << std::endl;
    std::cerr << "          --at-least-from,--from=[<time>]; if first timestamp greater than <time>, fill the hole with the first record" << std::endl;
    std::cerr << "          --at-least-to,--to=[<time>]; if last timestamp less than <time>, fill the hole with the last record" << std::endl;
    std::cerr << "    --verbose,-v: more output" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    if --period is not set, --timeout acts as a watchdog. If no input is seen" << std::endl;
    std::cerr << "    within the timeout csv-repeat exits with an error." << std::endl;
    std::cerr << std::endl;
    std::cerr << "    --append fields are appended to output; supported fields are:" << std::endl;
    std::cerr << "        time: append timestamp" << std::endl;
    std::cerr << "        repeating: 1 if currently repeating" << std::endl;
    std::cerr << "        repeat_count: 0 if not repeated, otherwise counts up for consecutive repeating records" << std::endl;
    std::cerr << std::endl;
    if( verbose )
    {
        std::cerr << "csv options:" << std::endl;
        std::cerr << comma::csv::options::usage() << std::endl;
    }
    else { std::cerr << "    see --help --verbose for more details" << std::endl << std::endl; }
    std::cerr << "examples" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    { echo -e \"1\\n2\\n3\"; sleep 10; } | csv-repeat --timeout=3 --period=1" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    { echo -e \"1\\n2\\n3\"; sleep 10; } \\" << std::endl;
    std::cerr << "        | csv-repeat --timeout=3 --period=1 --append=time,repeating" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    { echo -e \"1,2,3\\n4,5,6\\n7,8,9\"; sleep 10; } \\" << std::endl;
    std::cerr << "        | csv-to-bin 3d --flush \\" << std::endl;
    std::cerr << "        | csv-repeat --timeout=3 --period=1 --binary=3d \\" << std::endl;
    std::cerr << "        | csv-from-bin 3d --flush" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    { echo -e \"1\\n2\\n3\"; sleep 10; } | csv-repeat --timeout=3" << std::endl;
    std::cerr << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

struct input_t
{
    boost::posix_time::ptime time;
};

struct output_t
{
    boost::posix_time::ptime time;
    bool repeating;
    unsigned repeat_count;
    output_t() : repeating( false ), repeat_count(0) {}
    output_t( const boost::posix_time::ptime& time, bool repeating, unsigned repeat_count=0 ) : time( time ), repeating( repeating ), repeat_count(repeat_count) {}
};

namespace comma { namespace visiting {

template <> struct traits< input_t >
{
    template < typename K, typename V > static void visit( const K&, input_t& p, V& v ) { v.apply( "t", p.time ); }
    template < typename K, typename V > static void visit( const K&, const input_t& p, V& v ) { v.apply( "t", p.time ); }
};

template <> struct traits< output_t >
{
    template < typename K, typename V > static void visit( const K&, const output_t& p, V& v )
    {
        v.apply( "time", p.time );
        v.apply( "repeating", p.repeating );
        v.apply( "repeat_count", p.repeat_count );
    }
    template < typename K, typename V > static void visit( const K&, output_t& p, V& v )
    {
        v.apply( "time", p.time );
        v.apply( "repeating", p.repeating );
        v.apply( "repeat_count", p.repeat_count );
    }
};
    
} } // namespace comma { namespace visiting {

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        if( options.exists( "--bash-completion" ) ) bash_completion( ac, av );
        bool ignore_eof = options.exists( "--ignore-eof,--ignoreeof,--yes" );
        options.assert_mutually_exclusive( "--pace", "--ignore-eof,--ignoreeof,--yes" );
        options.assert_mutually_exclusive( "--timestamped,pace" );
        if( ignore_eof && !options.exists( "--period" ) ) { std::cerr << "csv-repeat: got --ignore-oef, thus please specify --period" << std::endl; return 1; }
        comma::csv::options csv = comma::csv::options( options );
        boost::scoped_ptr< comma::csv::output_stream< output_t > > ostream;
        comma::csv::options output_csv;
        output_csv.full_xpath = false;
        if( options.exists( "--append-fields,--append,-a" ) )
        {
            output_csv.fields = options.value< std::string >( "--append-fields,--append,-a", "" );
            if( csv.binary() )
            {
                std::string format;
                if( output_csv.fields.empty() )
                {
                    format = comma::csv::format::value< output_t >();
                }
                else
                {
                    const std::vector< std::string >& v = comma::split( output_csv.fields, ',' );
                    std::string comma;
                    for( unsigned int i = 0; i < v.size(); ++i )
                    {
                        if( v[i] == "repeating" ) { format += comma + 'b'; }
                        else if( v[i] == "time" ) { format += comma + 't'; }
                        else { std::cerr << "csv-repeat: expected one of: " << comma::join( comma::csv::names< output_t >( false ), ',' ) << "; got: \"" << v[i] << "\"" << std::endl; return 1; }
                        comma = ",";
                    }
                }
                output_csv.format( format );
            }
            output_csv.flush = true;
            output_csv.delimiter = csv.delimiter;
            ostream.reset( new comma::csv::output_stream< output_t >( std::cout, output_csv ) );
        }
        if( options.exists( "--output-fields" ) )
        {
            if( !options.exists( "--fields,-f" ) ) { std::cerr << "csv-repeat: --output-fields option requires --fields" << std::endl; return 1; }
            std::cout << options.value< std::string >( "--fields,-f" );
            if( !output_csv.fields.empty() ) { std::cout << csv.delimiter << output_csv.fields; }
            std::cout << std::endl;
            return 0;
        }
        if( options.exists( "--output-format" ) )
        {
            if( !options.exists( "--binary" ) ) { std::cerr << "csv-repeat: --output-format option requires --binary" << std::endl; return 1; }
            std::cout << options.value< std::string >( "--binary" );
            if( options.exists( "--append-fields,--append,-a" )) { std::cout << csv.delimiter << output_csv.format().string(); }
            std::cout << std::endl;
            return 0;
        }
        boost::optional< boost::posix_time::time_duration > period;
        if( options.exists( "--period" ) ) { period = boost::posix_time::microseconds( static_cast<unsigned int> (options.value< double >( "--period" ) * 1000000 )); }
        boost::posix_time::time_duration timeout;
        boost::optional< double > timeout_seconds = options.optional< double >( "--timeout,-t" );
        if( !period && !timeout_seconds ) { std::cerr << "csv-repeat: please specify either --period, or --timeout, or both" << std::endl; return 1; }
        timeout = timeout_seconds ? boost::posix_time::microseconds( static_cast<unsigned int>(*timeout_seconds * 1000000 )) : *period;
        std::cin.tie( NULL );
        if( options.exists( "--timestamped" ) )
        {
            if( !period ) { std::cerr << "csv-repeat: for --timestamped, please specify --period" << std::endl; return 1; }
            if( options.exists( "--timeout,-t" ) ) { std::cerr << "csv-repeat: for --timestamped: --timeout not supported" << std::endl; return 1; }
            comma::csv::input_stream< input_t > istream( std::cin, csv );
            boost::posix_time::ptime last;
            if( options.exists( "--from" ) ) { last = boost::posix_time::from_iso_string( options.value< std::string >( "--at-least-from,--from" ) ); }
            boost::posix_time::ptime to;
            if( options.exists( "--to" ) ) { to = boost::posix_time::from_iso_string( options.value< std::string >( "--at-least-to,--to" ) ); }
            std::string last_record;
            if( csv.binary() ) { last_record = std::string( csv.format().size(), 0 ); }
            auto pass = [&]( const output_t& )
            {
                static comma::csv::passed< input_t > passed( istream, std::cout, csv.flush );
                passed.write();
            };
            auto append = [&]( const output_t& o )
            {
                static comma::csv::tied< input_t, output_t > tied( istream, *ostream );
                tied.append( o );
            };
            std::function< void( const output_t& p ) > write;
            if( ostream ) { write = append; } else { write = pass; }
            auto write_last = [&]( boost::posix_time::ptime t )
            {
                std::cout.write( &last_record[0], last_record.size() );
                if( !csv.binary() ) { std::cout << csv.delimiter; }
                if( ostream ) { ostream->write( output_t( t, true ) ); }
                else if( !csv.binary() ) { std::cout << std::endl; }
            };
            auto repeat = [&]( boost::posix_time::ptime now )
            {
                if( now.is_not_a_date_time() || last.is_not_a_date_time() ) { return; }
                for( boost::posix_time::ptime t = last + *period; t <= now; )
                {
                    write_last( t );
                    if( t == now ) { break; }
                    t += *period;
                    if( t > now ) { t = now; }
                }
            };
            auto set_last_record = [&]()
            {
                if( csv.binary() ) { std::memcpy( &last_record[0], istream.binary().last(), last_record.size() ); } // todo! quick and dirty, watch performance! we don't need to copy each record, but that would make the code more complex
                else { last_record = comma::join( istream.ascii().last(), csv.delimiter ); }
            };
            while( istream.ready() || std::cin.good() )
            {
                const input_t* p = istream.read();
                if( !p ) { break; }
                if( p->time.is_not_a_date_time() ) { std::cerr << "csv-repeat: expected timestamp, got not a date/time" << std::endl; return 1; }
                if( last_record.empty() && !last.is_not_a_date_time() ) { set_last_record(); write_last( last ); } // quick and dirty
                repeat( p->time );
                write( output_t( p->time, false ) );
                last = p->time;
                set_last_record();
            }
            repeat( to );
            return 0;
        }
        if( options.exists( "--pace" ) )
        {
            if( !period ) { std::cerr << "csv-repeat: for --pace, please specify --period" << std::endl; return 1; }
            std::size_t record_size = csv.binary() ? csv.format().size() : 0;
            if( record_size == 0 )
            {
                while( std::cin.good() && !std::cin.eof() )
                {
                    std::string line;
                    std::getline( std::cin, line );
                    if( line.empty() ) { break; }
                    std::cout << line;
                    if( ostream ) { std::cout << csv.delimiter; ostream->write( output_t( boost::posix_time::microsec_clock::universal_time(), false ) ); }
                    else { std::cout << std::endl; }
                    boost::this_thread::sleep( *period );
                }
            }
            else
            {
                std::vector< char > buf( record_size );
                while( std::cin.good() && !std::cin.eof() ) // todo? quick and dirty; improve reading performance
                {
                    std::cin.read( &buf[0], record_size );
                    if( std::cin.gcount() <= 0 ) { break; }
                    if( std::cin.gcount() < int( record_size ) ) { std::cerr << "csv-repeat: expected " << record_size << " byte(s); got only: " << std::cin.gcount() << std::endl; return 1; }
                    std::cout.write( &buf[0], record_size );
                    if( ostream ) { ostream->write( output_t( boost::posix_time::microsec_clock::universal_time(), false ) ); }
                    std::cout.flush();
                    boost::this_thread::sleep( *period );
                }
            }
            return 0;
        }
        std::size_t record_size = csv.binary() ? csv.format().size() : 0;
        std::vector< char > buffer( csv.binary() ? ( 65536ul / record_size + 1 ) * record_size : 0 );
        char* buffer_begin = &buffer[0];
        const char* buffer_end = &buffer[0] + buffer.size();
        char* read_position = buffer_begin;
        char* write_position = buffer_begin;
        char* last_record = NULL;
        comma::io::select select;
        select.read().add( comma::io::stdin_fd );
        comma::io::istream is( "-", comma::io::mode::binary );
        bool end_of_stream = false;
        std::string line;
        std::string last_line;
        std::ios_base::sync_with_stdio( false ); // unsync to make rdbuf()->in_avail() working
        bool repeating = false;
        unsigned int repeat_count = 0;
        bool pace = options.exists( "--pace" );
        if( pace && !period ) { std::cerr << "csv-repeat: for --pace, please specify --period" << std::endl; return 1; }
        while( is->good() && !end_of_stream )
        {
            select.wait( repeating ? *period : timeout );
            repeating = true;
            while( is->rdbuf()->in_avail() > 0 || ( select.check() && select.read().ready( is.fd() ) ) )
            {
                end_of_stream = true;
                if( csv.binary() )
                {
                    std::size_t available = is->rdbuf()->in_avail(); //std::size_t available = is.available_on_file_descriptor();
                    std::size_t bytes_to_read = std::min( available, ( std::size_t )( buffer_end - read_position ));
                    is->read( read_position, bytes_to_read );
                    std::size_t bytes_read = is->gcount();
                    if( bytes_read <= 0 ) { break; }
                    read_position += bytes_read;
                    while( read_position - write_position >= ( std::ptrdiff_t )record_size )
                    {
                        std::cout.write( write_position, record_size );
                        /// do not do it! see the note inside csv::stream.h, search for passed<> class template
                        /// ::write( 1, write_position, record_size );
                        if( ostream ) { ostream->write( output_t( boost::posix_time::microsec_clock::universal_time(), false ) ); }
                        else { std::cout.flush(); }  // TODO: why?
                        last_record = write_position;
                        write_position += record_size;
                    }
                    if( read_position == buffer_end ) { read_position = write_position = buffer_begin; }
                }
                else
                {
                    if( ignore_eof ) { last_line = line; }
                    std::getline( std::cin, line );
                    if( line.empty() ) { break; }
                    std::cout << line;
                    if( ostream )
                    {
                        std::cout << csv.delimiter;
                        ostream->write( output_t( boost::posix_time::microsec_clock::universal_time(), false ) );
                    }
                    else { std::cout << std::endl; }
                }
                end_of_stream = repeating = false;
                repeat_count = 0;
                if( pace ) { boost::this_thread::sleep( *period ); } // todo: quick and dirty; fix it properly for --pace, to make sure sleep happens after each record only once
            }
            if( !is->good() || end_of_stream ) { break; }
            if( repeating )
            {
                if( !period ) { std::cerr << "csv-repeat: input data timed out" << std::endl; return 1; }
                repeat_count++;
                if( csv.binary() )
                {
                    if( last_record )
                    {
                        std::cout.write( last_record, record_size );
                        /// do not do it! see the note inside csv::stream.h, search for passed<> class template
                        /// ::write( 1, last_record, record_size );
                        if( ostream ) { ostream->write( output_t( boost::posix_time::microsec_clock::universal_time(), true, repeat_count ) ); }
                    }
                }
                else
                {
                    if( !line.empty() )
                    {
                        std::cout << line;
                        if( ostream )
                        {
                            std::cout << csv.delimiter;
                            ostream->write( output_t( boost::posix_time::microsec_clock::universal_time(), true, repeat_count ) );
                        }
                        else { std::cout << std::endl; }
                    }
                }
                std::cout.flush();
            }
        }
        if( ignore_eof )
        {
            comma::signal_flag is_shutdown;
            while( !is_shutdown )
            {
                boost::this_thread::sleep( *period ); // quick and dirty
                if( is_shutdown ) { break; }
                repeat_count++;
                if( csv.binary() )
                {
                    if( !last_record ) { break; }
                    std::cout.write( last_record, record_size );
                    /// do not do it! see the note inside csv::stream.h, search for passed<> class template
                    /// ::write( 1, last_record, record_size );
                    if( ostream ) { ostream->write( output_t( boost::posix_time::microsec_clock::universal_time(), true, repeat_count ) ); }
                }
                else
                {
                    if( last_line.empty() ) { break; }
                    std::cout << last_line;
                    if( ostream ) { std::cout << csv.delimiter; ostream->write( output_t( boost::posix_time::microsec_clock::universal_time(), true, repeat_count ) ); }
                    else { std::cout << std::endl; }
                }
                std::cout.flush();
            }
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-repeat: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-repeat: unknown exception" << std::endl; }
    return 1;
}
