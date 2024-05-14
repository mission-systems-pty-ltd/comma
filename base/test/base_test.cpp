// Copyright (c) 2023 Vsevolod Vlaskine

#include "../exception.h"
#include "../variant.h"
#include <gtest/gtest.h>

namespace comma {

TEST( base, exception )
{
    COMMA_ASSERT( true, "all good" );
    COMMA_ASSERT( 2 * 2 == 4, "all good" );
    EXPECT_THROW( COMMA_ASSERT( false, "all bad" ), comma::exception );
    EXPECT_THROW( COMMA_ASSERT( 2 * 2 == 5, "all bad" ), comma::exception );
    COMMA_THROW_IF( false, "all good" );
    COMMA_THROW_IF( 2 * 2 == 5, "all good" );
    EXPECT_THROW( COMMA_THROW_IF( true, "all bad" ), comma::exception );
    EXPECT_THROW( COMMA_THROW_IF( 2 * 2 == 4, "all bad" ), comma::exception );
}

TEST( base, variant )
{
    {
        comma::impl::variant< int, float, double > v;
        v.t = 1;
        v.values.t = 2;
        v.values.values.t = 3;
    }
    {
        comma::impl::variant< int, float, double > v;
        EXPECT_FALSE( v.is< int >() );
        EXPECT_FALSE( v.is< float >() );
        EXPECT_FALSE( v.is< double >() );
        v.set< int >( 5 );
        EXPECT_EQ( *v.t, 5 );
        EXPECT_TRUE( v.is< int >() );
        EXPECT_FALSE( v.is< float >() );
        EXPECT_FALSE( v.is< double >() );
        v.set< float >( 5 );
        EXPECT_EQ( *v.values.t, 5 );
        EXPECT_FALSE( v.is< int >() );
        EXPECT_TRUE( v.is< float >() );
        EXPECT_FALSE( v.is< double >() );
        v.set< double >( 5 );
        EXPECT_EQ( *v.values.values.t, 5 );
        EXPECT_FALSE( v.is< int >() );
        EXPECT_FALSE( v.is< float >() );
        EXPECT_TRUE( v.is< double >() );
        v.set< int >( 5 );
        EXPECT_EQ( *v.t, 5 );
        EXPECT_TRUE( v.is< int >() );
        EXPECT_FALSE( v.is< float >() );
        EXPECT_FALSE( v.is< double >() );
    }
}

} // namespace comma {

int main( int argc, char* argv[] )
{    
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
