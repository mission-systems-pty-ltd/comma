// Copyright (c) 2023 Vsevolod Vlaskine

#include <gtest/gtest.h>
//#include <boost/date_time/posix_time/posix_time.hpp>
#include "../multidimensional/array.h"

namespace cmd = comma::containers::multidimensional; 

TEST( multidimensional_array, impl_index_value )
{
    EXPECT_EQ( cmd::impl::index_traits< 2 >::value( {0, 0}, {1, 5} ), 0 );
    EXPECT_EQ( cmd::impl::index_traits< 2 >::value( {0, 1}, {1, 5} ), 1 );
    EXPECT_EQ( cmd::impl::index_traits< 2 >::value( {0, 4}, {1, 5} ), 4 );
    EXPECT_EQ( cmd::impl::index_traits< 2 >::value( {1, 0}, {1, 5} ), 5 );
    EXPECT_EQ( cmd::impl::index_traits< 2 >::value( {1, 1}, {2, 5} ), 6 );
    EXPECT_EQ( cmd::impl::index_traits< 2 >::value( {1, 2}, {2, 5} ), 7 );
    EXPECT_EQ( cmd::impl::index_traits< 2 >::value( {1, 2}, {3, 5} ), 7 );
    EXPECT_EQ( cmd::impl::index_traits< 2 >::value( {2, 2}, {3, 5} ), 12 );
    EXPECT_EQ( cmd::impl::index_traits< 3 >::value( {0, 0, 0}, {2, 3, 4} ), 0 );
    EXPECT_EQ( cmd::impl::index_traits< 3 >::value( {0, 0, 3}, {2, 3, 4} ), 3 );
    EXPECT_EQ( cmd::impl::index_traits< 3 >::value( {0, 1, 3}, {2, 3, 4} ), 7 );
    EXPECT_EQ( cmd::impl::index_traits< 3 >::value( {1, 2, 3}, {2, 3, 4} ), 3 + 4 * ( 2 + 3 * 1 ) );
    EXPECT_EQ( cmd::impl::index_traits< 3 >::value( {1, 2, 4}, {2, 3, 4} ), 2 * 3 * 4 );
    EXPECT_EQ( cmd::impl::index_traits< 3 >::product( {2, 3, 4} ), 2 * 3 * 4 );
}

TEST( multidimensional_array, impl_index_product )
{
    EXPECT_EQ( cmd::impl::index_traits< 1 >::product( {2} ), 2 );
    EXPECT_EQ( cmd::impl::index_traits< 2 >::product( {2, 3} ), 6 );
    EXPECT_EQ( cmd::impl::index_traits< 3 >::product( {2, 3, 4} ), 2 * 3 * 4 );
    EXPECT_EQ( cmd::impl::index_traits< 4 >::product( {2, 3, 4, 5} ), 2 * 3 * 4 * 5 );
}

TEST( multidimensional_array, impl_index_inverted_value )
{
    typedef std::array< std::size_t, 3 > array_t;
    { array_t a{0, 0, 0}; EXPECT_EQ( cmd::impl::index_traits< 3 >::value( 0, {2, 3, 4} ), a ); }
    { array_t a{0, 0, 1}; EXPECT_EQ( cmd::impl::index_traits< 3 >::value( 1, {2, 3, 4} ), a ); }
    { array_t a{2, 1, 3}; EXPECT_EQ( cmd::impl::index_traits< 3 >::value( 3 + 4 * ( 1 + 3 * 2 ), {2, 3, 4} ), a ); }
    // todo: way more tests
}

