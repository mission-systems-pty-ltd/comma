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
#include "istream.h"

namespace comma {
namespace io {
namespace zeromq {

istream::istream( const std::string& endpoint ):
    m_context( new zmq::context_t( 1 ) ),
    m_socket( new zmq::socket_t( *m_context, ZMQ_SUB ) ),
    m_index( m_buffer.size() )
{
    m_socket->connect( endpoint.c_str() );
    m_socket->setsockopt( ZMQ_SUBSCRIBE, "", 0 );
}


std::streamsize istream::read( char* s, std::streamsize n )
{
    zmq::message_t message;
    if( m_socket->recv( &message ) )
    {
        if( m_index != m_buffer.size() )
        {
            unsigned int size = std::min( m_buffer.size() - m_index, static_cast< std::size_t >( n ) );
            ::memcpy( s, &m_buffer[ m_index ], size );
            m_index += size;
            return size;
        }
        else if ( message.size() < static_cast< unsigned int >( n ) )
        {            
            ::memcpy( s, ( const char* )message.data(), message.size() );
            return message.size();
        }
        else
        {
            ::memcpy( s, ( const char* )message.data(), n );
            m_buffer.resize( message.size() - n ); // TODO do not resize if smaller ( for performance )
            ::memcpy( &m_buffer[0], ( const char* )message.data(), m_buffer.size() );
            m_index = 0;
            return n;
        }
    }
    else
    {
        return -1; // eof
    }
}

} } }

