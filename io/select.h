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

#ifndef COMMA_IO_SELECT_HEADER
#define COMMA_IO_SELECT_HEADER

#if defined(WINCE)
#include <Winsock.h>
#elif defined(WIN32)
#include <Winsock2.h>
#else
#include "sys/select.h"
#endif

#include <cassert>
#include <set>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/unordered_set.hpp>
#include "../base/exception.h"
#include "file_descriptor.h"

namespace comma { namespace io {

/// select() wrapper; currently implemented in a quick way: it
/// works as expected in POSIX systems, but in Windows it works
/// only on sockets, not on files (because select() in Winsock
/// works only for sockets.
///
/// @todo implement POSIX select() behaviour for Windows
///
/// @todo clean up: add WSAStartup for Windows (move WSAStartup to a proper
///       places in utilities, as currently it is sitting in Bytestreams;
///       now, it still works, because one can use select() in Windows
///       only on sockets, therefore, WSAStartup will be called from our
///       socket library; but the latter solution indirectly relies on
///       something extrinsic to select and therefore is not good.
class select
{
    public:
        /// blocking wait, if OK, returns what select() returned, otherwise throws
        std::size_t wait();

        /// wait with timeout, if OK, returns what select() returned, otherwise throws
        std::size_t wait( unsigned int timeout_seconds, unsigned int timeout_nanoseconds = 0 );

        /// wait with timeout, if OK, returns what select() returned, otherwise throws
        std::size_t wait( boost::posix_time::time_duration timeout );

        /// same as wait( 0 )
        std::size_t check();

        /// descriptor pool for select to monitor
        class descriptors
        {
            public:
                /// default constructor
                descriptors();
                
                /// add file descriptor
                void add( file_descriptor fd );
                template < typename T > void add( const T& t ) { add( t.fd() ); }
                
                /// remove file descriptor
                void remove( file_descriptor fd );
                template < typename T > void remove( const T& t ) { remove( t.fd() ); }
                
                /// return true, if file descriptor found in descriptor list and ready
                bool ready( file_descriptor fd ) const;
                template < typename T > bool ready( const T& t ) const { return ready( t.fd() ); }
                
                /// return set of descriptors
                const std::set< file_descriptor >& operator()() const { return descriptors_; } //const boost::unordered_set< file_descriptor >& operator()() const { return descriptors_; }

            private:
                friend class select;
                fd_set* reset_fds_();
                std::set< file_descriptor > descriptors_; //boost::unordered_set< file_descriptor > descriptors_;
                fd_set fd_set_;
        };

        /// return read descriptors
        descriptors& read() { return read_descriptors_; }
        const descriptors& read() const { return read_descriptors_; }

        /// return write descriptors
        descriptors& write() { return write_descriptors_; }
        const descriptors& write() const { return write_descriptors_; }

        /// return except descriptors
        descriptors& except() { return except_descriptors_; }
        const descriptors& except() const { return except_descriptors_; }

    private:
        descriptors read_descriptors_;
        descriptors write_descriptors_;
        descriptors except_descriptors_;
};

inline void select::descriptors::add( file_descriptor fd )
{
    if( fd == invalid_file_descriptor ) { COMMA_THROW( comma::exception, "invalid file descriptor" ); }
    descriptors_.insert( fd );
}

inline void select::descriptors::remove( file_descriptor fd )
{
    if( fd == invalid_file_descriptor ) { COMMA_THROW( comma::exception, "invalid file descriptor" ); }
    descriptors_.erase( fd );
}

inline bool select::descriptors::ready( file_descriptor fd ) const
{
    if( fd == invalid_file_descriptor ) { COMMA_THROW( comma::exception, "invalid file descriptor" ); }
    return descriptors_.find( fd ) != descriptors_.end() && FD_ISSET( fd, const_cast< fd_set* >( &fd_set_ ) ) != 0;
}


} } // namespace comma { namespace io {

#endif // COMMA_IO_SELECT_HEADER