TEST( multidimensional_array, iteration )
{
    {
        typedef std::array< std::size_t, 3 > array_t;
        cmd::array< int, 3 > a( {2, 3, 4}, 0 );
        { array_t s{2, 3, 4}; EXPECT_EQ( a.shape(), s ); EXPECT_EQ( a.data().size(), 2 * 3 * 4 ); }
        unsigned int i = 0;
        for( auto it = a.begin(); it != a.end(); ++it ) { *it = i++; }
        i = 0;
        for( auto it = a.data().begin(); it != a.data().end(); ++it ) { EXPECT_EQ( *it, i++ ); }
        auto it = a.begin();
        { array_t a{0, 0, 0}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 0 ); ++it; }
        { array_t a{0, 0, 1}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 1 ); ++it; }
        { array_t a{0, 0, 2}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 2 ); ++it; }
        { array_t a{0, 0, 3}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 3 ); ++it; }
        { array_t a{0, 1, 0}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 4 ); ++it; }
        { array_t a{0, 1, 1}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 5 ); ++it; }
        { array_t a{0, 1, 2}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 6 ); ++it; }
        { array_t a{0, 1, 3}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 7 ); ++it; }
        { array_t a{0, 2, 0}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 8 ); ++it; }
        { array_t a{0, 2, 1}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 9 ); ++it; }
        { array_t a{0, 2, 2}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 10 ); ++it; }
        { array_t a{0, 2, 3}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 11 ); ++it; }
        { array_t a{1, 0, 0}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 12 ); ++it; }
        { array_t a{1, 0, 1}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 13 ); ++it; }
        { array_t a{1, 0, 2}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 14 ); ++it; }
        { array_t a{1, 0, 3}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 15 ); ++it; }
        { array_t a{1, 1, 0}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 16 ); ++it; }
        { array_t a{1, 1, 1}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 17 ); ++it; }
        { array_t a{1, 1, 2}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 18 ); ++it; }
        { array_t a{1, 1, 3}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 19 ); ++it; }
        { array_t a{1, 2, 0}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 20 ); ++it; }
        { array_t a{1, 2, 1}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 21 ); ++it; }
        { array_t a{1, 2, 2}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 22 ); ++it; }
        { array_t a{1, 2, 3}; EXPECT_EQ( it.index(), a ); EXPECT_EQ( *it, 23 ); ++it; }
        EXPECT_TRUE( it == a.end() );
    }
}

TEST( multidimensional_array, array )
{
    {
        cmd::array< int, 3 > a( {2, 3, 4}, 0 );
        unsigned int i = 0;
        for( auto it = a.begin(); it != a.end(); ++it ) { *it = i++; }
        typedef cmd::array< int, 3 >::index_type index_t;
        { index_t i{0, 0, 0}; EXPECT_EQ( a[i], 0 ); }
        { index_t i{0, 1, 0}; EXPECT_EQ( a[i], 4 ); }
        { index_t i{1, 2, 3}; EXPECT_EQ( a[i], 23 ); }
        { index_t i{0, 0, 0}; a[{0, 0, 0}] = 111; EXPECT_EQ( a[i], 111 ); }
        { index_t i{1, 1, 2}; a[{1, 1, 2}] = 222; EXPECT_EQ( a[i], 222 ); }
        { index_t i{1, 2, 3}; a[{1, 2, 3}] = 333; EXPECT_EQ( a[i], 333 ); }
    }
}

