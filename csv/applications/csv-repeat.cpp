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
#include "../../io/select.h"
#include "../../io/stream.h"

static const unsigned long min_buffer_size = 65536ul;

static void bash_completion( unsigned const ac, char const * const * av )
{
    static const char* completion_options =
        " --help -h"
        " --size --timeout --period"
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
    std::cerr << "    --timeout,-t=[<seconds>]: timeout before repeating the last record" << std::endl;
    std::cerr << "    --period=[<seconds>]: period of repeated record" << std::endl;
    std::cerr << "    --size=[<bytes>]: specify size of one record of input data" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    if --timeout and --period are not set stdin is just echoed to stdout" << std::endl;
    std::cerr << "    if --size is not specified data is assumed to be ascii" << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    { echo -e \"1\\n2\\n3\"; sleep 10; } | csv-repeat --timeout=3 --period=1" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    { echo -e \"1,2,3\\n4,5,6\\n7,8,9\"; sleep 10; } \\" << std::endl;
    std::cerr << "        | csv-to-bin 3d \\" << std::endl;
    std::cerr << "        | csv-repeat --timeout=3 --period=1 --size=$( csv-size 3d ) \\" << std::endl;
    std::cerr << "        | csv-from-bin 3d" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        if( options.exists( "--bash-completion" ) ) bash_completion( ac, av );

        boost::optional< std::size_t > record_size;
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

        comma::io::select select;
        select.read().add( comma::io::stdin_fd );
        comma::io::istream is( "-", comma::io::mode::binary );

        comma::signal_flag is_shutdown;
        bool end_of_stream = false;

        std::vector< char > buffer;
        buffer.resize( buffer_size );

        std::size_t bytes_buffered = 0;
        const char* start_of_string = NULL;
        std::size_t length_of_string = 0;
        bool repeating = false;
        bool last_record_valid = false;

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
                    bytes_buffered = std::min( available, buffer.size() );
                    is->read( &buffer[0], bytes_buffered );
                    bytes_buffered = is->gcount();
                    if( bytes_buffered <= 0 ) { break; }
                    available -= bytes_buffered;
                    std::cout.write( &buffer[0], bytes_buffered );
                    end_of_stream = repeating = last_record_valid = false;
                }
                std::cout.flush();
            }

            if( repeating )
            {
                if( !last_record_valid && bytes_buffered > 0 )
                {
                    if( record_size )
                    {
                        length_of_string = *record_size;
                        start_of_string = &buffer[ bytes_buffered - length_of_string ];
                    }
                    else
                    {
                        // Find the last ascii string, by looking for the second-last \n
                        start_of_string = (char *)memrchr( &buffer[0], '\n', bytes_buffered - 1 );
                        if( start_of_string ) { start_of_string++; } else { start_of_string = &buffer[0]; }
                        length_of_string = &buffer[ bytes_buffered ] - start_of_string;
                    }
                    last_record_valid = true;
                }
                if( last_record_valid )
                {
                    std::cout.write( start_of_string, length_of_string );
                    std::cout.flush();
                }
            }
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-repeat: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-repeat: unknown exception" << std::endl; }
    return 1;
}
