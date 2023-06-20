// Copyright (c) 2023 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <array>
#include <cmath>
#include <type_traits>
#include "../../math/compare.h"

namespace comma { namespace containers { namespace multidimensional { namespace impl {

template < typename T > inline int index( T p, T origin, T resolution )
{
    static constexpr int negative_flooring = static_cast< int >( -1.5 ) == -1 ? -1 : static_cast< int >( -1.5 ) == -2 ? 0 : 0;
    static constexpr int positive_flooring = static_cast< int >( 1.5 ) == 1 ? 0 : static_cast< int >( 1.5 ) == 2 ? -1 : -1;
    double diff = ( p - origin ) / resolution;
    int i = diff;
    if( i == 0 || diff != i ) { i += diff < 0 ? negative_flooring : positive_flooring; }
    return i;
}

template < int Base, unsigned int Pow > static constexpr int pow = Base * pow< Base, Pow - 1 >;
template < int Base > static constexpr int pow< Base, 0 >{1};
template < typename I, std::size_t Size > static constexpr std::array< I, Size > neighbours; // quick and dirty, for now leaving it to the enthusiasts to implement it using metaprogramming
template < typename I > static constexpr std::array< I, 2 > neighbours< I, 1 > = {{ {{ 0 }}, {{ 1 }} }};
template < typename I > static constexpr std::array< I, 4 > neighbours< I, 2 > = {{ {{ 0, 0 }}, {{ 0, 1 }}, {{ 1, 0 }}, {{ 1, 1 }} }};
template < typename I > static constexpr std::array< I, 8 > neighbours< I, 3 > = {{ {{ 0, 0, 0 }}, {{ 0, 0, 1 }}, {{ 0, 1, 0 }}, {{ 0, 1, 1 }}, {{ 1, 0, 0 }}, {{ 1, 0, 1 }}, {{ 1, 1, 0 }}, {{ 1, 1, 1 }} }};
template < typename I > static constexpr std::array< I, 16 > neighbours< I, 4 > = {{ {{ 0, 0, 0, 0 }}, {{ 0, 0, 0, 1 }}, {{ 0, 0, 1, 0 }}, {{ 0, 0, 1, 1 }}, {{ 0, 1, 0, 0 }}, {{ 0, 1, 0, 1 }}, {{ 0, 1, 1, 0 }}, {{ 0, 1, 1, 1 }}, {{ 1, 0, 0, 0 }}, {{ 1, 0, 0, 1 }}, {{ 1, 0, 1, 0 }}, {{ 1, 0, 1, 1 }}, {{ 1, 1, 0, 0 }}, {{ 1, 1, 0, 1 }}, {{ 1, 1, 1, 0 }}, {{ 1, 1, 1, 1 }} }};
// todo: add more dimensions as required or write that little metaprogramming piece

template < std::size_t Size > struct operations
{
    template < typename S, typename T > static S& add( S& s, const T& t ) { s[ Size - 1 ] += t[ Size - 1 ]; operations< Size - 1 >::add( s, t ); return s; }
    template < typename S, typename T > static S& subtract( S& s, const T& t ) { s[ Size - 1 ] -= t[ Size - 1 ]; operations< Size - 1 >::subtract( s, t ); return s; }
    template < typename S, typename T > static S& vdivide( S& s, const T& t ) { s[ Size - 1 ] /= t[ Size - 1 ]; operations< Size - 1 >::vdivide( s, t ); return s; }
    template < typename S, typename T > static S& multiply( S& s, const T& t ) { s[ Size - 1 ] *= t; operations< Size - 1 >::multiply( s, t ); return s; }
    template < typename S, typename T > static S& vmultiply( S& s, const T& t ) { s[ Size - 1 ] *= t[ Size - 1 ]; operations< Size - 1 >::vmultiply( s, t ); return s; }
    template < typename S, typename T > static double dot( S& s, const T& t ) { return s[ Size - 1 ] * t[ Size - 1 ] + operations< Size - 1 >::dot( s, t ); }
    template < typename S, typename T > static S& mask( S& s, const T& t ) { s[ Size - 1 ] = t[ Size - 1 ] ? t[ Size - 1 ] : s[ Size - 1 ]; operations< Size - 1 >::mask( s, t ); return s; }
    template < typename S, typename T > static S masked( const S& s, const T& t ) { S m = s; mask( m, t ); return m; }
    template < typename S, typename T > static S& logical_not( S& s ) { s[ Size - 1 ] = !s[ Size - 1 ]; operations< Size - 1 >::logical_not( s ); return s; }
    template < typename S > static S& fill( S&s, double value ) { s[ Size - 1 ] = value; operations< Size - 1 >::fill( s, value ); return s; }
    template < typename S > static S filled( double value ) { S s; fill( s, value ); return s; }
    template < typename S > static S zero() { S s; fill( s, 0 ); return s; }
    template < typename S > static auto product( const S& s ) -> typename std::remove_reference< decltype( s[Size] ) >::type { return s[ Size - 1 ] * operations< Size - 1 >::product( s ); }
    template < typename S, typename T, typename Diff > static bool near( const S& s, const T& t, const Diff& epsilon ) { return comma::math::equal( s[ Size - 1 ], t[ Size - 1 ], epsilon ) && operations< Size - 1 >::near( s, t, epsilon ); }
    template < typename S, typename T > static S add( const S& s, const T& t ) { S r = s; add( r, t ); return r; }
    template < typename S, typename T > static S subtract( const S& s, const T& t ) { S r = s; subtract( r, t ); return r; }
    template < typename S, typename T > static S vdivide( const S& s, const T& t ) { S r = s; vdivide( r, t ); return r; }
    template < typename S, typename T > static S multiply( const S& s, const T& t ) { S r = s; multiply( r, t ); return r; }
    template < typename S, typename T > static S vmultiply( const S& s, const T& t ) { S r = s; vmultiply( r, t ); return r; }

    template < typename S, typename I > static I& index_of( I& i, const S& p, const S& origin, const S& resolution )
    {
        i[ Size - 1 ] = index( p[ Size - 1 ], origin[ Size - 1 ], resolution[ Size - 1 ] );
        operations< Size - 1 >::index_of( i, p, origin, resolution );
        return i;
    }

    template < typename S, typename I > static I index_of( const S& p, const S& origin, const S& resolution ) { I i; index_of( i, p, origin, resolution ); return i; }

    template < typename S, typename I > static I nearest( const S& p, const S& origin, const S& resolution ) // todo? metaprogram?
    {
        const S& s = subtract( p, origin );
        double m = dot( s, s );
        unsigned int j = 0;
        const auto& n = neighbours< I, Size >;
        for( unsigned int i = 1; m > 0 && i < n.size(); ++i )
        {
            const S& r = subtract( vmultiply( resolution, n[i] ), s );
            double d = dot( r, r );
            if( d < m ) { m = d; j = i; }
        }
        return n[j];
    }

    template < typename S, typename I > static std::array< double, pow< 2, Size > > squared_norms( const S& p, const S& origin, const S& resolution ) // todo? metaprogram?
    {
        const S& s = subtract( p, origin );
        std::array< double, pow< 2, Size > > d;
        const auto& n = neighbours< I, Size >;
        for( unsigned int i = 0; i < n.size(); ++i )
        {
            const S& r = subtract( vmultiply( resolution, n[i] ), s );
            d[i] = dot( r, r );
        }
        return d;
    }

    template < typename S, typename I > static std::array< double, pow< 2, Size > > norms( const S& p, const S& origin, const S& resolution )
    {
        std::array< double, pow< 2, Size > > d = squared_norms( p, origin, resolution );
        for( auto& v: d ) { v = std::sqrt( v ); }
        return d;
    }

    struct interpolation
    {
        struct linear
        {
            template < typename S > static std::array< double, pow< 2, Size > > weights( const S& p, const S& origin, const S& resolution )
            {
                S s = vdivide( subtract( p, origin ), resolution );
                S t;
                subtract( fill( t, 1. ), s );
                const auto& n = neighbours< std::array< unsigned int, Size >, Size >;
                std::array< double, pow< 2, Size > > w;
                for( unsigned int i = 0; i < pow< 2, Size >; ++i ) { w[i] = product( masked( s, n[ pow< 2, Size > - i - 1 ] ) ) * product( masked( t, n[i] ) ); }
                return w;
            }
        };
    };
};

template <> struct operations< 1 >
{
    template < typename S, typename T > static S& add( S& s, const T& t ) { s[0] += t[0]; return s; }
    template < typename S, typename T > static S& subtract( S& s, const T& t ) { s[0] -= t[0]; return s; }
    template < typename S, typename T > static S& vdivide( S& s, const T& t ) { s[0] /= t[0]; return s; }
    template < typename S, typename T > static S& multiply( S& s, const T& t ) { s[0] *= t; return s; }
    template < typename S, typename T > static S& vmultiply( S& s, const T& t ) { s[0] *= t[0]; return s; }
    template < typename S, typename T > static double dot( S& s, const T& t ) { return s[0] * t[0]; }
    template < typename S, typename T > static S& logical_not( S& s ) { s[0] = !s[0]; return s; }
    template < typename S, typename T > static S& mask( S& s, const T& t ) { s[0] = t[0] ? t[0] : s[0]; return s; }
    template < typename S > static auto product( const S& s ) -> typename std::remove_reference< decltype( s[0] ) >::type { return s[0]; }
    template < typename S, typename T, typename Diff > static bool near( const S& s, const T& t, const Diff& epsilon ) { return comma::math::equal( s[0], t[0], epsilon ); }
    template < typename S > static S& fill( S&s, double value ) { s[0] = value; return s; }
    template < typename S > static S zero() { S s; fill( s, 0 ); return s; }
    template < typename S, typename I > static I& index_of( I& i, const S& p, const S& origin, const S& resolution ) { i[0] = index( p[0], origin[0], resolution[0] ); return i; }
    template < typename S, typename I > static I nearest( const S& p, const S& origin, const S& resolution ) { return p[0] - origin[0] < resolution[0] / 2 ? I{ 0 } : I{ 1 }; }
};

} } } } // namespace comma { namespace containers { namespace multidimensional { namespace impl {