TEST( multidimensional_array, slice )
{
    {
        {
            cmd::array< int, 3 > a( {2, 3, 4}, 0 );
            unsigned int i = 0;
            for( auto it = a.begin(); it != a.end(); ++it ) { *it = i++; }
            typedef cmd::array< int, 2 >::index_type index_t;
            cmd::slice< int, 2 > s = a.at( 0 );
            { index_t i{0, 0}; EXPECT_EQ( s[i], 0 ); }
            { index_t i{0, 1}; EXPECT_EQ( s[i], 1 ); }
            { index_t i{0, 2}; EXPECT_EQ( s[i], 2 ); }
            { index_t i{0, 3}; EXPECT_EQ( s[i], 3 ); }
            { index_t i{1, 0}; EXPECT_EQ( s[i], 4 ); }
            { index_t i{1, 1}; EXPECT_EQ( s[i], 5 ); }
            { index_t i{1, 2}; EXPECT_EQ( s[i], 6 ); }
            { index_t i{1, 3}; EXPECT_EQ( s[i], 7 ); }
            { index_t i{2, 0}; EXPECT_EQ( s[i], 8 ); }
            { index_t i{2, 1}; EXPECT_EQ( s[i], 9 ); }
            { index_t i{2, 2}; EXPECT_EQ( s[i], 10 ); }
            { index_t i{2, 3}; EXPECT_EQ( s[i], 11 ); }
            {
                auto t = s.at( 0 );
                typedef cmd::array< int, 1 >::index_type index_t;
                { index_t i{0}; EXPECT_EQ( t[i], 0 ); }
                { index_t i{1}; EXPECT_EQ( t[i], 1 ); }
                { index_t i{2}; EXPECT_EQ( t[i], 2 ); }
                { index_t i{3}; EXPECT_EQ( t[i], 3 ); }
                t = s.at( 1 );
                { index_t i{0}; EXPECT_EQ( t[i], 4 ); }
                { index_t i{1}; EXPECT_EQ( t[i], 5 ); }
                { index_t i{2}; EXPECT_EQ( t[i], 6 ); }
                { index_t i{3}; EXPECT_EQ( t[i], 7 ); }
            }
            s = a.at( 1 );
            { index_t i{0, 0}; EXPECT_EQ( s[i], 12 ); }
            { index_t i{0, 1}; EXPECT_EQ( s[i], 13 ); }
            { index_t i{0, 2}; EXPECT_EQ( s[i], 14 ); }
            { index_t i{0, 3}; EXPECT_EQ( s[i], 15 ); }
            { index_t i{1, 0}; EXPECT_EQ( s[i], 16 ); }
            { index_t i{1, 1}; EXPECT_EQ( s[i], 17 ); }
            { index_t i{1, 2}; EXPECT_EQ( s[i], 18 ); }
            { index_t i{1, 3}; EXPECT_EQ( s[i], 19 ); }
            { index_t i{2, 0}; EXPECT_EQ( s[i], 20 ); }
            { index_t i{2, 1}; EXPECT_EQ( s[i], 21 ); }
            { index_t i{2, 2}; EXPECT_EQ( s[i], 22 ); }
            { index_t i{2, 3}; EXPECT_EQ( s[i], 23 ); }
            { s[{1, 3}] = 111; std::array< std::size_t, 3 > i{1, 1, 3}; EXPECT_EQ( a[i], 111 ); }
        }
        {
            cmd::array< int, 3 > a( {2, 3, 4}, 0 );
            unsigned int i = 0;
            for( auto it = a.begin(); it != a.end(); ++it ) { *it = i++; }
            typedef cmd::array< int, 1 >::index_type index_t;
            {
                cmd::slice< int, 1 > s = a.at< 2 >( {0, 0} ); // todo! super-ugly! improve templating!
                { index_t i{0}; EXPECT_EQ( s[i], 0 ); } // todo: improve usage on 1-dimensional slices
                { index_t i{1}; EXPECT_EQ( s[i], 1 ); }
                { index_t i{2}; EXPECT_EQ( s[i], 2 ); }
                { index_t i{3}; EXPECT_EQ( s[i], 3 ); }
                s = a.at< 2 >( {0, 1} );
                { index_t i{0}; EXPECT_EQ( s[i], 4 ); }
                { index_t i{1}; EXPECT_EQ( s[i], 5 ); }
                { index_t i{2}; EXPECT_EQ( s[i], 6 ); }
                { index_t i{3}; EXPECT_EQ( s[i], 7 ); }
                s = a.at< 2 >( {0, 2} );
                { index_t i{0}; EXPECT_EQ( s[i], 8 ); }
                { index_t i{1}; EXPECT_EQ( s[i], 9 ); }
                { index_t i{2}; EXPECT_EQ( s[i], 10 ); }
                { index_t i{3}; EXPECT_EQ( s[i], 11 ); }
                s = a.at< 2 >( {1, 0} );
                { index_t i{0}; EXPECT_EQ( s[i], 12 ); }
                { index_t i{1}; EXPECT_EQ( s[i], 13 ); }
                { index_t i{2}; EXPECT_EQ( s[i], 14 ); }
                { index_t i{3}; EXPECT_EQ( s[i], 15 ); }
                s = a.at< 2 >( {1, 1} );
                { index_t i{0}; EXPECT_EQ( s[i], 16 ); }
                { index_t i{1}; EXPECT_EQ( s[i], 17 ); }
                { index_t i{2}; EXPECT_EQ( s[i], 18 ); }
                { index_t i{3}; EXPECT_EQ( s[i], 19 ); }
                s = a.at< 2 >( {1, 2} );
                { index_t i{0}; EXPECT_EQ( s[i], 20 ); }
                { index_t i{1}; EXPECT_EQ( s[i], 21 ); }
                { index_t i{2}; EXPECT_EQ( s[i], 22 ); }
                { index_t i{3}; EXPECT_EQ( s[i], 23 ); }
            }
        }
    }
}

