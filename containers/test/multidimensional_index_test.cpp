// Copyright (c) 2023 Vsevolod Vlaskine

#include <gtest/gtest.h>
//#include <boost/date_time/posix_time/posix_time.hpp>
#include "../multidimensional/index.h"

namespace cmd = comma::containers::multidimensional; 

TEST( multidimentional_index, basics )
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
        cmd::index< 4 > i{0, 1, 2, 3};
        EXPECT_EQ( i[0], 0 );
        EXPECT_EQ( i[1], 1 );
        EXPECT_EQ( i[2], 2 );
        EXPECT_EQ( i[3], 3 );
    }
    {
        cmd::index< 1 > i;
        cmd::index< 1 > j{5};
        EXPECT_TRUE( i < j );
        EXPECT_TRUE( i != j );
        EXPECT_EQ( i.increment( j ), cmd::index< 1 >{1} );
        EXPECT_EQ( i.increment( j ), cmd::index< 1 >{2} );
    }
    {
        cmd::index< 2 > i;
        cmd::index< 2 > j{3, 2};
        EXPECT_EQ( i               , ( cmd::index< 2 >{0, 0} ) );
        EXPECT_TRUE( i < j );
        EXPECT_EQ( i.increment( j ), ( cmd::index< 2 >{0, 1} ) );
        EXPECT_TRUE( i < j );
        EXPECT_EQ( i.increment( j ), ( cmd::index< 2 >{1, 0} ) );
        EXPECT_TRUE( i < j );
        EXPECT_EQ( i.increment( j ), ( cmd::index< 2 >{1, 1} ) );
        EXPECT_TRUE( i < j );
        EXPECT_EQ( i.increment( j ), ( cmd::index< 2 >{2, 0} ) );
        EXPECT_TRUE( i < j );
        EXPECT_EQ( i.increment( j ), ( cmd::index< 2 >{2, 1} ) );
        EXPECT_TRUE( i < j );
        EXPECT_EQ( i.increment( j ), ( cmd::index< 2 >{0, 0} ) );
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
