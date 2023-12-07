// Copyright (c) 2011 The University of Sydney

/// @author Matthew Herrmann 2007
/// @author Vsevolod Vlaskine 2010-2011

#pragma once

#include <limits>
#include <string.h>
#include <memory>
#include <type_traits>
#include <boost/type_traits.hpp>
#include "../packed/field.h"
#include "../base/types.h"

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

    static type default_value() { static const integer_type d = Default; type t; std::memcpy( ( char* )( &t ), ( const char* )( &d ), size ); return t; }

    static void pack( char* storage, type value ) { std::memcpy( storage, ( const char* )( &value ), size ); }

    static type unpack( const char* storage ) { type t; std::memcpy( ( char* )( &t ), storage, size ); return t; }

    const bits& operator=( const bits& rhs ) { return base_type::operator=( rhs ); }

    const bits& operator=( type rhs ) { return base_type::operator=( rhs ); }

    const bits& operator=( integer_type rhs ) { type t; std::memcpy( ( char* )( &t ), &rhs, size ); return base_type::operator=( t ); }

    type& fields() { return *( reinterpret_cast< type* >( this ) ); }

    const type& fields() const { return *( reinterpret_cast< const type* >( this ) ); }

    integer_type integer_value() const { return *reinterpret_cast< const integer_type* >( base_type::data() ); }
};

template< typename T > inline void reverse_bits( T& v )
{
    static_assert( boost::is_unsigned< T >::value, "expected unsigned value" );
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

    static type default_value() { static const integer_type d = Default; type t; std::memcpy( ( char* )( &t ), ( const char* )( &d ), size ); return t; }

    static void pack( char* storage, type t ) { integer_type v; std::memcpy( &v, &t, size ); reverse_bits( v ); std::memcpy( storage, ( const char* )( &v ), size ); }

    static type unpack( const char* storage ) { integer_type v; std::memcpy( &v, storage, size ); reverse_bits( v ); type t; std::memcpy( ( char* )( &t ), ( const char* )( &v ), size ); return t; }

    const reversed_bits& operator=( const reversed_bits& rhs ) { return base_type::operator=( rhs ); }

    const reversed_bits& operator=( type rhs ) { return base_type::operator=( rhs ); }

    const reversed_bits& operator=( integer_type rhs ) { type t; std::memcpy( ( char* )( &t ), ( const char* )( &rhs ), size ); return base_type::operator=( t ); }
};

} } // namespace comma { namespace packed {