TEST( multidimensional_array, grid_index )
{
    {
        cmd::grid< double, 2 > g( {0, 0}, {1, 1}, {2, 3}, 0 );
        typedef std::array< std::size_t, 2 > index_t;
        int i = 0;
        for( auto it = g.begin(); it != g.end(); ++it ) { *it = i++; }
        { index_t i = {0, 0}; EXPECT_EQ( g.index_of( {0, 0} ), i ); }
        { index_t i = {0, 1}; EXPECT_EQ( g.index_of( {0, 1} ), i ); }
        { index_t i = {1, 0}; EXPECT_EQ( g.index_of( {1, 0} ), i ); }
        { index_t i = {1, 1}; EXPECT_EQ( g.index_of( {1, 1} ), i ); }
        { index_t i = {0, 1}; EXPECT_EQ( g.index_of( {0, 1.01} ), i ); }
        // todo: more tests
    }
}

TEST( multidimensional_array, grid_interpolate )
{
    {
        cmd::grid< double, 2 > g( {0, 0}, {1, 1}, {2, 2}, 0 );
        g[{0, 0}] = 0; g[{0, 1}] = 1; g[{1, 0}] = 0; g[{1, 1}] = 1;
        EXPECT_EQ( g.interpolated( {0, 0} ), 0 );
        EXPECT_EQ( g.interpolated( {0, 0.5} ), 0.5 );
        EXPECT_EQ( g.interpolated( {0.5, 0.5} ), 0.5 );
        //EXPECT_EQ( g.interpolated( {0.5, 0} ), 0.5 );
        //EXPECT_EQ( g.interpolated( {1, 0} ), 2 );
        //EXPECT_EQ( g.interpolated( {1, 1} ), 3 );
    }
}

TEST( multidimensional_array, index )
{
    {
        cmd::index< 4 > i;
        EXPECT_EQ( i[0], 0 );
        EXPECT_EQ( i[1], 0 );
        EXPECT_EQ( i[2], 0 );
        EXPECT_EQ( i[3], 0 );
    }
    {
        cmd::index< 4 > i{};
        EXPECT_EQ( i[0], 0 );
        EXPECT_EQ( i[1], 0 );
        EXPECT_EQ( i[2], 0 );
        EXPECT_EQ( i[3], 0 );
    }
    {
        cmd::index< 4 > i{0u, 1u, 2u, 3u};
        EXPECT_EQ( i[0], 0 );
        EXPECT_EQ( i[1], 1 );
        EXPECT_EQ( i[2], 2 );
        EXPECT_EQ( i[3], 3 );
    }
    {
        cmd::index< 1 > i;
        cmd::index< 1 > j{5u};
        EXPECT_TRUE( i < j );
        EXPECT_TRUE( i != j );
        EXPECT_EQ( i.increment( j ), cmd::index< 1 >{1u} );
        EXPECT_EQ( i.increment( j ), cmd::index< 1 >{2u} );
    }
    {
        cmd::index< 2 > i;
        cmd::index< 2 > j{3u, 2u};
        EXPECT_EQ( i               , ( cmd::index< 2 >{0u, 0u} ) );
        EXPECT_TRUE( i < j );
        EXPECT_EQ( i.increment( j ), ( cmd::index< 2 >{0u, 1u} ) );
        EXPECT_TRUE( i < j );
        EXPECT_EQ( i.increment( j ), ( cmd::index< 2 >{1u, 0u} ) );
        EXPECT_TRUE( i < j );
        EXPECT_EQ( i.increment( j ), ( cmd::index< 2 >{1u, 1u} ) );
        EXPECT_TRUE( i < j );
        EXPECT_EQ( i.increment( j ), ( cmd::index< 2 >{2u, 0u} ) );
        EXPECT_TRUE( i < j );
        EXPECT_EQ( i.increment( j ), ( cmd::index< 2 >{2u, 1u} ) );
        EXPECT_TRUE( i < j );
        EXPECT_EQ( i.increment( j ), ( cmd::index< 2 >{0u, 0u} ) );
        EXPECT_TRUE( i == cmd::index< 2 >{} );
    }
    {
        cmd::index< 2 >::iterator i{{3u, 2u}};
        EXPECT_TRUE( bool( i ) );
        EXPECT_EQ( *i, ( cmd::index< 2 >{0u, 0u} ) );
        EXPECT_TRUE( bool( ++i ) );
        EXPECT_EQ( *i, ( cmd::index< 2 >{0u, 1u} ) );
        EXPECT_TRUE( bool( ++i ) );
        EXPECT_EQ( *i, ( cmd::index< 2 >{1u, 0u} ) );
        EXPECT_TRUE( bool( ++i ) );
        EXPECT_EQ( *i, ( cmd::index< 2 >{1u, 1u} ) );
        EXPECT_TRUE( bool( ++i ) );
        EXPECT_EQ( *i, ( cmd::index< 2 >{2u, 0u} ) );
        EXPECT_TRUE( bool( ++i ) );
        EXPECT_EQ( *i, ( cmd::index< 2 >{2u, 1u} ) );
        EXPECT_FALSE( bool( ++i ) );
        EXPECT_EQ( *i, ( cmd::index< 2 >{0u, 0u} ) );
    }
    {
        unsigned int count{0};
        for( cmd::index< 2 >::iterator i{{3u, 2u}}; i; ++i, ++count );
        EXPECT_EQ( count, 6 );
    }
}

