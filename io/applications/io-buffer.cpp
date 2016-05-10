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

/// @author dewey nguyen

#ifndef WIN32
#include <stdlib.h>
#include <sys/ioctl.h>
#endif

#include <cctype>
#include <vector>
#include <fstream>
#include <boost/filesystem/operations.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/algorithm/string.hpp>
#include <comma/application/command_line_options.h>
#include <comma/application/contact_info.h>
#include <comma/application/signal_flag.h>
#include <comma/base/exception.h>
#include <comma/base/types.h>
#include <comma/io/stream.h>
#include <comma/io/select.h>
#include <comma/string/string.h>

void usage( bool verbose = false )
{
    std::cerr << std::endl;
    std::cerr << "buffer stdin data to synchronised output data to stdout using a file lock" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: io-buffer <in|out> --lock-file <file> [--buffer-size <size>] [--lines <num>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "<address>" << std::endl;
    std::cerr << "    local:<path>: local socket" << std::endl;
    std::cerr << "    tcp:<host>:<port>: tcp socket" << std::endl;
    std::cerr << "    udp:<port>: udp socket" << std::endl;
    std::cerr << "    zmp-<protocol>:<address>: zmq (todo)" << std::endl;
    std::cerr << "    <filename>: file" << std::endl;
    std::cerr << "    <fifo>: named pipe" << std::endl;
    std::cerr << "    -: stdin" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --flush,unbuffered,-u: flush output" << std::endl;
    std::cerr << "    --round-robin=[<number of packets>]: todo: only for multiple inputs: read not more" << std::endl;
    std::cerr << "                                         than <number of packets> from an input at once," << std::endl;
    std::cerr << "                                         before checking other inputs" << std::endl;
    std::cerr << "                                         if not specified, read from each input" << std::endl;
    std::cerr << "                                         all available data" << std::endl;
    std::cerr << "                                         ignored for udp streams, where one full udp" << std::endl;
    std::cerr << "                                         packet at a time is always read" << std::endl;
    std::cerr << "    --size,-s=[<size>]: packet size, if binary data (required only for multiple sources)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "supported address types: tcp, udp, local (unix) sockets, named pipes, files, zmq (todo)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    single stream" << std::endl;
    std::cerr << "        io-cat tcp:localhost:12345" << std::endl;
    std::cerr << "        io-cat udp:12345" << std::endl;
    std::cerr << "        io-cat local:/tmp/socket" << std::endl;
    std::cerr << "        io-cat some/pipe" << std::endl;
    std::cerr << "        io-cat some/file" << std::endl;
    std::cerr << "        io-cat zmq-local:/tmp/socket (not implemented)" << std::endl;
    std::cerr << "        io-cat zmq-tcp:localhost:12345 (not implemented)" << std::endl;
    std::cerr << "        echo hello | io-cat -" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    multiple streams" << std::endl;
    std::cerr << "        merge line-based input" << std::endl;
    std::cerr << "            io-cat tcp:localhost:55555 tcp:localhost:88888" << std::endl;
    std::cerr << "        merge binary input with packet size 100 bytes" << std::endl;
    std::cerr << "            io-cat tcp:localhost:55555 tcp:localhost:88888 --size 100" << std::endl;
    std::cerr << "        merge line-based input with stdin" << std::endl;
    std::cerr << "            echo hello | io-cat tcp:localhost:55555 -" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

const char* name() { return "io-buffer: "; }


typedef boost::optional< std::vector< std::string > > lines_buffer_type;
typedef std::vector< char >  char_buffer_t;
typedef boost::optional< char_buffer_t > binary_buffer_type;

lines_buffer_type lines_buffer;
binary_buffer_type binary_buffer;

static comma::uint64 get_buffer_size( const std::string& str )
{
    std::string type;
    std::string digits;
    // Get size type, todo use something smarter
    for( std::size_t i=0; i<str.size(); ++i) { 
        if( std::isdigit(str[i]) ) { digits += str[i]; } else  { type = str.substr( i ); break; }
    }
    std::cerr << "digits: " << digits << " type: " << type << std::endl;
    
    comma::uint64 bytes = 0;
    try { bytes = boost::lexical_cast< comma::uint64 >( digits ); } 
    catch(...) { COMMA_THROW( comma::exception, "failed to parse buffer size: " << str ); }
    
    if( type.empty() ){ }
    else if ( boost::iequals(type, "mb") ) { bytes *= 1024000; }
    else if ( boost::iequals(type, "kb") ) { bytes *= 1024; }
    
    return bytes;
}

int main( int argc, char** argv )
{
    try
    {
        if( argc < 2 ) { usage(); }
        comma::command_line_options options( argc, argv, usage );
        comma::uint32 lines_num = options.value( "--lines,-n", 1 );
        std::string lockfile_path = options.value< std::string >( "--lock-file,--lock" );
        boost::optional< comma::uint32 > has_size = options.optional< comma::uint32 >( "--size,s" );
        boost::optional< std::string > buffer_size_string = options.optional< std::string >( "--buffer-size,-b" );
        const std::vector< std::string >& operation = options.unnamed( "--lines,--buffer-size,--lock-file,--lock", "-.+" );
        
        bool verbose = options.value( "--verbose,-v", true );
        if( verbose ) { std::cerr << name() << ": called as: " << options.string() << std::endl; }

        bool in_operation = false;
        #ifdef WIN32
        if( has_size || operation.size() == 1 ) { _setmode( _fileno( stdout ), _O_BINARY ); }
        #endif
        if( operation.empty() ) { std::cerr << "io-buffer: please specify operation" << std::endl; return 1; }
        else if ( operation.front() == "out" ){ }
        else if ( operation.front() == "in" ){ in_operation = true; }
        else { std::cerr  << "unknown operation found: " << operation.front() << std::endl; return 1; }
        
        if( options.exists("--lines,-n") && options.exists("--size,-s") ) { std::cerr << name() << " option --lines(|-n) is not compatible with --size(|-s)." << std::endl; return 1; }
        
        
        bool output_last = options.exists("--no-last");
        
        comma::uint32 message_size = 0;
        comma::uint64 buffer_size = 0;
        if( has_size ) 
        { 
            if( buffer_size_string ){ buffer_size = get_buffer_size( buffer_size_string.get() ); } else { buffer_size = has_size.get(); }
            
            message_size = has_size.get();
            if( message_size == 0 ) { std::cerr << "io-buffer: message size cannot be zero" << std::endl; return 1; }
            if( message_size > buffer_size ) { std::cerr << "io-buffer: buffer size cannot be smaller than message size." << std::endl; return 1; }
            // make buffer size a multiple of message_size
            buffer_size = buffer_size - ( buffer_size % message_size );
            if( verbose) { std::cerr << "io-buffer: create binary buffer of size " << buffer_size << '(' << comma::uint32(buffer_size / message_size) << " messages)" << std::endl; }
        }
        
        // Check the lockfile can be opened and  truncated
        {
            std::ofstream  lockfile( lockfile_path.c_str(), std::ios::trunc | std::ios::out );
            if( !lockfile.is_open() ) { std::cerr << name() << "failed to open lockfile: " << lockfile << std::endl; return 1; }
            lockfile.close();
        }
        
        using boost::interprocess::file_lock;
        using boost::interprocess::scoped_lock;
        file_lock lock( lockfile_path.c_str() );     // Note lock file must exists or an exception with crytic message is thrown 
        if( has_size ) 
        { 
            binary_buffer = char_buffer_t();
            binary_buffer->reserve( buffer_size );
            while( true )
            {
                char_buffer_t message( message_size );
                std::cin.read( &message[0], message_size );
                if( !std::cin.good() || std::cin.eof() ) { break; }
                binary_buffer->insert( binary_buffer->end(), message.begin(), message.end() );  // can't use emplace_back
                
                if( binary_buffer->size() >= buffer_size )
                {
                    std::cerr << "io-buffer: binary buffer filled at size " << binary_buffer->size() << std::endl;
                    scoped_lock< file_lock > filelock( lock );  // Blocks until it has the lock
                    std::cout.write( &(binary_buffer.get()[0]), binary_buffer->size() );
                    binary_buffer->clear();
                    
                    if( in_operation ) { return 0; }
                }
            }
            
            if( !binary_buffer->empty() )
            {
                std::cerr << "io-buffer: binary buffer at end size " << binary_buffer->size() << std::endl;
                scoped_lock< file_lock > filelock( lock );  // Blocks until it has the lock
                std::cout.write( &(binary_buffer.get()[0]), binary_buffer->size() );
                binary_buffer->clear();
            }
        }
        else
        {
            lines_buffer = std::vector< std::string >();
            lines_buffer->reserve( lines_num );
            std::cerr << "io-buffer: create lines buffer of size " << lines_num << std::endl;
            while( std::cin.good() && !std::cin.eof() )
            {
                std::string line;
                std::getline( std::cin, line );
                if( !line.empty() ) { lines_buffer->push_back( line ); }
                
                if( lines_buffer->size() >= lines_num )
                {
                    std::cerr << "io-buffer: lines buffer filled at size " << lines_buffer->size() << std::endl;
                    scoped_lock< file_lock > filelock( lock );  // Blocks until it has the lock
                    for( std::size_t i=0; i<lines_buffer->size(); ++i ) { std::cout << lines_buffer.get()[i] << std::endl;  }
                    lines_buffer->clear();
                    
                    if( in_operation ) { return 0; }
                }
            }
            
            if( !output_last && !lines_buffer->empty() )
            {
                std::cerr << "io-buffer: lines buffer at end size " << lines_buffer->size() << std::endl;
                scoped_lock< file_lock > filelock( lock );  // Blocks until it has the lock
                for( std::size_t i=0; i<lines_buffer->size(); ++i ) { std::cout << lines_buffer.get()[i] << std::endl;  }
                lines_buffer->clear();
            }
            
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "io-buffer: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "io-buffer: unknown exception" << std::endl; }
    return 1;
}
