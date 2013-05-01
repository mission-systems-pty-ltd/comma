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

#include <comma/io/zeromq/istream.h>
#include <comma/io/zeromq/ostream.h>
#include <iostream>

namespace comma { namespace io { namespace zeromq {

template< typename S > struct traits {};
template<> struct traits< std::istream > { typedef comma::io::zeromq::istream stream; };
template<> struct traits< std::ostream > { typedef comma::io::zeromq::ostream stream; };

template< typename S >
inline static S* make_stream( const std::string& endpoint, comma::io::file_descriptor& fd )
{
    typedef typename traits< S >::stream stream_t;
    boost::iostreams::stream< stream_t >* stream = new boost::iostreams::stream< stream_t >( endpoint.c_str() );
    bool have_fd = false;
    while( !have_fd ) // TODO ugly, have timeout instead.
    {
        try
        {
            std::size_t size = sizeof( comma::io::file_descriptor );
            ( *stream )->socket().getsockopt( ZMQ_FD, &fd, &size );
            have_fd = true;
            assert( size == sizeof( fd ) );
        }
        catch( std::exception& e )
        {
            ::usleep( 20 );
        }
    }
    return stream;
}

} } } // namespace comma { namespace io { namespace zeromq {
