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

#ifndef COMMA_PACKED_LITTLEENDIAN_H_
#define COMMA_PACKED_LITTLEENDIAN_H_

#include <boost/static_assert.hpp>
#include <comma/base/exception.h>
#include <comma/base/types.h>
#include <comma/packed/field.h>

namespace comma { namespace packed {

namespace detail {

BOOST_STATIC_ASSERT( sizeof( float ) == 4 );
BOOST_STATIC_ASSERT( sizeof( double ) == 8 );
    
template < unsigned int Size, bool Signed, bool Floating = false > struct little_endian_traits { typedef typename comma::integer< Size, Signed >::type type; typedef typename comma::integer< Size, false >::type uint_of_same_size; };
template <> struct little_endian_traits< 3, true > { typedef comma::int32 type; typedef comma::uint32 uint_of_same_size; };
template <> struct little_endian_traits< 3, false > { typedef comma::uint32 type; typedef comma::uint32 uint_of_same_size; };
template <> struct little_endian_traits< 4, true, true > { typedef float type; typedef comma::uint32 uint_of_same_size; };
template <> struct little_endian_traits< 8, true, true > { typedef double type; typedef comma::uint64 uint_of_same_size; };

template < unsigned int Size, bool Signed, bool Floating = false >
struct little_endian : public packed::field< little_endian< Size, Signed, Floating >, typename little_endian_traits< Size, Signed, Floating >::type, Size >
{
    static const unsigned int size = Size;

    typedef typename little_endian_traits< Size, Signed, Floating >::type type;
    
    BOOST_STATIC_ASSERT( size <= sizeof( type ) );

    typedef packed::field< little_endian< Size, Signed, Floating >, typename little_endian_traits< Size, Signed, Floating >::type, Size > base_type;

    static type default_value() { return 0; }

    typedef typename little_endian_traits< size, Signed, Floating >::uint_of_same_size uint_of_same_size;
    
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

    const little_endian& operator=( const little_endian& rhs ) { return base_type::operator=( rhs ); }

    const little_endian& operator=( const type& rhs ) { return base_type::operator=( rhs ); }
};

} // namespace detail {

/// packed little endian 16-bit integers
typedef detail::little_endian< 2, true > little_endian16;
typedef detail::little_endian< 2, false > little_endian_uint16;
typedef little_endian16 int16;
typedef little_endian_uint16 uint16;
/// packed little endian 24-bit integers (strangely, there are protocols using it)
typedef detail::little_endian< 3, true > little_endian24;
typedef detail::little_endian< 3, false > little_endian_uint24;
typedef little_endian24 int24;
typedef little_endian_uint24 uint24;
/// packed little endian 32-bit integers
typedef detail::little_endian< 4, true > little_endian32;
typedef detail::little_endian< 4, false > little_endian_uint32;
typedef little_endian32 int32;
typedef little_endian_uint32 uint32;
/// packed little endian 32-bit integers
typedef detail::little_endian< 8, true > little_endian64;
typedef detail::little_endian< 8, false > little_endian_uint64;
typedef little_endian64 int64;
typedef little_endian_uint64 uint64;
/// packed floating point number (does it even make sense?)
typedef detail::little_endian< 4, true, true > little_endian_float32;
typedef detail::little_endian< 8, true, true > little_endian_float64;
typedef little_endian_float32 float32;
typedef little_endian_float64 float64;

} } // namespace comma { namespace packed {

#endif // #ifndef COMMA_PACKED_LITTLEENDIAN_H_
