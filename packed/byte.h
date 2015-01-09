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


/// @author Vsevolod Vlaskine 2010-2011

#ifndef COMMA_PACKED_BYTE_HEADER_H_
#define COMMA_PACKED_BYTE_HEADER_H_

#if defined(__linux__) || defined(__APPLE__) || defined(__QNXNTO__)
#include <arpa/inet.h>
#elif defined(WIN32)
#if defined(WINCE)
#include <Winsock.h>
#else
#include <Winsock2.h>
#endif
#endif

#include <boost/static_assert.hpp>
#include <comma/packed/field.h>

namespace comma { namespace packed {

/// packed byte
struct byte : public packed::field< byte, unsigned char, sizeof( unsigned char ) >
{
    enum { size = sizeof( unsigned char ) };

    BOOST_STATIC_ASSERT( size == 1 );

    typedef unsigned char type;

    typedef packed::field< byte, unsigned char, sizeof( unsigned char ) > base_type;

    static type default_value() { return 0; }

    static void pack( char* storage, type value ) { *storage = static_cast< char >( value ); }

    static type unpack( const char* storage ) { return static_cast< unsigned char >( *storage ); }

    const byte& operator=( const byte& rhs ) { return base_type::operator=( rhs ); }

    const byte& operator=( type rhs ) { return base_type::operator=( rhs ); }
};

/// packed fixed-value byte
template < unsigned char C >
struct const_byte : public packed::field< const_byte< C >, unsigned char, sizeof( unsigned char ) >
{
    enum { size = sizeof( unsigned char ) };

    BOOST_STATIC_ASSERT( size == 1 );

    typedef unsigned char type;

    typedef packed::field< const_byte, unsigned char, sizeof( unsigned char ) > base_type;

    const_byte() { base_type::operator=( C ); }

    static type default_value() { return C; }

    static void pack( char* storage, type value ) { *storage = static_cast< char >( value ); }

    static type unpack( const char* storage ) { return static_cast< unsigned char >( *storage ); }

    //const const_byte& operator=( const const_byte& rhs ) { return base_type::operator=( rhs ); }

    //const const_byte& operator=( type rhs ) { return base_type::operator=( rhs ); }
};

typedef byte uint8;

} } // namespace comma { namespace packed {

#endif // #ifndef COMMA_PACKED_BYTE_HEADER_H_
