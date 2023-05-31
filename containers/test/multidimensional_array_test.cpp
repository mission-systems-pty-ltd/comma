// Copyright (c) 2023 Vsevolod Vlaskine

#include <gtest/gtest.h>
#include "../multidimensional_array.h"

TEST( multidimensional_array, usage )
{
    // todo
}

TEST( multidimensional_array, impl_index_value )
{
    EXPECT_EQ( comma::impl::index< 2 >::value( {0, 0}, {1, 5} ), 0 );
    EXPECT_EQ( comma::impl::index< 2 >::value( {0, 1}, {1, 5} ), 1 );
    EXPECT_EQ( comma::impl::index< 2 >::value( {0, 4}, {1, 5} ), 4 );
    EXPECT_EQ( comma::impl::index< 2 >::value( {1, 0}, {1, 5} ), 5 );
    EXPECT_EQ( comma::impl::index< 2 >::value( {1, 1}, {2, 5} ), 6 );
    EXPECT_EQ( comma::impl::index< 2 >::value( {1, 2}, {2, 5} ), 7 );
    EXPECT_EQ( comma::impl::index< 2 >::value( {1, 2}, {3, 5} ), 7 );
    EXPECT_EQ( comma::impl::index< 2 >::value( {2, 2}, {3, 5} ), 12 );
    EXPECT_EQ( comma::impl::index< 3 >::value( {0, 0, 0}, {2, 3, 4} ), 0 );
    EXPECT_EQ( comma::impl::index< 3 >::value( {0, 0, 3}, {2, 3, 4} ), 3 );
    EXPECT_EQ( comma::impl::index< 3 >::value( {0, 1, 3}, {2, 3, 4} ), 7 );
    EXPECT_EQ( comma::impl::index< 3 >::value( {2, 1, 3}, {2, 3, 4} ), 3 + 4 * ( 1 + 3 * 2 ) );
}

TEST( multidimensional_array, impl_index_inverted_value )
{
    typedef std::array< std::size_t, 3 > array_t;
    { array_t a{0, 0, 0}; EXPECT_EQ( comma::impl::index< 3 >::value( 0, {2, 3, 4} ), a ); }
    { array_t a{0, 0, 1}; EXPECT_EQ( comma::impl::index< 3 >::value( 1, {2, 3, 4} ), a ); }
    { array_t a{2, 1, 3}; EXPECT_EQ( comma::impl::index< 3 >::value( 3 + 4 * ( 1 + 3 * 2 ), {2, 3, 4} ), a ); }
    // todo: way more tests
}

TEST( multidimensional_array, basics )
{
    {
        comma::multidimensional_array< int, 3 > a( {2, 3, 4}, 0 );
        unsigned int i = 0;
        for( auto it = a.begin(); it != a.end(); ++it ) { *it = i++; }
        EXPECT_EQ( a( {2, 1, 3} ), 3 + 4 * ( 1 + 3 * 2 ) );
        // todo: more tests
    }
}
