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

#ifndef WIN32
#include <stdlib.h>
#endif
#include <iostream>
#include <type_traits>
#include <boost/array.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>
#include "../../application/command_line_options.h"
#include "../../base/exception.h"
#include "../../base/types.h"
#include "../../csv/format.h"
#include "detail/publish.h"

static void usage()
{
    std::cerr << R"(
simple udp client: receive udp packets and outputs them on stdout (default)
                   or to given output streams

rationale: netcat and socat somehow do not work very well with udp

usage: udp-client <port> [<output-streams>] [<options>]

options
    --ascii; output timestamp as ascii; default: 64-bit binary
    --binary; output timestamp as 64-bit binary; default
    --cache-size,--cache=<n>; default=0; number of cached records; if a new client connects, the
                                         the cached records will be sent to it once connected
    --delimiter=<delimiter>: if ascii and --timestamp, use this delimiter; default: ','
    --flush; flush stdout after each packet
    --no-discard: if present, do blocking write to every open output stream
    --size=<size>; hint of maximum buffer size; default 16384
    --reuse-addr,--reuseaddr: reuse udp address/port
    --timestamp: output packet timestamp (currently just system time)

output streams: <address>
    <address>
        tcp:<port>: e.g. tcp:1234
        udp:<port>: e.g. udp:1234 (todo)
        local:<name>: linux/unix local server socket e.g. local:./tmp/my_socket
        <named pipe name>: named pipe, which will be re-opened, if client reconnects
        <filename>: a regular file
        -: stdout

examples
    todo
)" << std::endl;
    exit( 0 );
}

int main( int argc, char** argv )
{
    try
    {
        comma::command_line_options options( argc, argv );
        if( argc < 2 || options.exists( "--help,-h" ) ) { usage(); }
        const std::vector< std::string >& unnamed = options.unnamed( "--ascii,--binary,--flush,--no-discard,--reuse-addr,--reuseaddr,--timestamp", "--delimiter,--size" );
        COMMA_ASSERT_BRIEF( !unnamed.empty(), "please specify port" );
        std::string publish_address = unnamed.size() == 2 ? "-" : unnamed[1];
        COMMA_ASSERT_BRIEF( publish_address == "-", "publish: todo; got: '" << publish_address << "'" );
        std::vector< std::string > output_streams( unnamed.size() > 1 ? unnamed.size() - 1 : 1 );
        if( unnamed.size() == 1 ) { output_streams[0] = "-"; }
        else { std::copy( unnamed.begin() + 1, unnamed.end(), output_streams.begin() ); }
        unsigned short port = boost::lexical_cast< unsigned short >( unnamed[0] );
        bool timestamped = options.exists( "--timestamp" );
        bool binary = !options.exists( "--ascii" );
        char delimiter = options.value( "--delimiter", ',' );
        std::vector< char > packet( options.value( "--size", 16384 ) );
        #if ( BOOST_VERSION >= 106600 )
            boost::asio::io_context service;
        #else
            boost::asio::io_service service;
        #endif
        boost::asio::ip::udp::socket socket( service );
        socket.open( boost::asio::ip::udp::v4() );
        boost::system::error_code error;
        socket.set_option( boost::asio::ip::udp::socket::broadcast( true ), error );
        COMMA_ASSERT_BRIEF( !bool( error ), "failed to set broadcast option on port " << port );
        if( options.exists( "--reuse-addr,--reuseaddr" ) )
        {
            socket.set_option( boost::asio::ip::udp::socket::reuse_address( true ), error );
            COMMA_ASSERT_BRIEF( !bool( error ), "failed to set reuse address option on port " << port );
        }
        socket.bind( boost::asio::ip::udp::endpoint( boost::asio::ip::udp::v4(), port ), error );
        COMMA_ASSERT_BRIEF( !bool( error ), "failed to bind port " << port );
        #ifdef WIN32
        if( binary ) { _setmode( _fileno( stdout ), _O_BINARY ); }
        #endif
        static_assert( sizeof( boost::posix_time::ptime ) == sizeof( comma::uint64 ), "expected time of size 8" );
        comma::io::detail::publish p( output_streams
                                    , options.value( "-s,--size", 0 ) * options.value( "-m,--multiplier", 1 )
                                    , !options.exists( "--no-discard" )
                                    , options.exists( "--flush" )
                                    , false
                                    , false
                                    , options.value( "--cache-size,--cache", 0 ) );
        while( std::cout.good() )
        {
            boost::system::error_code error;
            std::size_t size = socket.receive( boost::asio::buffer( packet ), 0, error );
            if( error || size == 0 ) { break; }
            if( timestamped )
            {
                boost::posix_time::ptime timestamp = boost::posix_time::microsec_clock::universal_time();
                if( binary )
                { 
                    static char buf[ sizeof( comma::int64 ) ];
                    comma::csv::format::traits< boost::posix_time::ptime, comma::csv::format::time >::to_bin( timestamp, buf );
                    std::cout.write( buf, sizeof( comma::int64 ) );
                }
                else
                {
                    std::cout << boost::posix_time::to_iso_string( timestamp ) << delimiter;
                }
            }
            p.write( &packet[0], size );
        }
        return 0;
    }
    catch( std::exception& ex ) { comma::say() << ex.what() << std::endl; }
    catch( ... ) { comma::say() << "unknown exception" << std::endl; }
    return 1;
}
