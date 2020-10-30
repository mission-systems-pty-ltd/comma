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
#include <boost/thread.hpp>
#include "../../application/signal_flag.h"
#include "../../io/publisher.h"
#include "../../string/string.h"

void usage( boost::program_options::options_description const & description, bool const verbose )
{
    std::cerr << "\nforward stdin to zeromq publisher or subscribe from zeromq to stdout"
                 "\n"
                 "\nusage: zero-cat <options> [endpoints]"
                 "\n" << description
                << std::endl;
    if( verbose )
    {
        std::cerr << "\nconnect vs bind..."
                     "\nexamples..."
                     "\nuse --help --verbose for more detail"
                    << std::endl;
        return;
    }
    std::cerr << "\nconnect vs bind"
                 "\n"
                 "\n    persistent sender (publisher), multiple receivers (subscribers) (default behaviour)"
                 "\n        use case: publishing sensor data to whoever is interested"
                 "\n"
                 "\n        publisher: listens to incoming connections (binds) and publishes"
                 "\n            example: echo yes | zero-cat --publish ipc:///tmp/some_socket"
                 "\n"
                 "\n        subscribers: listens to incoming connections (binds)"
                 "\n            example: zero-cat ipc:///tmp/some_socket"
                 "\n"
                 "\n    persistent receiver, multiple senders"
                 "\n"
                 "\n        use case: logging server"
                 "\n"
                 "\n        receiver: listens to incoming connections (binds) and receives"
                 "\n            example: zero-cat ipc:///tmp/some_socket --bind > log.txt"
                 "\n"
                 "\n        senders: connect to the receiver and send"
                 "\n            example: echo yes | zero-cat --publish ipc:///tmp/some_socket --connect"
                 "\n"
                 "\n        the difference is that if senders bind (as oppose to connect) and they"
                 "\n        can send data straight away, since zmq is connectionless, even though the"
                 "\n        receiver has not finished connecting to them yet and therefore the initial"
                 "\n        portion of data may be lost"
                 "\n"
                 "\n        example"
                 "\n            in the following case, the receiver most likely will miss the message"
                 "\n            since the sender will send it, before the receiver has got connected:"
                 "\n"
                 "\n            zero-cat ipc:///tmp/some_socket > my-log.txt &"
                 "\n            echo hello world | zero-cat --publish ipc:///tmp/some_socket"
                 "\n"
                 "\n            but this will work:"
                 "\n"
                 "\n            zero-cat ipc:///tmp/some_socket --bind > my-log.txt &"
                 "\n            echo hello world | zero-cat --publish ipc:///tmp/some_socket --connect"
                 "\n"
                 "\nwait after connect"
                 "\n"
                 "\n    All inter-process and network protocols require some time to establish the"
                 "\n    channel, and there is no way with the zmq connectionless design to"
                 "\n    determine if a channel is open. As such, if message delivery is required"
                 "\n    the user will need to specify a wait period after connection before"
                 "\n    messages are sent. The user will need to tune the wait for the system."
                 "\n    During testing on a 2014 PC, 0.2 seconds was adequate for Linux IPC."
                 "\n    This problem was uncovered because data from a pipe was not being sent."
                 "\n    Examples:"
                 "\n        zero-cat ipc:///tmp/some_socket > msglog & "
                 "\n        for i in seq 10; do echo Try $i | zero-cat --wait-after-connect=0.2 --publish ipc:///tmp/some_socket; done"
                 "\n        # note reciever is still running use fg"
                 "\n   Notes:"
                 "\n        If outputting messages to a single pipe it is better to keep zero-cat"
                 "\n        running to minimise load and maximise throughput."
                 "\n"
                << std::endl;
}

static std::string get_endl()
{
    std::ostringstream oss;
    oss << std::endl;
    return oss.str();
}

static const std::string endl = get_endl();

static bool quiet_interrupt = false;

