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

#ifndef COMMA_IO_STREAM_H_
#define COMMA_IO_STREAM_H_

#include <iostream>
#include <string>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <comma/io/file_descriptor.h>

namespace comma { namespace io {

struct mode
{
    enum value { ascii = 0, binary = std::ios::binary };
    enum blocking_value { non_blocking = false, blocking = true };
};
    
/// interface class
/// constructs standard stream from name and owns it:
///     filename: file stream
///     -: std::cin or std::cout
///     tcp:address:port: tcp client socket stream
///     @todo udp:address:port: udp socket stream
///     @todo linux socket name: linux socket client stream
///     @todo serial device name: serial stream
/// see unit test for usage
template < typename S >
class stream : boost::noncopyable
{
    public:
        /// close stream, if closable
        void close();

        /// return pointer to stream
        S* operator()();

        /// return reference to stream
        S& operator*();

        /// return pointer to stream
        S* operator->();

        /// return file descriptor (to use in select)
        comma::io::file_descriptor fd() const;

        /// return stream name
        const std::string& name() const;
        
    protected:
        stream( const std::string& name, mode::value mode, mode::blocking_value blocking );
        template < typename T >
        stream( T* s, io::file_descriptor fd, mode::value mode, mode::blocking_value blocking, boost::function< void() > close )
            : mode_( mode )
            , stream_( s )
            , close_( close )
            , fd_( fd )
            , close_d( false )
            , blocking_( blocking )
        {
        }
        ~stream();
        std::string name_;
        mode::value mode_;
        S* stream_;
        boost::function< void() > close_;
        comma::io::file_descriptor fd_;
        bool close_d;
        bool blocking_;
};
    
/// input stream owner
struct istream : public stream< std::istream >
{
    istream( const std::string& name, mode::value mode = mode::ascii, mode::blocking_value blocking = mode::blocking );
    istream( std::istream* s, io::file_descriptor fd, mode::value mode, boost::function< void() > close );
    istream( std::istream* s, io::file_descriptor fd, mode::value mode, mode::blocking_value blocking, boost::function< void() > close );
};

/// output stream owner
struct ostream : public stream< std::ostream >
{
    ostream( const std::string& name, mode::value mode = mode::ascii, mode::blocking_value blocking = mode::blocking );
    ostream( std::ostream* s, io::file_descriptor fd, mode::value mode, boost::function< void() > close );
    ostream( std::ostream* s, io::file_descriptor fd, mode::value mode, mode::blocking_value blocking, boost::function< void() > close );
};

/// input/output stream owner
struct iostream : public stream< std::iostream >
{
    iostream( const std::string& name, mode::value mode = mode::ascii, mode::blocking_value blocking = mode::blocking );
};

} } // namespace comma { namespace io {
    
#endif
