// Copyright (c) 2023 Vsevolod Vlaskine

#include <array>
#include <gtest/gtest.h>
#include "../multidimensional/array.h"

namespace cci = comma::containers::impl; 

TEST( array_traits, operations_nearest )
{
    {
        typedef std::array< double, 1 > point_t;
        typedef std::array< int, 1 > index_t;
        { index_t i = cci::operations< 1 >::nearest< point_t, index_t >( point_t{0}, point_t{0}, point_t{1} ); EXPECT_EQ( index_t{0}, i ); }
        { index_t i = cci::operations< 1 >::nearest< point_t, index_t >( point_t{0.3}, point_t{0}, point_t{1} ); EXPECT_EQ( index_t{0}, i ); }
        { index_t i = cci::operations< 1 >::nearest< point_t, index_t >( point_t{0.5}, point_t{0}, point_t{1} ); EXPECT_EQ( index_t{1}, i ); }
        { index_t i = cci::operations< 1 >::nearest< point_t, index_t >( point_t{0.999}, point_t{0}, point_t{1} ); EXPECT_EQ( index_t{1}, i ); }
        { index_t i = cci::operations< 1 >::nearest< point_t, index_t >( point_t{0}, point_t{0}, point_t{1} ); EXPECT_EQ( index_t{0}, i ); }
        { index_t i = cci::operations< 1 >::nearest< point_t, index_t >( point_t{0.3}, point_t{0}, point_t{1} ); EXPECT_EQ( index_t{0}, i ); }
        { index_t i = cci::operations< 1 >::nearest< point_t, index_t >( point_t{0.5}, point_t{0}, point_t{1} ); EXPECT_EQ( index_t{1}, i ); }
        { index_t i = cci::operations< 1 >::nearest< point_t, index_t >( point_t{0.999}, point_t{0}, point_t{1} ); EXPECT_EQ( index_t{1}, i ); }
    }
    {
        typedef std::array< double, 2 > point_t;
        typedef std::array< int, 2 > index_t;
        { index_t expected{0, 0}; index_t actual = cci::operations< 2 >::nearest< point_t, index_t >( point_t{0, 0}, point_t{0, 0}, point_t{1, 1} ); EXPECT_EQ( expected, actual ); }        
        { index_t expected{0, 0}; index_t actual = cci::operations< 2 >::nearest< point_t, index_t >( point_t{0.1, 0}, point_t{0, 0}, point_t{1, 1} ); EXPECT_EQ( expected, actual ); }
        { index_t expected{0, 0}; index_t actual = cci::operations< 2 >::nearest< point_t, index_t >( point_t{0, 0.1}, point_t{0, 0}, point_t{1, 1} ); EXPECT_EQ( expected, actual ); }
        { index_t expected{0, 0}; index_t actual = cci::operations< 2 >::nearest< point_t, index_t >( point_t{0.1, 0.2}, point_t{0, 0}, point_t{1, 1} ); EXPECT_EQ( expected, actual ); }
        { index_t expected{0, 1}; index_t actual = cci::operations< 2 >::nearest< point_t, index_t >( point_t{0, 0.7}, point_t{0, 0}, point_t{1, 1} ); EXPECT_EQ( expected, actual ); }
        { index_t expected{0, 1}; index_t actual = cci::operations< 2 >::nearest< point_t, index_t >( point_t{0.1, 0.7}, point_t{0, 0}, point_t{1, 1} ); EXPECT_EQ( expected, actual ); }
        { index_t expected{1, 1}; index_t actual = cci::operations< 2 >::nearest< point_t, index_t >( point_t{1, 1}, point_t{0, 0}, point_t{1, 1} ); EXPECT_EQ( expected, actual ); }
    }
}
