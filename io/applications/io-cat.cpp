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

#ifndef WIN32
#include <stdlib.h>
#include <sys/ioctl.h>
#endif

#include <vector>
#include <boost/asio/ip/udp.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <comma/application/command_line_options.h>
#include <comma/application/signal_flag.h>
#include <comma/base/exception.h>
#include <comma/base/types.h>
#include <comma/io/stream.h>
#include <comma/io/select.h>
#include <comma/string/string.h>

void usage( bool verbose = false )
{
    std::cerr << std::endl;
    std::cerr << "read from a few sources, merge, and output to stdout" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: io-cat <address> [<address>] ... [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options: " << std::endl;
    std::cerr << "    --size,-s=[<size>]: packet size, if multiple sources" << std::endl;
    std::cerr << "    --unbuffered,-u: flush output" << std::endl;
    std::cerr << std::endl;
    std::cerr << "supported address types: tcp, udp, local (unix) sockets, named pipes, files, zmq (todo)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    single stream" << std::endl;
    std::cerr << "        io-cat tcp:localhost:12345" << std::endl;
    std::cerr << "        io-cat udp:12345" << std::endl;
    std::cerr << "        io-cat local:/tmp/socket" << std::endl;
    std::cerr << "        io-cat some/pipe" << std::endl;
    std::cerr << "        io-cat some/file" << std::endl;
    std::cerr << "        io-cat zmq-local:/tmp/socket (not implemented)" << std::endl;
    std::cerr << "        io-cat zmq-tcp:localhost:12345 (not implemented)" << std::endl;
    std::cerr << "    multiple streams" << std::endl;
    std::cerr << "        todo" << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

class stream
{
    public:
        stream( const std::string address ) : address_( address ) {}
        
        virtual ~stream() {}
        
        virtual unsigned int read( std::vector< char >& buffer ) = 0;
        
        virtual comma::io::file_descriptor fd() const = 0;
        
        virtual bool eof() const = 0;
        
        const std::string& address() const { return address_; }
        
    private:
        std::string address_;
};

class udp_stream : public stream
{
    public:
        udp_stream( const std::string& address ) : stream( address ), socket_( service_ )
        {
            const std::vector< std::string >& v = comma::split( address, ':' );
            if( v.size() != 2 ) { COMMA_THROW( comma::exception, "io-cat: expected udp:<port>, e.g. udp:12345, got" << address ); }
            unsigned short port = boost::lexical_cast< unsigned short >( v[1] );
            socket_.open( boost::asio::ip::udp::v4() );
            boost::system::error_code error;
            socket_.set_option( boost::asio::ip::udp::socket::broadcast( true ), error );
            if( error ) { COMMA_THROW( comma::exception, "io-cat: udp failed to set broadcast option on port " << port ); }
            socket_.bind( boost::asio::ip::udp::endpoint( boost::asio::ip::udp::v4(), port ), error );
            if( error ) { COMMA_THROW( comma::exception, "io-cat: udp failed to bind port " << port ); }
        }
        
        bool eof() const { return false; }
        
        comma::io::file_descriptor fd() const { return socket_.native_handle(); }
        
        unsigned int read( std::vector< char >& buffer )
        {
            boost::system::error_code error;
            std::size_t size = socket_.receive( boost::asio::buffer( buffer ), 0, error );
            return error ? 0 : size;
        }
        
    private:
        boost::asio::io_service service_;
        mutable boost::asio::ip::udp::socket socket_; // boost::asio::ip::udp::socket::fd() is non-const for some reason
};

class any_stream : public stream
{
    public:
        any_stream( const std::string& address, unsigned int size )
            : stream( address )
            , istream_( address, comma::io::mode::binary, comma::io::mode::non_blocking )
            , size_( size )
            , binary_( size > 0 )
        {
        }
        
        comma::io::file_descriptor fd() const { return istream_.fd(); }
        
        unsigned int read( std::vector< char >& buffer )
        {
            if( binary_ )
            {
                unsigned int available = istream_.available();
                unsigned int size = std::min( std::size_t( size_ ? ( available / size_ ) * size_ : available ), buffer.size() );
                if( size == 0 ) { return 0; }
                istream_->read( &buffer[0], size );
                return istream_->gcount() <= 0 ? 0 : istream_->gcount();
            }
            else
            {
                std::string line; // quick and dirty, no-one expects ascii to be fast
                std::getline( *istream_, line );
                if( line.empty() ) { return 0; }
                if( line.size() > buffer.size() ) { buffer.resize( line.size() ); }
                ::memcpy( &line[0], &buffer[0], line.size() );
                return line.size();
            }
        }
        
        bool eof() const { return !istream_->good() || istream_->eof(); std::cin.good(); }
        
    private:
        comma::io::istream istream_;
        unsigned int size_;
        bool binary_;
};

stream* make_stream( const std::string& address, unsigned int size )
{
    const std::vector< std::string >& v = comma::split( address, ':' );
    if( v[0] == "udp" ) { return new udp_stream( address ); }
    if( v[0] == "zmq-local" || v[0] == "zero-local" || v[0] == "zmq-tcp" || v[0] == "zero-tcp" ) { COMMA_THROW( comma::exception, "io-cat: zmq support not implemented" ); }
    return new any_stream( address, size );
}

int main( int argc, char** argv )
{
    #ifdef WIN32
    std::cerr << "io-cat: not implemented on windows" << std::endl;
    return 1;
    #endif
    if( argc < 2 ) { usage(); }
    comma::command_line_options options( argc, argv, usage );
    unsigned int size = options.value( "--size,-s", 0 );
    bool unbuffered = options.exists( "--unbuffered,-u" );
    const std::vector< std::string >& unnamed = options.unnamed( "--unbuffered,-u", "-.*" );
    #ifdef WIN32
    if( size || unnamed.size() == 1 ) { _setmode( _fileno( stdout ), _O_BINARY ); }
    #endif
    if( unnamed.empty() ) { std::cerr << "io-cat: please specify at least one source" << std::endl; return 1; }
    boost::ptr_vector< stream > streams;
    comma::io::select select;
    for( unsigned int i = 0; i < unnamed.size(); ++i )
    { 
        streams.push_back( make_stream( unnamed[i], size ) );
        select.read().add( streams.back() );
    }
    std::vector< char > buffer( 65536 );
    comma::signal_flag is_shutdown;
    while( !is_shutdown )
    {
        bool done = true;
        for( unsigned int i = 0; i < streams.size() && done; done = streams[i].eof(), ++i );
        if( done ) { break; }
        select.wait( 1 );
        for( unsigned int i = 0; i < streams.size() && done; ++i )
        {
            if( streams[i].eof() || !select.read().ready( streams[i].fd() ) ) { continue; }
            while( !is_shutdown && !streams[i].eof() )
            {
                unsigned int count = streams[i].read( buffer );
                if( count == 0 ) { break; }
                if( size && count % size != 0 ) { std::cerr << "io-cat: expected " << size << " byte(s), got only " << ( count % size ) << " on " << streams[i].address() << std::endl; return 1; }
                std::cout.write( &buffer[0], count );
                if( unbuffered ) { std::cout.flush(); }
            }
        }
    }
}
