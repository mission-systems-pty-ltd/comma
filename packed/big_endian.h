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


/// @author Matthew Herrmann 2007
/// @author Vsevolod Vlaskine 2010-2011

#ifndef COMMA_PACKED_BIG_ENDIAN_H_
#define COMMA_PACKED_BIG_ENDIAN_H_

#include <algorithm>
#include <boost/static_assert.hpp>
#include <comma/base/exception.h>
#include <comma/base/types.h>
#include <comma/packed/field.h>

namespace comma { namespace packed {

namespace detail {

template < typename T > struct net_traits {};

template <> struct net_traits< comma::uint16 >
{
    BOOST_STATIC_ASSERT( sizeof( comma::uint16 ) == 2 );
    static comma::uint16 hton( comma::uint16 v ) { return htons( v ); }
    static comma::uint16 ntoh( comma::uint16 v ) { return ntohs( v ); }
};

template <> struct net_traits< comma::int16 >
{
    BOOST_STATIC_ASSERT( sizeof( comma::int16 ) == 2 );
    static comma::int16 hton( comma::int16 v ) { return htons( v ); }
    static comma::int16 ntoh( comma::int16 v ) { return ntohs( v ); }
};

template <> struct net_traits< comma::uint32 >
{
    BOOST_STATIC_ASSERT( sizeof( comma::uint32 ) == 4 );
    static comma::uint32 hton( comma::uint32 v ) { return htonl( v ); }
    static comma::uint32 ntoh( comma::uint32 v ) { return ntohl( v ); }
};

template <> struct net_traits< comma::int32 >
{
    BOOST_STATIC_ASSERT( sizeof( comma::int32 ) == 4 );
    static comma::int32 hton( comma::int32 v ) { return htonl( v ); }
    static comma::int32 ntoh( comma::int32 v ) { return ntohl( v ); }
};

template< typename T >
inline T reverse( T v )
{ 
    static bool is_big_endian = comma::uint16( 0xab ) == htons( comma::uint16( 0xab ) );
    if( is_big_endian ) { COMMA_THROW( comma::exception, "don't know how to deal with floating point in big endian architectures" ); } //if( is_big_endian ) { return v; }
    char* p = reinterpret_cast< char* >( &v ); std::reverse( p, p + sizeof( T ) );
    return v;
}

template <> struct net_traits< float >
{
    BOOST_STATIC_ASSERT( sizeof( float ) == 4 );
    static float hton( float v ) { return reverse< float >( v ); }
    static float ntoh( float v ) { return reverse< float >( v ); }
};

template <> struct net_traits< double >
{
    BOOST_STATIC_ASSERT( sizeof( double ) == 8 );
    static double hton( double v ) { return reverse< double >( v ); }
    static double ntoh( double v ) { return reverse< double >( v ); }
};

template < typename T >
class big_endian : public packed::field< big_endian< T >, T, sizeof( T ) >
{
    public:
        enum { size = sizeof( T ) };

        typedef T type;

        typedef packed::field< big_endian< T >, T, size > base_type;

        static type default_value() { return 0; }

        static void pack( char* storage, type value )
        {
            type v( net_traits< type >::hton( value ) );
            ::memcpy( storage, ( void* )&v, size );
        }

        static type unpack( const char* storage )
        {
            type value;
            ::memcpy( ( void* )&value, storage, size );
            return net_traits< type >::ntoh( value );
        }

        const big_endian& operator=( const big_endian& rhs ) { return base_type::operator=( rhs ); }

        const big_endian& operator=( type rhs ) { return base_type::operator=( rhs ); }
};

} // namespace detail {

/// big endian 16-bit integers
typedef detail::big_endian< comma::uint16 > big_endian_uint16;
typedef detail::big_endian< comma::int16 > big_endian_int16;
/// aliases for big endian 16-bit integers
typedef big_endian_uint16 net_uint16;
typedef big_endian_int16 net_int16;
/// big endian 32-bit integers
typedef detail::big_endian< comma::uint32 > big_endian_uint32;
typedef detail::big_endian< comma::int32 > big_endian_int32;
/// aliases for big endian 32-bit integers
typedef big_endian_uint32 net_uint32;
typedef big_endian_int32 net_int32;
/// big endian float and double
typedef detail::big_endian< float > big_endian_float32;
typedef detail::big_endian< double > big_endian_float64;
typedef big_endian_float64 big_endian_double;
/// aliases for big endian float and double
typedef big_endian_float32 net_float32;
typedef big_endian_float64 net_float64;
typedef net_float64 net_double;


} } // namespace comma { namespace packed {

#endif // #ifndef COMMA_PACKED_BIG_ENDIAN_H_
