// Copyright (c) 2023 Vsevolod Vlaskine

#include "../exception.h"
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

} // namespace comma {

int main( int argc, char* argv[] )
{    
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
