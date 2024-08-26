// This file is part of comma, a generic and flexible library
// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2024 Vsevolod Vlaskine
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
#include <sstream>
#include <type_traits>
#include <boost/array.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>
#include "../../application/command_line_options.h"
#include "../../application/signal_flag.h"
#include "../../base/exception.h"
#include "../../base/types.h"
#include "../../csv/format.h"
#include "../../csv/options.h"
#include "../../string/string.h"
#include "detail/publish.h"

static void usage()
{
    std::cerr << R"(
simple udp client: receive udp packets and outputs them on stdout (default)
                   or to given output streams

rationale: netcat and socat somehow do not work very well with udp

usage: udp-client <port> [<output-streams>] [<options>]

attention! it is possible to receive several packets udp packets in a single
           read from the udp socket; the way to deal with it:
               - if fixed-width data is published on the UDP socket, use:
                 udp-client ... --size=<expected-fixed-size>
               - if variable-size data is published on the UDP socket, the data
                 receiver will have to parse the packets depending on communication
                 protocol or nature of the data
               - if using udp-client --fields=size,data or --fields=t,size,data,
                 size field will have the total size in bytes for all UDP packets
                 read from the UDP socket in a single receive call

options
    --ascii; output timestamp as ascii; default: 64-bit binary
    --cache-size,--cache=<n>; default=0; number of cached records; if a new client connects, the
                                         the cached records will be sent to it once connected
    --delimiter=<delimiter>: if ascii and --timestamp, use this delimiter; default: ','
    --discard: not present, do blocking write to every open output stream
    --endl; if --ascii, output '\n' after data
    --fields=<fields>; default=data; choices (for now): 'data', 't,data', 't,size,data', 'size,data'
                                                        't', 't,size', 'size' 
        <fields>
            t: utc timestamp, same as --timestamp
            size: data size in bytes
            data: udp packet data
    --flush; flush stdout after each packet
    --reuse-addr,--reuseaddr: reuse udp address/port
    --size=<size>; default=16384; hint of maximum buffer size in bytes, if using timestamped
                                  fixed-width data, use --size=<fixed-width-size>, otherwise
                                  multiple packets may be read from the UDP socket at once
    --timestamp: deprecated, use --fields; output packet timestamp; currently just system
                 time as UTC; if binary, little endian uint64

output streams: <address>
    <address>
        tcp:<port>: e.g. tcp:1234
        udp:<port>: e.g. udp:1234 (todo)
        local:<name>: linux/unix local server socket e.g. local:./tmp/my_socket
        <named pipe name>: named pipe, which will be re-opened, if client reconnects
        <filename>: a regular file
        -: stdout

examples
    publishing on udp
        ( echo a; echo b; echo c ) | socat - udp:localhost::12345
    basics
        udp-client 12435 > raw.bin
        udp-client 12435 --fields=t,data > timestamped.bin
        udp-client 12435 --fields=t,size,data > timestamp.size.bin
        udp-client 12435 --fields=t,size,data --ascii > timestamp.size.csv
    re-publishing
        udp-client 12435 tcp::4567 
        udp-client 12435 tcp::4567 tcp::7890 
        udp-client 12435 tcp::4567 - > log.bin
)" << std::endl;
    exit( 0 );
}

int main( int argc, char** argv )
{
    try
    {
        comma::command_line_options options( argc, argv );
        if( argc < 2 || options.exists( "--help,-h" ) ) { usage(); }
        const std::vector< std::string >& unnamed = options.unnamed( "--ascii,--binary,--discard,--endl,--flush,--reuse-addr,--reuseaddr,--timestamp", "-.+" );
        COMMA_ASSERT_BRIEF( !unnamed.empty(), "please specify port" );
        std::vector< std::string > output_streams( unnamed.size() > 1 ? unnamed.size() - 1 : 1 );
        if( unnamed.size() == 1 ) { output_streams[0] = "-"; }
        else { std::copy( unnamed.begin() + 1, unnamed.end(), output_streams.begin() ); }
        unsigned short port = boost::lexical_cast< unsigned short >( unnamed[0] );
        options.assert_mutually_exclusive( "--timestamp", "--fields" );
        bool timestamped = options.exists( "--timestamp" );
        if( timestamped ) { comma::say() << "--timestamped: deprecated (will be maintained for now); use --fields=t,data" << std::endl; }
        if( options.exists( "--binary" ) ) { comma::say() << "--binary: deprecated, please remove; data deemed binary anyway unless --ascii specified" << std::endl; }
        bool binary = !options.exists( "--ascii" );
        bool endl = options.exists( "--endl" );
        comma::csv::options csv( options, timestamped ? "t,data" : "data" );
        COMMA_ASSERT_BRIEF(    csv.fields == "data"
                            || csv.fields == "t,data"
                            || csv.fields == "t,size,data"
                            || csv.fields == "size,data"
                            || csv.fields == "t"
                            || csv.fields == "t,size"
                            || csv.fields == "size"
                          , "unsupported fields: '" << csv.fields << "'" ); // uber-quick and dirty, shameful
        bool has_time = csv.has_field( "t" ) || timestamped;
        bool has_size = csv.has_field( "size" );
        bool has_data = csv.has_field( "data" );
        static_assert( sizeof( boost::posix_time::ptime ) == 8 ); // quick and dirty
        unsigned max_size = options.value( "--size", 16384 );
        std::vector< char > buffer( max_size + 12 ); // quick and dirty
        #if BOOST_VERSION >= 106600
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
                                    , options.exists( "--discard" )
                                    , options.exists( "--flush" ) || !binary
                                    , false
                                    , false
                                    , options.value( "--cache-size,--cache", 0 ) );
        comma::signal_flag is_shutdown;
        if( binary )
        {
            unsigned int offset = ( has_time ? 8 : 0 ) + ( has_size ? 4 : 0 ); // hyper-quick and dirty for now
            while( !is_shutdown )
            {
                std::uint32_t size = socket.receive( boost::asio::buffer( &buffer[offset], max_size ), 0, error );
                if( error || size == 0 ) { break; } // todo? throw on error?
                if( has_time ) { comma::csv::format::traits< boost::posix_time::ptime, comma::csv::format::time >::to_bin( boost::posix_time::microsec_clock::universal_time(), &buffer[0] ); }
                if( has_size ) { ::memcpy( &buffer[ has_time ? 8 : 0 ], reinterpret_cast< const char* >( &size ), 4 ); }
                p.write( &buffer[0], offset + ( has_data ? size : 0 ) );
            }
        }
        else
        {
            std::string delimiter;
            while( !is_shutdown )
            {
                std::size_t size = socket.receive( boost::asio::buffer( &buffer[0], max_size ), 0, error );
                if( error || size == 0 ) { break; } // todo? throw on error?
                std::ostringstream oss;
                if( has_time ) { oss << boost::posix_time::to_iso_string( boost::posix_time::microsec_clock::universal_time() ); delimiter = csv.delimiter; }
                if( has_size ) { oss << delimiter << size; delimiter = csv.delimiter; }
                if( has_data ) { oss << delimiter; oss.write( &buffer[0], size ); if( endl ) { oss << std::endl; } }
                const std::string& s = oss.str();
                p.write( &s[0], s.size() );
            }
        }
        return 0;
    }
    catch( std::exception& ex ) { comma::say() << ex.what() << std::endl; }
    catch( ... ) { comma::say() << "unknown exception" << std::endl; }
    return 1;
}
