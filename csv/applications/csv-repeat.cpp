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

#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "../../application/command_line_options.h"
#include "../../application/contact_info.h"
#include "../../application/signal_flag.h"
#include "../../csv/options.h"
#include "../../csv/stream.h"
#include "../../io/select.h"
#include "../../io/stream.h"

static void bash_completion( unsigned const ac, char const * const * av )
{
    static const char* completion_options =
        " --help -h --verbose -v"
        " --timeout --period"
        " --append-fields --append -a"
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
    std::cerr << "    --timeout,-t=[<seconds>]: timeout before repeating the last record" << std::endl;
    std::cerr << "    --period=[<seconds>]: period of repeated record" << std::endl;
    std::cerr << "    --append-fields,--append,-a=[<fields>]: add extra fields to output" << std::endl;
    std::cerr << "    --verbose,-v: more output" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    if --timeout and --period are not set stdin is just echoed to stdout" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    --append fields are appended to output; supported fields are:" << std::endl;
    std::cerr << "        time: append timestamp" << std::endl;
    std::cerr << "        repeating: 1 if currently repeating" << std::endl;
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
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

struct output_t
{
    boost::posix_time::ptime time;
    bool repeating;
    output_t() : repeating( false ) {}
    output_t( const boost::posix_time::ptime& time, bool repeating ) : time( time ), repeating( repeating ) {}
};

namespace comma { namespace visiting {

template <> struct traits< output_t >
{
    template < typename K, typename V > static void visit( const K&, const output_t& p, V& v )
    {
        v.apply( "time", p.time );
        v.apply( "repeating", p.repeating );
    }
    template < typename K, typename V > static void visit( const K&, output_t& p, V& v )
    {
        v.apply( "time", p.time );
        v.apply( "repeating", p.repeating );
    }
};
    
} } // namespace comma { namespace visiting {

// todo: visiting traits

// todo, Dave
// - --output-fields, --output-format
// - WIN32!!! fix cmake
// - remove is_shutdown?
// - re-enable test
// - test: make period and timeout MUCH shorter
// - if period is not given, exit with error on timeout
// - no period: test; --help: explain it's a watchdog

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        if( options.exists( "--bash-completion" ) ) bash_completion( ac, av );

        static comma::csv::options csv = comma::csv::options( options );

        std::size_t record_size = csv.binary() ? csv.format().size() : 0;
        std::vector< char > buffer( csv.binary() ? ( 65536ul / record_size + 1 ) * record_size : 0 );
        char* buffer_begin = &buffer[0];
        const char* buffer_end = &buffer[0] + buffer.size();
        char* read_position = buffer_begin;
        char* write_position = buffer_begin;
        char* last_record = NULL;

        boost::posix_time::time_duration timeout;
        boost::optional< boost::posix_time::time_duration > period;

        timeout = boost::posix_time::microseconds( options.value< double >( "--timeout,-t" ) * 1000000 );
        if( options.exists( "--period" )) { period = boost::posix_time::microseconds( options.value< double >( "--period" ) * 1000000 ); }

        comma::io::select select;
        select.read().add( comma::io::stdin_fd );
        comma::io::istream is( "-", comma::io::mode::binary );
        boost::scoped_ptr< comma::csv::output_stream< output_t > > ostream;
        if( options.exists( "--append-fields,--append,-a" ) )
        {
            comma::csv::options output_csv;
            output_csv.fields = options.value< std::string >( "--append-fields,--append,-a" );
            if( csv.binary() ) { output_csv.format( comma::csv::format::value< output_t >() ); }
            output_csv.delimiter = csv.delimiter;
            ostream.reset( new comma::csv::output_stream< output_t >( std::cout, csv ) );
        }

        comma::signal_flag is_shutdown;
        bool end_of_stream = false;

        std::string line;

        bool repeating = false;

        while( !is_shutdown && !end_of_stream )
        {
            select.wait( repeating ? *period : timeout );
            repeating = true;
            while( select.check() && select.read().ready( is.fd() ) && is->good()
                   && !end_of_stream && !is_shutdown )
            {
                end_of_stream = true;
                std::size_t available = is.available();
                while( !is_shutdown && available > 0 )
                {
                    if( csv.binary() )
                    {
                        std::size_t bytes_to_read = std::min( available, ( std::size_t )( buffer_end - read_position ));
                        is->read( read_position, bytes_to_read );
                        std::size_t bytes_read = is->gcount();
                        if( bytes_read <= 0 ) { break; }
                        read_position += bytes_read;
                        available -= bytes_read;
                        while( read_position - write_position >= ( std::ptrdiff_t )record_size )
                        {
                            std::cout.write( write_position, record_size );
                            if( ostream ) { ostream->write( output_t( boost::posix_time::microsec_clock::universal_time(), false ) ); }
                            last_record = write_position;
                            write_position += record_size;
                        }
                        if( read_position == buffer_end )
                        {
                            read_position = write_position = buffer_begin;
                        }
                    }
                    else
                    {
                        std::getline( std::cin, line );
                        if( line.empty() ) { break; }
                        available -= ( line.size() + 1 );
                        std::cout << line;
                        if( ostream )
                        {
                            std::cout << csv.delimiter;
                            ostream->write( output_t( boost::posix_time::microsec_clock::universal_time(), false ) );
                        }
                        else { std::cout << std::endl; }
                    }
                    end_of_stream = repeating = false;
                }
                std::cout.flush();
            }

            if( repeating )
            {
                if( !period ) { std::cerr << "csv-repeat: input data timed out" << std::endl; return 1; }
                if( csv.binary() )
                {
                    if( last_record )
                    {
                        std::cout.write( last_record, record_size );
                        if( ostream ) { ostream->write( output_t( boost::posix_time::microsec_clock::universal_time(), true ) ); }
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
                            ostream->write( output_t( boost::posix_time::microsec_clock::universal_time(), true ) );
                        }
                        else { std::cout << std::endl; }
                    }
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