// TEST( vector_of_vectors, performance )
// {
//     //std::pair< unsigned int, unsigned int > size{ 10000000, 4 };
//     std::pair< unsigned int, unsigned int > size{ 4096, 8 };
//     std::vector< std::vector< float > > a( size.first, std::vector< float >( size.second, 0 ) );
//     std::vector< float > b( size.first * size.second, 0 );
//     {
//         auto t0 = boost::posix_time::microsec_clock::universal_time();
//         for( auto& c: a )
//         {
//             for( auto& d: c )
//             {
//                 d += 10;
//             }
//         }
//         auto t1 = boost::posix_time::microsec_clock::universal_time();
//         for( auto& d: b )
//         {
//             d += 10;
//         }
//         auto t2 = boost::posix_time::microsec_clock::universal_time();
//         auto e0 = double( ( t1 - t0 ).total_microseconds() ) / 1e6;
//         auto e1 = double( ( t2 - t1 ).total_microseconds() ) / 1e6;
//         //std::cerr << "==> cashe hits:\tspeedup: " << ( e0 / e1 ) << "\tvector of vectors: elapsed: " << e0 << "\tvector: elapsed: " << e1 << std::endl;
//         std::cerr << "==> cashe hits:\tspeedup: " << ( e0 / e1 ) << std::endl;
//     }
//     {
//         std::vector< float > z( size.first * size.second, 0 );
//         auto t0 = boost::posix_time::microsec_clock::universal_time();
//         for( unsigned int i = 0; i < b.size(); ++i ) { z[i] = b[i]; }
//         auto t1 = boost::posix_time::microsec_clock::universal_time();
//         std::memcpy( reinterpret_cast< char* >( &z[0] ), reinterpret_cast< const char* >( &b[0] ), b.size() * sizeof( b[0] ) );
//         auto t2 = boost::posix_time::microsec_clock::universal_time();
//         auto e0 = double( ( t1 - t0 ).total_microseconds() ) / 1e6;
//         auto e1 = double( ( t2 - t1 ).total_microseconds() ) / 1e6;
//         //std::cerr << "==> memcpy:\tspeedup: " << ( e0 / e1 ) << "\tvector of vectors: elapsed: " << e0 << "\tvector: elapsed: " << e1 << std::endl;
//         std::cerr << "==> memcpy vs element-wise assignment:\tspeedup: " << ( e0 / e1 ) << std::endl;
//     }
//     {
//         std::vector< float > z( size.first * size.second, 0 );
//         auto t0 = boost::posix_time::microsec_clock::universal_time();
//         std::copy( b.begin(), b.end(), z.begin() );
//         auto t1 = boost::posix_time::microsec_clock::universal_time();
//         std::memcpy( reinterpret_cast< char* >( &z[0] ), reinterpret_cast< const char* >( &b[0] ), b.size() * sizeof( b[0] ) );
//         auto t2 = boost::posix_time::microsec_clock::universal_time();
//         auto e0 = double( ( t1 - t0 ).total_microseconds() ) / 1e6;
//         auto e1 = double( ( t2 - t1 ).total_microseconds() ) / 1e6;
//         //std::cerr << "==> memcpy:\tspeedup: " << ( e0 / e1 ) << "\tvector of vectors: elapsed: " << e0 << "\tvector: elapsed: " << e1 << std::endl;
//         std::cerr << "==> memcpy vs std::copy:\tspeedup: " << ( e0 / e1 ) << std::endl;
//     }
//     // todo! multidimensional::array performance
// }
