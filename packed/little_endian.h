// This file is part of comma library
//
// Copyright (C) 2011 The University of Sydney
//
// comma is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// comma is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License 
// for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with comma. If not, see <http://www.gnu.org/licenses/>.

/// @author Matthew Herrmann 2007
/// @author Vsevolod Vlaskine 2010-2011

#ifndef COMMA_PACKED_LITTLEENDIAN_H_
#define COMMA_PACKED_LITTLEENDIAN_H_

#include <comma/base/types.h>
#include <comma/packed/field.h>

namespace comma { namespace packed {

namespace detail {

template < unsigned int Size, bool Signed > struct little_endian_traits { typedef void type; };
template <> struct little_endian_traits< 2, true > { typedef comma::int16 type; };
template <> struct little_endian_traits< 2, false > { typedef comma::uint16 type; };
template <> struct little_endian_traits< 3, true > { typedef comma::int32 type; };
template <> struct little_endian_traits< 3, false > { typedef comma::uint32 type; };
template <> struct little_endian_traits< 4, true > { typedef comma::int32 type; };
template <> struct little_endian_traits< 4, false > { typedef comma::uint32 type; };
template <> struct little_endian_traits< 8, true > { typedef comma::int64 type; };
template <> struct little_endian_traits< 8, false > { typedef comma::uint64 type; };
    
template < unsigned int Size, bool Signed, bool Floating = false >
struct little_endian_int : public packed::field< little_endian_int< Size, Signed >, typename little_endian_traits< Size, Signed >::type, Size >
{
    static const unsigned int size = Size;

    typedef typename little_endian_traits< Size, Signed >::type type;
    
    BOOST_STATIC_ASSERT( size <= sizeof( type ) );

    typedef packed::field< little_endian_int< Size, Signed >, typename little_endian_traits< Size, Signed >::type, Size > base_type;

    static type default_value() { return 0; }

    static void pack( char* storage, type value )
    {
        for( unsigned int i = 0; i < size; ++i, value >>= 8 ) { storage[i] = value & 0xff; }
    }

    static type unpack( const char* storage )
    {
        type v = 0;
        int shift = 0;
        unsigned int i = 0;
        for( ; i < size; ++i, shift += 8 )
        {
            v += ( unsigned char )( storage[i] ) << shift;
        }
        if( storage[ size - 1 ] & 0x80 )
        {
            for( ; i < sizeof( type ); ++i, shift += 8 ) { v += 0xff << shift; }
        }
        return v;
    }

    const little_endian_int& operator=( const little_endian_int& rhs ) { return base_type::operator=( rhs ); }

    const little_endian_int& operator=( type rhs ) { return base_type::operator=( rhs ); }
};

} // namespace detail {

/// packed little endian 16-bit integers
typedef detail::little_endian_int< 2, true > little_endian_int16;
typedef detail::little_endian_int< 2, false > little_endian_uint16;
typedef little_endian_int16 int16;
typedef little_endian_uint16 uint16;
/// packed little endian 24-bit integers (strangely, there are protocols using it)
typedef detail::little_endian_int< 3, true > little_endian_int24;
typedef detail::little_endian_int< 3, false > little_endian_uint24;
typedef little_endian_int24 int24;
typedef little_endian_uint24 uint24;
/// packed little endian 32-bit integers
typedef detail::little_endian_int< 4, true > little_endian_int32;
typedef detail::little_endian_int< 4, false > little_endian_uint32;
typedef little_endian_int32 int32;
typedef little_endian_uint32 uint32;

} } // namespace comma { namespace packed {

#endif // #ifndef COMMA_PACKED_LITTLEENDIAN_H_
