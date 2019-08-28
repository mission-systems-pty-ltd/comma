// This file is provided in addition to comma and is not an integral
// part of comma library.
// Copyright (c) 2018 Vsevolod Vlaskine
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
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

// comma is a generic and flexible library
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

#pragma once

#include <endian.h>
#include <algorithm>
#include <boost/static_assert.hpp>
#include "../../base/exception.h"
#include "../../base/types.h"
#include "../field.h"

namespace comma { namespace packed { namespace detail {

BOOST_STATIC_ASSERT( sizeof( float ) == 4 );
BOOST_STATIC_ASSERT( sizeof( double ) == 8 );

enum endiannes { little = 0, big = 1 };
    
template < endiannes Endianness, unsigned int Size, bool Signed, bool Floating = false > struct endian_traits { typedef typename comma::integer< Size, Signed >::type type; typedef typename comma::integer< Size, false >::type uint_of_same_size; };
template < endiannes Endianness > struct endian_traits< Endianness, 3, true > { typedef comma::int32 type; typedef comma::uint32 uint_of_same_size; };
template < endiannes Endianness > struct endian_traits< Endianness, 3, false > { typedef comma::uint32 type; typedef comma::uint32 uint_of_same_size; };
template < endiannes Endianness > struct endian_traits< Endianness, 4, true, true > { typedef float type; typedef comma::uint32 uint_of_same_size; };
template < endiannes Endianness > struct endian_traits< Endianness, 8, true, true > { typedef double type; typedef comma::uint64 uint_of_same_size; };

template < typename T > struct net_traits;

template <> struct net_traits< comma::uint16 >
{
    static comma::uint16 htobe( comma::uint16 v ) { return htobe16( v ); }
    static comma::uint16 betoh( comma::uint16 v ) { return be16toh( v ); }
    static comma::uint16 htole( comma::uint16 v ) { return htole16( v ); }
    static comma::uint16 letoh( comma::uint16 v ) { return le16toh( v ); }
};

template <> struct net_traits< comma::uint32 >
{
    static comma::uint32 htobe( comma::uint32 v ) { return htobe32( v ); }
    static comma::uint32 betoh( comma::uint32 v ) { return be32toh( v ); }
    static comma::uint32 htole( comma::uint32 v ) { return htole32( v ); }
    static comma::uint32 letoh( comma::uint32 v ) { return le32toh( v ); }
};

template <> struct net_traits< comma::uint64 >
{
    static comma::uint64 htobe( comma::uint64 v ) { return htobe64( v ); }
    static comma::uint64 betoh( comma::uint64 v ) { return be64toh( v ); }
    static comma::uint64 htole( comma::uint64 v ) { return htole64( v ); }
    static comma::uint64 letoh( comma::uint64 v ) { return le64toh( v ); }
};

template < endiannes Endianness > struct convert;

template <> struct convert< packed::detail::little >
{
    template < typename T > static T from_host( T t ) { return net_traits< T >::htole( t ); }
    template < typename T > static T to_host( T t ) { return net_traits< T >::letoh( t ); }
};

template <> struct convert< packed::detail::big >
{
    template < typename T > static T from_host( T t ) { return net_traits< T >::htobe( t ); }
    template < typename T > static T to_host( T t ) { return net_traits< T >::betoh( t ); }
};

template < unsigned int Size > struct ff { enum { value = ff< Size - 1 >::value << 8 + 0xff }; };
template <> struct ff< 1 > { enum { value = 0xff }; };

template < endiannes Endianness, unsigned int Size, bool Signed, bool Floating = false >
struct endian : public packed::field< endian< Endianness, Size, Signed, Floating >, typename endian_traits< Endianness, Size, Signed, Floating >::type, Size >
{
    static const unsigned int size = Size;

    typedef typename endian_traits< Endianness, Size, Signed, Floating >::type type;
    
    BOOST_STATIC_ASSERT( size <= sizeof( type ) );
    
    BOOST_STATIC_ASSERT( Signed || !Floating ); // unsigned floats don't make sense

    typedef packed::field< endian< Endianness, Size, Signed, Floating >, typename endian_traits< Endianness, Size, Signed, Floating >::type, Size > base_type;

    static type default_value() { return 0; }

    typedef typename endian_traits< Endianness, size, Signed, Floating >::uint_of_same_size uint_of_same_size;
    
    static void pack( char* storage, type value )
    {
        uint_of_same_size* p = reinterpret_cast< uint_of_same_size* >( &value );
        *p = convert< Endianness >::from_host( *p );
        ::memcpy( storage, ( void* )p + ( Endianness == little ? 0 : sizeof( uint_of_same_size ) - size ), size );
    }
    
    static type unpack( const char* storage ) // for floats it is a real hack, since there is no standard
    {
        uint_of_same_size i = ( !Floating && Signed && ( storage[ Endianness == little ? size - 1 : 0 ] & 0x80 ) ) ? -1 : 0;
        ::memcpy( ( void* )&i + ( Endianness == little ? 0 : sizeof( uint_of_same_size ) - size ), storage, size );
        i = convert< Endianness >::to_host( i );
        return *( reinterpret_cast< type* >( &i ) );
    }

    const endian& operator=( const endian& rhs ) { return base_type::operator=( rhs ); }

    const endian& operator=( const type& rhs ) { return base_type::operator=( rhs ); }
};

} } } // namespace comma { namespace packed { namespace detail {
