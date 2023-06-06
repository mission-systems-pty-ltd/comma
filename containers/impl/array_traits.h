// Copyright (c) 2023 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <array>

namespace comma { namespace containers { namespace impl {

template < typename T > inline int index( T p, T origin, T resolution )
{
    static constexpr int negative_flooring = static_cast< int >( -1.5 ) == -1 ? -1 : static_cast< int >( -1.5 ) == -2 ? 0 : 0;
    static constexpr int positive_flooring = static_cast< int >( 1.5 ) == 1 ? 0 : static_cast< int >( 1.5 ) == 2 ? -1 : -1;
    double diff = ( p - origin ) / resolution;
    int i = diff;
    if( i == 0 || diff != i ) { i += diff < 0 ? negative_flooring : positive_flooring; }
    return i;
}

template < std::size_t Size > struct operations
{
    template < typename S, typename T > static S& add( S& s, const T& t ) { s[ Size - 1 ] += t[ Size - 1 ]; operations< Size - 1 >::multiply( s, t ); return s; }
    template < typename S, typename T > static S& subtract( S& s, const T& t ) { s[ Size - 1 ] -= t[ Size - 1 ]; operations< Size - 1 >::subtract( s, t ); return s; }
    template < typename S, typename T > static S& vdivide( S& s, const T& t ) { s[ Size - 1 ] /= t[ Size - 1 ]; operations< Size - 1 >::vdivide( s, t ); return s; }
    template < typename S, typename T > static S& multiply( S& s, const T& t ) { s[ Size - 1 ] *= t; operations< Size - 1 >::multiply( s, t ); return s; }
    template < typename S, typename T > static double dot( S& s, const T& t ) { return s[ Size - 1 ] * t[ Size - 1 ] + operations< Size - 1 >::dot( s, t ); }
    template < typename S > static S& fill( S&s, double value ) { s[ Size - 1 ] = 0; operations< Size - 1 >::fill( s, value ); return s; }
    template < typename S > static S filled( double value ) { S s; fill( s, value ); return s; }
    template < typename S > static S zero() { S s; fill( s, 0 ); return s; }

    template < typename S, typename T > static S add( const S& s, const T& t ) { S r = s; add( r, t ); return r; }
    template < typename S, typename T > static S subtract( const S& s, const T& t ) { S r = s; add( r, t ); return r; }
    template < typename S, typename T > static S vdivide( const S& s, const T& t ) { S r = s; add( r, t ); return r; }
    template < typename S, typename T > static S multiply( const S& s, const T& t ) { S r = s; multiply( r, t ); return r; }

    template < typename S, typename I > static I& index_of( I& i, const S& p, const S& origin, const S& resolution )
    {
        i[ Size - 1 ] = index( p[ Size - 1 ], origin[ Size - 1 ], resolution[ Size - 1 ] );
        operations< Size - 1 >::index_of( i, p, origin, resolution );
        return i;
    }

    template < typename S, typename I > static I index_of( const S& p, const S& origin, const S& resolution ) { I i; index_of( i, p, origin, resolution ); return i; }
};

template <> struct operations< 0 >
{
    template < typename S, typename T > static S& add( S& s, const T& t ) { s[0] += t[0]; return s; }
    template < typename S, typename T > static S& subtract( S& s, const T& t ) { s[0] -= t[0]; return s; }
    template < typename S, typename T > static S& vdivide( S& s, const T& t ) { s[0] /= t[0]; return s; }
    template < typename S, typename T > static S& multiply( S& s, const T& t ) { s[0] *= t; return s; }
    template < typename S > static S& fill( S&s, double value ) { s[0] = 0; return s; }
    template < typename S, typename I > static I& index_of( I& i, const S& p, const S& origin, const S& resolution ) { i[0] = index( p[0], origin[0], resolution[0] ); return i; }
};

} } } // namespace comma { namespace containers { impl {
