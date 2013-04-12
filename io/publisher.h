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


/// @author vsevolod vlaskine

#ifndef COMMA_IO_PUBLISHER_H_
#define COMMA_IO_PUBLISHER_H_

#include <stdlib.h>
#include <string>
#include <boost/noncopyable.hpp>
#include <comma/io/stream.h>
#include <comma/io/impl/publisher.h>

namespace comma { namespace io {

/// a simple publisher that opens and writes to using services (e.g. tcp, udp, etc)
class publisher : public boost::noncopyable
{
    public:
        /// constructor
        /// @param name ::= tcp:<port> | udp:<port> | <filename>
        ///     if tcp:<port>, create tcp server
        ///     @todo if udp:<port>, broadcast on udp
        ///     if <filename> is a regular file, just write to it
        ///     if <filename> is named pipe, keep reopening it, if closed
        ///     @todo if <filename> is Linux domain socket, create Linux domain socket server
        /// @param mode ascii or binary, a hint for Windows
        /// @param blocking if true, blocking write to a client, otherwise discard, if client not ready
        publisher( const std::string& name, io::mode::value mode, bool blocking = false, bool flush = true );

        /// destructor
        ~publisher();

        /// publish to all existing connections (blocking), return number of client with successful write
        std::size_t write( const char* buf, std::size_t size );

        /// publish to all existing connections (blocking)
        /// @note data integrity is the user's responsibility
        ///       i.e. if someone writes:
        ///           publisher p( "tcp:localhost:1234" );
        ///           p << 1 << "," << 2 << std::endl;
        ///       and a client connects after "1" already
        ///       has been output, this client will receive
        ///       ",2", which most likely was not intended
        template < typename T >
        publisher& operator<<( const T& rhs ) { pimpl_->operator<<( rhs ); return *this; }

        /// close
        void close();

        /// return current number of connected clients
        std::size_t size() const;

        /// accept waiting clients, non-blocking
        void accept();
        
    private:
        impl::publisher* pimpl_;
};

} } // namespace comma { namespace io {

#endif // #ifndef COMMA_IO_PUBLISHER_H_
