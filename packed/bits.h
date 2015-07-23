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


/// @author Matthew Herrmann 2007
/// @author Vsevolod Vlaskine 2010-2011

#ifndef COMMA_PACKED_BITS_HEADER_H_
#define COMMA_PACKED_BITS_HEADER_H_

#include <limits>
#include <string.h>
#include <boost/static_assert.hpp>
#include <comma/packed/field.h>
#include <comma/base/types.h>
#include <boost/type_traits.hpp>

namespace comma { namespace packed {

/// packed bit-field structure
/// @note had to use memcpy(), since reinterpret_cast gives a compilation warning
template < typename B, typename comma::integer< sizeof( B ), false >::type Default = 0 >
struct bits : public packed::field< bits< B, Default >, B, sizeof( typename comma::integer< sizeof( B ), false >::type ) >
{
    enum { size = sizeof( B ) };

    typedef typename comma::integer< sizeof( B ), false >::type integer_type;

    typedef B type;

    typedef packed::field< bits< B, Default >, B, sizeof( integer_type ) > base_type;

    bits() {}
    bits( integer_type v ) { operator=( v ); }
    bits( type t ) : base_type( t ) {}

    static type default_value() { static const integer_type d = Default; type t; ::memcpy( &t, &d, size ); return t; }

    static void pack( char* storage, type value ) { ::memcpy( storage, &value, size ); }

    static type unpack( const char* storage ) { type t; ::memcpy( &t, storage, size ); return t; }

    const bits& operator=( const bits& rhs ) { return base_type::operator=( rhs ); }

    const bits& operator=( type rhs ) { return base_type::operator=( rhs ); }

    const bits& operator=( integer_type rhs ) { type t; ::memcpy( &t, &rhs, size ); return base_type::operator=( t ); }

    type& fields() { return *( reinterpret_cast< type* >( this ) ); }

    const type& fields() const { return *( reinterpret_cast< const type* >( this ) ); }

    integer_type integer_value() const { return *reinterpret_cast< const integer_type* >( base_type::data() ); }
};

template< typename T > inline void reverse_bits( T& v )
{
    BOOST_STATIC_ASSERT( boost::is_unsigned< T >::value );
    unsigned int s = std::numeric_limits< T >::digits - 1;
    T r = v;
    for( v >>= 1; v; v >>= 1 )
    {
        r <<= 1;
        r |= v & 1;
        s--;
    }
    r <<= s;
    v = r;
}

template< typename T > inline T get_reversed_bits( T v ) { reverse_bits( v ); return v; }

/// packed reversed bit-field structure
template < typename B, typename comma::integer< sizeof( B ), false >::type Default = 0  >
struct reversed_bits : public packed::field< reversed_bits< B, Default >, B, sizeof( typename comma::integer< sizeof( B ), false >::type ) >
{
    typedef typename comma::integer< sizeof( B ), false >::type integer_type;

    enum { size = sizeof( B ) };

    typedef B type;

    typedef packed::field< reversed_bits< B, Default >, B, size > base_type;

    reversed_bits() {}
    reversed_bits( integer_type v ) { operator=( v ); }
    reversed_bits( type t ) : base_type( t ) {}

    static type default_value() { static const integer_type d = Default; type t; ::memcpy( &t, &d, size ); return t; }

    static void pack( char* storage, type t ) { integer_type v; ::memcpy( &v, &t, size ); reverse_bits( v ); ::memcpy( storage, &v, size ); }

    static type unpack( const char* storage ) { integer_type v; ::memcpy( &v, storage, size ); reverse_bits( v ); type t; ::memcpy( &t, &v, size ); return t; }

    const reversed_bits& operator=( const reversed_bits& rhs ) { return base_type::operator=( rhs ); }

    const reversed_bits& operator=( type rhs ) { return base_type::operator=( rhs ); }

    const reversed_bits& operator=( integer_type rhs ) { type t; ::memcpy( &t, &rhs, size ); return base_type::operator=( t ); }

};

} } // namespace comma { namespace packed {

#endif // COMMA_PACKED_BITS_HEADER_H_
