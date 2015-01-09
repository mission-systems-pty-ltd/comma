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
//    This product includes software developed by the University of Sydney.
// 4. Neither the name of the University of Sydney nor the
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
#include <cerrno>
#endif

#include <string>
#include <comma/base/exception.h>
#include <comma/base/last_error.h>
#include <comma/io/select.h>

namespace comma { namespace io {
    
static std::size_t select_impl_( int nfds, fd_set* fdr, fd_set* fdw, fd_set* fde, struct timeval* t )
{
    if( fdr == NULL && fdw == NULL && fde == NULL ) { return 0; } // good semantics?
    int r = ::select( nfds, fdr, fdw, fde, t );
    if( r >= 0 ) { return r; }
#ifndef WIN32
    int error = last_error::value();
    if( error != EINTR ) // do no throw if select is interrupted by signal
#endif
    {
        last_error::to_exception( "select() failed" );
    }
    return 0;
}

static int nfds( file_descriptor a, file_descriptor b, file_descriptor c ) // quick and dirty
{
    if( a < b ) { a = b; }
    if( a < c ) { a = c; }
    return ++a;
}

std::size_t select::wait()
{
    return select_impl_(   nfds( read_descriptors_.descriptors_.empty() ? 0 : *read_descriptors_.descriptors_.rbegin()
                             , write_descriptors_.descriptors_.empty() ? 0 : *write_descriptors_.descriptors_.rbegin()
                             , except_descriptors_.descriptors_.empty() ? 0 : *except_descriptors_.descriptors_.rbegin() )
                       , read_descriptors_.reset_fds_()
                       , write_descriptors_.reset_fds_()
                       , except_descriptors_.reset_fds_()
                       , NULL );
}

std::size_t select::wait( unsigned int timeout_seconds, unsigned int timeout_nanoseconds )
{
    struct timeval t;
    t.tv_sec = static_cast< int >( timeout_seconds );
    t.tv_usec = static_cast< int >( timeout_nanoseconds / 1000 );
    return select_impl_(   nfds( read_descriptors_.descriptors_.empty() ? 0 : *read_descriptors_.descriptors_.rbegin()
                             , write_descriptors_.descriptors_.empty() ? 0 : *write_descriptors_.descriptors_.rbegin()
                             , except_descriptors_.descriptors_.empty() ? 0 : *except_descriptors_.descriptors_.rbegin() )
                       , read_descriptors_.reset_fds_()
                       , write_descriptors_.reset_fds_()
                       , except_descriptors_.reset_fds_()
                       , &t );
}

std::size_t select::wait( boost::posix_time::time_duration timeout )
{
    unsigned int sec = timeout.total_seconds();
	unsigned int nanosec = ( static_cast< unsigned int >( timeout.total_microseconds() ) - sec * 1000000 ) * 1000;
//     std::cerr << "select wait " << sec << " , " << nanosec << std::endl;
    return wait( sec, nanosec );
}

std::size_t select::check() { return wait( 0 ); }

select::descriptors::descriptors()
{
    reset_fds_();
}

fd_set* select::descriptors::reset_fds_()
{
    FD_ZERO( &fd_set_ );
    if( descriptors_.empty() ) { return NULL; }
    for( std::set< file_descriptor >::const_iterator it = descriptors_.begin(); it != descriptors_.end(); ++it )
    {
        #ifdef WIN32
        #pragma warning( disable : 4127 )
        #endif
        FD_SET( *it, &fd_set_ );
        #ifdef WIN32
        #pragma warning( default : 4127 )
        #endif
    }
    return &fd_set_;
}

} } // namespace comma { namespace io {
