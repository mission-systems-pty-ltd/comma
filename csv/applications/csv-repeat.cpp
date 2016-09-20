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
#include "../../io/select.h"
#include "../../io/stream.h"

static const unsigned long min_buffer_size = 65536ul;

static void bash_completion( unsigned const ac, char const * const * av )
{
    static const char* completion_options =
        " --help -h --verbose -v"
        " --size --timeout --period"
        " --decorate --local"
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
    std::cerr << "    --decorate=[<fields>]: add extra fields to output" << std::endl;
    std::cerr << "    --local: if present, decorate with local time; default: utc" << std::endl;
    std::cerr << "    --size=[<bytes>]: specify size of one record of input data" << std::endl;
    std::cerr << "    --verbose,-v: more output" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    if --timeout and --period are not set stdin is just echoed to stdout" << std::endl;
    std::cerr << "    if --size or --binary are not specified data is assumed to be ascii" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    --decorate fields are appended to output; supported fields are:" << std::endl;
    std::cerr << "        timestamp: append timestamp" << std::endl;
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
    std::cerr << "        | csv-repeat --timeout=3 --period=1 --decorate=timestamp,repeating" << std::endl;
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

struct decoration_types
{
    enum values { repeating, timestamp };

    static values from_string( const std::string& value_str )
    {
        if( value_str == "repeating" ) { return decoration_types::repeating; }
        else if( value_str == "timestamp" ) { return decoration_types::timestamp; }
        else { COMMA_THROW( comma::exception, "expected decoration type, got " << value_str ); }
    }
};

static boost::optional< std::size_t > record_size;
static comma::csv::options csv;
static std::vector< decoration_types::values > decorations;
static bool local;

static void decorate( bool repeating )
{
    for( std::vector< decoration_types::values >::iterator it = decorations.begin();
         it != decorations.end();
         ++it )
    {
        if( *it == decoration_types::repeating )
        {
            if( record_size )
            {
                static const unsigned int bool_size = comma::csv::format::traits< unsigned char >::size;
                static char repeating_bytes[ bool_size ];
                comma::csv::format::traits< unsigned char >::to_bin( repeating, repeating_bytes );
                std::cout.write(( char* )( &repeating_bytes ), bool_size );
            }
            else { std::cout << csv.delimiter << repeating; }
        }
        else if( *it == decoration_types::timestamp )
        {
            boost::posix_time::ptime now = local
                ? boost::posix_time::microsec_clock::local_time()
                : boost::posix_time::microsec_clock::universal_time();
            if( record_size )
            {
                static const unsigned int time_size = comma::csv::format::traits< boost::posix_time::ptime, comma::csv::format::time >::size;
                static char timestamp[ time_size ];
                comma::csv::format::traits< boost::posix_time::ptime, comma::csv::format::time >::to_bin( now, timestamp );
                std::cout.write(( char* )( &timestamp ), time_size );
            }
            else { std::cout << csv.delimiter << boost::posix_time::to_iso_string( now ); }
        }
    }
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        if( options.exists( "--bash-completion" ) ) bash_completion( ac, av );

        csv = comma::csv::options( options );

        if( csv.binary() ) { record_size = csv.format().size(); }
        if( options.exists( "--size" )) { record_size = options.value< std::size_t >( "--size" ); }
        unsigned int buffer_size = std::max( min_buffer_size, record_size.get_value_or( 0 ));

        boost::optional< boost::posix_time::time_duration > timeout;
        boost::optional< boost::posix_time::time_duration > period;

        if( options.exists( "--timeout,-t" )) { timeout = boost::posix_time::microseconds( options.value< double >( "--timeout,-t" ) * 1000000 ); }
        if( options.exists( "--period" )) { period = boost::posix_time::microseconds( options.value< double >( "--period" ) * 1000000 ); }

        if( !timeout != !period )
        {
            if( timeout ) { COMMA_THROW( comma::exception, "--timeout option requires --period option" ); }
            else { COMMA_THROW( comma::exception, "--period option requires --timeout option" ); }
        }

        if( options.exists( "--decorate" ))
        {
            std::vector< std::string > decoration_values = comma::split( options.value< std::string >( "--decorate" ), "," );
            for( std::vector< std::string >::iterator it = decoration_values.begin();
                 it != decoration_values.end();
                 ++it )
            {
                decorations.push_back( decoration_types::from_string( *it ));
            }
        }
        local = options.exists( "--local" );

        comma::io::select select;
        select.read().add( comma::io::stdin_fd );
        comma::io::istream is( "-", comma::io::mode::binary );

        comma::signal_flag is_shutdown;
        bool end_of_stream = false;

        std::vector< char > buffer;
        buffer.resize( buffer_size );
        char* buffer_begin = NULL;
        const char* buffer_end = NULL;
        char* read_position = NULL;
        char* write_position = NULL;
        char* last_record = NULL;
        if( record_size )
        {
            buffer_begin = &buffer[0];
            buffer_end = buffer_begin + ( buffer.size() / *record_size ) * *record_size;
            write_position = buffer_begin;
            read_position = buffer_begin;
        }

        std::string line;

        bool repeating = false;

        while( !is_shutdown && !end_of_stream )
        {
            if( timeout ) { select.wait( repeating ? *period : *timeout ); }
            else { select.wait(); }

            // We only consider repeating if timeout was set
            if( timeout ) { repeating = true; }

            while( select.check() && select.read().ready( is.fd() ) && is->good()
                   && !end_of_stream && !is_shutdown )
            {
                end_of_stream = true;
                std::size_t available = is.available();
                while( !is_shutdown && available > 0 )
                {
                    if( record_size )
                    {
                        std::size_t bytes_to_read = std::min( available, ( std::size_t )( buffer_end - read_position ));
                        is->read( read_position, bytes_to_read );
                        std::size_t bytes_read = is->gcount();
                        if( bytes_read <= 0 ) { break; }
                        read_position += bytes_read;
                        available -= bytes_read;
                        while( read_position - write_position >= ( std::ptrdiff_t )*record_size )
                        {
                            std::cout.write( write_position, *record_size );
                            decorate( false );
                            last_record = write_position;
                            write_position += *record_size;
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
                        decorate( false );
                        std::cout << '\n';
                    }
                    end_of_stream = repeating = false;
                }
                std::cout.flush();
            }

            if( repeating )
            {
                if( record_size )
                {
                    if( last_record )
                    {
                        std::cout.write( last_record, *record_size );
                        decorate( true );
                    }
                }
                else
                {
                    if( !line.empty() )
                    {
                        std::cout << line;
                        decorate( true );
                        std::cout << '\n';
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
