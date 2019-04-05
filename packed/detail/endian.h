// This file is provided in addition to snark and is not an integral
// part of snark library.
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

// snark is a generic and flexible library for robotics research
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

#include <algorithm>
#include <boost/static_assert.hpp>
#include "../../base/exception.h"
#include "../../base/types.h"
#include "../field.h"

namespace comma { namespace packed { namespace detail {

BOOST_STATIC_ASSERT( sizeof( float ) == 4 );
BOOST_STATIC_ASSERT( sizeof( double ) == 8 );

//inline static bool host_is_little_endian_() { comma::uint32 i = 1; return reinterpret_cast< char* >( &i )[0] == 1; }

//static bool host_is_little_endian = detail::host_is_little_endian_(); // super quick and dirty

template < typename T > struct net_traits {};

template <> struct net_traits< comma::uint16 >
{
    static comma::uint16 hton( comma::uint16 v ) { return htons( v ); }
    static comma::uint16 ntoh( comma::uint16 v ) { return ntohs( v ); }
};

template <> struct net_traits< comma::int16 >
{
    static comma::int16 hton( comma::int16 v ) { return htons( v ); }
    static comma::int16 ntoh( comma::int16 v ) { return ntohs( v ); }
};

template <> struct net_traits< comma::uint32 >
{
    static comma::uint32 hton( comma::uint32 v ) { return htonl( v ); }
    static comma::uint32 ntoh( comma::uint32 v ) { return ntohl( v ); }
};

template <> struct net_traits< comma::int32 >
{
    static comma::int32 hton( comma::int32 v ) { return htonl( v ); }
    static comma::int32 ntoh( comma::int32 v ) { return ntohl( v ); }
};

template < typename type, typename uint_of_same_size >
inline type pack_float( type value )
{
    char storage[sizeof(type)];
    uint_of_same_size* p = reinterpret_cast< uint_of_same_size* >( &value );
    for( unsigned int i = 0; i < sizeof( type ); ++i, *p >>= 8 ) { storage[sizeof(type)-i-1] = *p & 0xff; } 
    const type* result = reinterpret_cast< const type* >( &storage );
    return *result;
}

template< typename type, typename uint_of_same_size >
inline type unpack_float( type value ) 
{
    const char* storage = reinterpret_cast< const char* >( &value ); 
    uint_of_same_size v = 0;
    unsigned int shift = 0;
    for( unsigned int i = 0; i < sizeof( type ); ++i, shift += 8 ) { v += static_cast< uint_of_same_size >( ( unsigned char )( storage[sizeof(type)-i-1] ) ) << shift; }
    const type* result = reinterpret_cast< const type* >( &v );
    return *result;
}

template <> struct net_traits< float >
{
    typedef comma::uint32 uint_of_same_size;
    static float hton( float value ) { return pack_float< float, uint_of_same_size >( value ); }
    static float ntoh( float value ) { return unpack_float< float, uint_of_same_size >( value ); }
};

template <> struct net_traits< double >
{
    typedef comma::uint64 uint_of_same_size;
    static double hton( double value ) { return pack_float< double, uint_of_same_size >( value ); }
    static double ntoh( double value ) { return unpack_float< double, uint_of_same_size >( value ); }
};
    
template < bool Little, unsigned int Size, bool Signed, bool Floating = false > struct endian_traits { typedef typename comma::integer< Size, Signed >::type type; typedef typename comma::integer< Size, false >::type uint_of_same_size; };
template < bool Little > struct endian_traits< Little, 3, true > { typedef comma::int32 type; typedef comma::uint32 uint_of_same_size; };
template < bool Little > struct endian_traits< Little, 3, false > { typedef comma::uint32 type; typedef comma::uint32 uint_of_same_size; };
template < bool Little > struct endian_traits< Little, 4, true, true > { typedef float type; typedef comma::uint32 uint_of_same_size; };
template < bool Little > struct endian_traits< Little, 8, true, true > { typedef double type; typedef comma::uint64 uint_of_same_size; };

enum { little = 0, big = 1 };

template < bool Little, unsigned int Size, bool Signed, bool Floating = false >
struct endian : public packed::field< endian< Little, Size, Signed, Floating >, typename endian_traits< Little, Size, Signed, Floating >::type, Size >
{
    static const unsigned int size = Size;

    typedef typename endian_traits< Little, Size, Signed, Floating >::type type;
    
    BOOST_STATIC_ASSERT( size <= sizeof( type ) );

    typedef packed::field< endian< Little, Size, Signed, Floating >, typename endian_traits< Little, Size, Signed, Floating >::type, Size > base_type;

    static type default_value() { return 0; }

    typedef typename endian_traits< Little, size, Signed, Floating >::uint_of_same_size uint_of_same_size;
    
    static void pack( char* storage, type value )
    {
        uint_of_same_size* p = reinterpret_cast< uint_of_same_size* >( &value );
        for( unsigned int i = 0; i < size; ++i, *p >>= 8 ) { storage[i] = *p & 0xff; }
    }

    static type unpack( const char* storage ) // for floats it is a real hack, since there is no standard
    {
        uint_of_same_size v = 0;
        unsigned int shift = 0;
        unsigned int i = 0;
        for( ; i < size; ++i, shift += 8 )
        {
            v += static_cast< uint_of_same_size >( ( unsigned char )( storage[i] ) ) << shift;
        }
        if( !Floating && Signed && ( storage[ size - 1 ] & 0x80 ) )
        {            
            for( ; i < sizeof( type ); ++i, shift += 8 ) { v +=  static_cast< uint_of_same_size >( 0xff ) << shift; } 
        }
        const type* result = reinterpret_cast< const type* >( &v );
        return *result;
    }

    const endian& operator=( const endian& rhs ) { return base_type::operator=( rhs ); }

    const endian& operator=( const type& rhs ) { return base_type::operator=( rhs ); }
};

} } } // namespace comma { namespace packed { namespace detail {