int main(int argc, char* argv[])
{
    try
    {
        unsigned int size;
        double wait_after_connect = 0.0;
        std::size_t hwm;
        std::string server;
        boost::program_options::options_description description( "options" );
        description.add_options()
            ( "help,h", "display help message; --help --verbose for more help" )
            ( "publish", "use bind and publish (as opposed to connect and subscribe)" )
            ( "connect", "use connect instead of bind" )
            ( "bind", "use bind instead of connect" )
            ( "request", "request/reply client" )
            ( "reply", "request/reply server" )
            ( "size,s", boost::program_options::value< unsigned int >( &size ), "packet size in bytes, in publish, request, or reply mode; if not present, data is line-based ascii" )
            ( "buffer,b", boost::program_options::value< std::size_t >( &hwm )->default_value( 1024 ), "set buffer size in packets (high water mark in zmq)" )
            ( "server", boost::program_options::value< std::string >( &server ), "in subscribe mode, republish the data on a socket, eg tcp:1234" )
            ( "wait-after-connect,conwait", boost::program_options::value< double >( &wait_after_connect ), "time to wait, in seconds, after initial connection before attempting to read or write" )
            ( "quiet-interrupt", "suppress error messages due to interrupt" )
            ( "verbose,v", "more output" );

        boost::program_options::variables_map vm;
        boost::program_options::store( boost::program_options::parse_command_line( argc, argv, description), vm );
        boost::program_options::parsed_options parsed = boost::program_options::command_line_parser(argc, argv).options( description ).allow_unregistered().run();
        boost::program_options::notify( vm );
        if( vm.count( "help" ) )
        {
            usage( description, !vm.count( "verbose" ) );
            return 0;
        }
        const std::vector< std::string >& endpoints = boost::program_options::collect_unrecognized( parsed.options, boost::program_options::include_positional );
        if( endpoints.empty() ) { std::cerr << "zero-cat: please provide at least one endpoint" << std::endl; return 1; }
        bool binary = vm.count( "size" );
        quiet_interrupt = vm.count( "quiet-interrupt" );
        comma::signal_flag is_shutdown;
        zmq::context_t context( 1 );
        const bool is_request = vm.count( "request" );
        const bool is_reply = vm.count( "reply" );
        const bool is_publisher = bool( vm.count( "publish" ) );
        if( is_request + is_reply + is_publisher > 1 ) { std::cerr << "zero-cat: expected only one of: --publisher, --request, --reply; got " << ( is_request + is_reply + is_publisher ) << std::endl; return 1; }
        if( is_request || is_reply )
        {
            #ifdef WIN32
            if( binary ) { _setmode( _fileno( stdin ), _O_BINARY ); _setmode( _fileno( stdout ), _O_BINARY ); }
            #endif
            zmq::socket_t socket( context, is_request ? ZMQ_REQ : ZMQ_REP );
            #if ZMQ_VERSION_MAJOR == 2
            socket.setsockopt( ZMQ_HWM, &hwm, sizeof( hwm ) );
            #endif
            if( endpoints.size() != 1 ) { std::cerr << "zero-cat: request/reply server/client expected 1 endpoint, got " << endpoints.size() << ": " << comma::join( endpoints, ',' ) << std::endl; return 1; }
            if( is_request || vm.count( "connect" ) ) { socket.connect( &endpoints[0][0] ); }
            else if( is_reply || vm.count( "bind" ) ) { socket.bind( &endpoints[0][0] ); }
            if( is_request )
            {
                std::string buffer;
                if( binary ) { buffer.resize( size ); }
                while( !is_shutdown && std::cin.good() )
                {
                    
                    if( binary )
                    {
                        std::cin.read( &buffer[0], size );
                        int count = std::cin.gcount();
                        if( count == 0 ) { break; }
                        if( count < int( size ) ) { std::cerr << "zero-cat: expected " << size << " byte(s), got: " << count << std::endl; return 1; }
                    }
                    else
                    {
                        std::getline( std::cin, buffer );
                        if( buffer.empty() ) { break; }
                        buffer += endl;
                    }
                    zmq::message_t request( buffer.size() );
                    ::memcpy( ( void * )request.data(), &buffer[0], buffer.size() );
                    #if ZMQ_VERSION_MAJOR == 2
                    if( !socket.send( request ) ) { std::cerr << "zero-cat: failed to send " << buffer.size() << " bytes; zmq errno: EAGAIN" << std::endl; return 1; }
                    #else // ZMQ_VERSION_MAJOR == 2
                    if( !socket.send( &buffer[0], buffer.size() ) ) { std::cerr << "zero-cat: failed to send " << buffer.size() << " bytes; zmq errno: EAGAIN" << std::endl; return 1; }
                    #endif // ZMQ_VERSION_MAJOR == 2
                    zmq::message_t reply;
                    if( !socket.recv( &reply ) ) { break; }
                    std::cout.write( reinterpret_cast< const char* >( reply.data() ), reply.size() );
                    if( binary ) { std::cout.flush(); }
                }
            }
            else
            {
                std::string buffer;
                if( binary ) { buffer.resize( size ); }
                while( !is_shutdown && std::cin.good() )
                {
                    zmq::message_t request;
                    if( !socket.recv( &request ) ) { break; }
                    std::cout.write( reinterpret_cast< const char* >( request.data() ), request.size() );
                    if( binary ) { std::cout.flush(); }
                    if( binary )
                    {
                        std::cin.read( &buffer[0], size );
                        int count = std::cin.gcount();
                        if( count == 0 ) { break; }
                        if( count < int( size ) ) { std::cerr << "zero-cat: expected " << size << " byte(s), got: " << count << std::endl; return 1; }
                    }
                    else
                    {
                        std::getline( std::cin, buffer );
                        if( buffer.empty() ) { break; }
                        buffer += endl;
                    }
                    zmq::message_t reply( buffer.size() );
                    ::memcpy( ( void * )reply.data(), &buffer[0], buffer.size() );
                    #if ZMQ_VERSION_MAJOR == 2
                    if( !socket.send( reply ) ) { std::cerr << "zero-cat: failed to send " << buffer.size() << " bytes; zmq errno: EAGAIN" << std::endl; return 1; }
                    #else // ZMQ_VERSION_MAJOR == 2
                    if( !socket.send( &buffer[0], buffer.size() ) ) { std::cerr << "zero-cat: failed to send " << buffer.size() << " bytes; zmq errno: EAGAIN" << std::endl; return 1; }
                    #endif // ZMQ_VERSION_MAJOR == 2
                }
            }
            return 0;
        }
        int mode = is_publisher ? ZMQ_PUB : ZMQ_SUB;
        zmq::socket_t socket( context, mode );
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
                else { socket.bind( &endpoints[i][0] ); }
            }
            // we convert to milliseconds as converting to second floors the number so 0.99 becomes 0
            if( wait_after_connect > 0 ) { boost::this_thread::sleep(boost::posix_time::milliseconds( static_cast< long >( wait_after_connect * 1000.0 ) ) ); }
            
            std::string buffer;
            if( binary ) { buffer.resize( size ); }
            while( !is_shutdown && std::cin.good() && !std::cin.eof() && !std::cin.bad() )
            {
                if( binary )
                {                    
                    std::cin.read( &buffer[0], buffer.size() );
                    int count = std::cin.gcount();
                    if( count <= 0 ) { break; }
                    buffer.resize( std::size_t( count ) );
                }
                else
                {
                    std::getline( std::cin, buffer );
                    if( !is_shutdown && std::cin.good() && !std::cin.eof() && !std::cin.bad() ) { buffer += endl; }
                }
                if( buffer.empty() ) { break; }
                #if ZMQ_VERSION_MAJOR == 2
                zmq::message_t message( buffer.size() );
                ::memcpy( ( void * )message.data(), &buffer[0], buffer.size() );
                if( !socket.send( message ) ) { std::cerr << "zero-cat: failed to send " << buffer.size() << " bytes; zmq errno: EAGAIN" << std::endl; return 1; }
                #else // ZMQ_VERSION_MAJOR == 2
                if( !socket.send( &buffer[0], buffer.size() ) ) { std::cerr << "zero-cat: failed to send " << buffer.size() << " bytes; zmq errno: EAGAIN" << std::endl; return 1; }
                #endif // ZMQ_VERSION_MAJOR == 2
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
            if( wait_after_connect > 0 ) { boost::this_thread::sleep( boost::posix_time::milliseconds( static_cast< long >( wait_after_connect * 1000.0 ) ) ); }
            if( vm.count( "server" ) )
            {
                comma::io::publisher publisher( server, comma::io::mode::binary, true, false );
                while( !is_shutdown )
                {
                    zmq::message_t message;
                    socket.recv( &message );
                    publisher.write( reinterpret_cast< const char* >( message.data() ), message.size() );
                }
            }
            else
            {
                while( !is_shutdown && std::cout.good() )
                {
                    zmq::message_t message;
                    socket.recv( &message );
                    std::cout.write( reinterpret_cast< const char* >( message.data() ), message.size() );
                    if( binary ) { std::cout.flush(); }
                }
            }
        }
        return 0;
    }
    catch ( zmq::error_t& e )
    {
        if( !quiet_interrupt || e.num() != EINTR ) { std::cerr << argv[0] << ": zeromq error: (" << e.num() << ") " << e.what() << std::endl; }
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
