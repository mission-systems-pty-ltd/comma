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
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by the The University of Sydney.
// 4. Neither the name of the The University of Sydney nor the
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


/// @author cedric wohlleber

#ifdef WIN32
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#endif

#include <iostream>
#include <zmq.hpp>
#include <boost/array.hpp>
#include <boost/program_options.hpp>
#include <comma/application/contact_info.h>
#include <comma/application/signal_flag.h>
#include <comma/io/publisher.h>

static std::string get_endl()
{
    std::ostringstream oss;
    oss << std::endl;
    return oss.str();
}

int main(int argc, char* argv[])
{
    try
    {
        unsigned int size;
        std::size_t hwm;
        std::string server;
        boost::program_options::options_description description( "options" );
        description.add_options()
            ( "help,h", "display help message; --help --verbose for more help" )
            ( "publish", "use bind and publish (as opposed to connect and subscribe)" )
            ( "connect", "use connect instead of bind" )
            ( "bind", "use bind instead of connect" )
            ( "size,s", boost::program_options::value< unsigned int >( &size ), "packet size in bytes, in publish mode; if not present, data is line-based ascii" )
            ( "buffer,b", boost::program_options::value< std::size_t >( &hwm )->default_value( 1024 ), "set buffer size in packets (high water mark in zmq)" )
            ( "server", boost::program_options::value< std::string >( &server ), "in subscribe mode, republish the data on a socket, eg tcp:1234" )
            ( "verbose,v", "more output" );

        boost::program_options::variables_map vm;
        boost::program_options::store( boost::program_options::parse_command_line( argc, argv, description), vm );
        boost::program_options::parsed_options parsed = boost::program_options::command_line_parser(argc, argv).options( description ).allow_unregistered().run();
        boost::program_options::notify( vm );

        if( vm.count( "help" ) )
        {
            std::cerr << std::endl;
            std::cerr << "forward stdin to zeromq publisher or subscribe from zeromq to stdout" << std::endl;
            std::cerr << std::endl;
            std::cerr << "usage: zero-cat <options> [endpoints]" << std::endl;
            std::cerr << std::endl;
            std::cerr << description << std::endl;
            std::cerr << std::endl;
            if( !vm.count( "verbose" ) )
            {
                std::cerr << "connect vs bind..." << std::endl;
                std::cerr << std::endl;
                std::cerr << "examples..." << std::endl;
                std::cerr << std::endl;
                std::cerr << "use --help --verbose for more detail" << std::endl;
                std::cerr << std::endl;
                return 0;
            }
            std::cerr << std::endl;
            std::cerr << "connect vs bind" << std::endl;
            std::cerr << std::endl;
            std::cerr << "    persistent sender (publisher), multiple receivers (subscribers) (default behaviour)" << std::endl;
            std::cerr << std::endl;
            std::cerr << "        use case: publishing sensor data to whoever is interested" << std::endl;
            std::cerr << std::endl;
            std::cerr << "        publisher: listens to incoming connections (binds) and publishes" << std::endl;
            std::cerr << "            example: echo yes | zero-cat --publish ipc:///tmp/some_socket" << std::endl;
            std::cerr << std::endl;
            std::cerr << "        subscribers: listens to incoming connections (binds)" << std::endl;
            std::cerr << "            example: zero-cat ipc:///tmp/some_socket" << std::endl;
            std::cerr << std::endl;
            std::cerr << "    persistent receiver, multiple senders" << std::endl;
            std::cerr << std::endl;
            std::cerr << "        use case: logging server" << std::endl;
            std::cerr << std::endl;
            std::cerr << "        receiver: listens to incoming connections (binds) and receives" << std::endl;
            std::cerr << "            example: zero-cat ipc:///tmp/some_socket --bind > log.txt" << std::endl;
            std::cerr << std::endl;
            std::cerr << "        senders: connect to the receiver and send" << std::endl;
            std::cerr << "            example: echo yes | zero-cat --publish ipc:///tmp/some_socket --connect" << std::endl;
            std::cerr << std::endl;
            std::cerr << "        the difference is that if senders bind (as oppose to connect) and they can send" << std::endl;
            std::cerr << "        data straight away, since zmq is connectionless, even though the receiver has" << std::endl;
            std::cerr << "        not finished connecting to them yet and therefore the initial portion of data" << std::endl;
            std::cerr << "        may be lost" << std::endl;
            std::cerr << std::endl;
            std::cerr << "        example" << std::endl;
            std::cerr << "            in the following case, the receiver most likely will miss the message" << std::endl;
            std::cerr << "            since the sender will send it, before the receiver has got connected:" << std::endl;
            std::cerr << std::endl;
            std::cerr << "            zero-cat ipc:///tmp/some_socket > my-log.txt &" << std::endl;
            std::cerr << "            echo hello world | zero-cat --publish ipc:///tmp/some_socket" << std::endl;
            std::cerr << std::endl;
            std::cerr << "            but this will work:" << std::endl;
            std::cerr << std::endl;
            std::cerr << "            zero-cat ipc:///tmp/some_socket --bind > my-log.txt &" << std::endl;
            std::cerr << "            echo hello world | zero-cat --publish ipc:///tmp/some_socket --connect" << std::endl;
            std::cerr << std::endl;
            std::cerr << std::endl;
            std::cerr << comma::contact_info << std::endl;
            std::cerr << std::endl;
            return 0;
        }
        const bool is_publisher = bool( vm.count( "publish" ) );
        
        const std::vector< std::string >& endpoints = boost::program_options::collect_unrecognized( parsed.options, boost::program_options::include_positional );
        if( endpoints.empty() ) { std::cerr << "zero-cat: please provide at least one endpoint" << std::endl; return 1; }
        comma::signal_flag is_shutdown;
        zmq::context_t context( 1 );
        int mode = is_publisher ? ZMQ_PUB : ZMQ_SUB;
        zmq::socket_t socket( context, mode );
        bool binary = vm.count( "size" );
        #ifdef WIN32
        if( is_publisher && binary ) { _setmode( _fileno( stdin ), _O_BINARY ); }
        #endif
        // Although the documentation says that HWM is supported in ZMQ4, the
        // code shows that if the sock opt is HWM an exception will be thrown.
        #if ZMQ_VERSION_MAJOR == 2
        socket.setsockopt( ZMQ_HWM, &hwm, sizeof( hwm ) );
        #endif
        if( is_publisher )
        {
            bool output_to_stdout = false;
            for( unsigned int i = 0; i < endpoints.size(); i++ )
            {
                if( endpoints[i] == "-" ) { output_to_stdout = true; }
                else if( vm.count( "connect" ) ) { socket.connect( endpoints[i].c_str() ); }
                else { socket.bind( endpoints[i].c_str() ); }
            }
            while( !is_shutdown && std::cin.good() && !std::cin.eof() && !std::cin.bad() )
            {
                std::string buffer;
                if( binary )
                {
                    buffer.resize( size );
                    std::cin.read( &buffer[0], buffer.size() );
                    int count = std::cin.gcount();
                    if( count <= 0 ) { break; }
                    buffer.resize( std::size_t( count ) );
                }
                else
                {
                    std::getline( std::cin, buffer );
                    static const std::string endl = get_endl();
                    if( !is_shutdown && std::cin.good() && !std::cin.eof() && !std::cin.bad() ) { buffer += endl; }
                }
                if( buffer.empty() ) { break; }
                zmq::message_t message( buffer.size() );
                ::memcpy( ( void * )message.data(), &buffer[0], buffer.size() );
                if( !socket.send( message ) ) { std::cerr << "zero-cat: failed to send " << buffer.size() << " bytes; zmq errno: EAGAIN" << std::endl; return 1; }
                if( !output_to_stdout ) { continue; }
                std::cout.write( &buffer[0], buffer.size() );
                if( binary ) { std::cout.flush(); }
            }
        }
        else
        {
            for( unsigned int i = 0; i < endpoints.size(); i++ )
            {
                if( vm.count( "bind" ) ) { socket.bind( endpoints[i].c_str() ); }
                else { socket.connect( endpoints[i].c_str() ); }
            }
            socket.setsockopt( ZMQ_SUBSCRIBE, "", 0 );
            if( vm.count( "server" ) )
            {
                comma::io::publisher publisher( server, comma::io::mode::binary, true, false );
                while( !is_shutdown )
                {
                    zmq::message_t message;
                    socket.recv( &message );
                    publisher.write( ( const char* )message.data(), message.size() );
                }
            }
            else
            {
                while( !is_shutdown && std::cout.good() )
                {
                    zmq::message_t message;
                    socket.recv( &message );
                    std::cout.write( ( const char* )message.data(), message.size() );
                    if( binary ) { std::cout.flush(); }
                }
            }
        }
        return 0;
    }
    catch ( zmq::error_t& e )
    {
        std::cerr << argv[0] << ": zeromq error: " << e.what() << std::endl;
    }
    catch ( std::exception& e )
    {
        std::cerr << argv[0] << ": " << e.what() << std::endl;
    }
    catch ( ... )
    {
        std::cerr << argv[0] << ": unknown exception" << std::endl;
    }
    return 1;
}
