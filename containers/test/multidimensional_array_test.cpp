// Copyright (c) 2023 Vsevolod Vlaskine

#include <gtest/gtest.h>
#include "../multidimensional_array.h"

TEST( multidimensional_array, usage )
{
    //comma::multidimensional_array< double, int, 3 > m( { 0, 0, 0 }, { 1, 2, 3 } );
    //m.touch_at( { 1, 2, 3 } );
    // todo
}

TEST( multidimensional_array, impl_index )
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
