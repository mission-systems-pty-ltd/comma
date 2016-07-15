// This file is part of comma, a generic and flexible library
// Copyright (c) 2011 The University of Sydney
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Sydney nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
// HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include <gtest/gtest.h>
#include "../interval.h"

namespace comma { namespace math {

TEST( interval, basics )
{
    EXPECT_TRUE( math::equal( 1, 1 ) );
    EXPECT_TRUE( interval< int >( 1, 5 ).contains( 1 ) );
    EXPECT_TRUE( interval< int >( 1, 5 ).contains( 2 ) );
    EXPECT_TRUE( interval< int >( 1, 5 ).contains( 3 ) );
    EXPECT_TRUE( interval< int >( 1, 5 ).contains( 4 ) );
    EXPECT_TRUE( !interval< int >( 1, 5 ).contains( 0 ) );
    EXPECT_FALSE( interval< int >( 1, 5 ).contains( 5 ) );
    EXPECT_TRUE( interval< int >( -1, 2 ).contains( -1 ) );
    EXPECT_TRUE( interval< int >( -1, 2 ).contains( 0 ) );
    EXPECT_TRUE( interval< int >( -1, 2 ).contains( 1 ) );
    EXPECT_FALSE( interval< int >( -1, 2 ).contains( -2 ) );
    EXPECT_FALSE( interval< int >( -1, 2 ).contains( 2 ) );
    EXPECT_TRUE( interval< int >( -1, 0 ).contains( -1 ) );
    EXPECT_FALSE( interval< int >( -1, 0 ).contains( -2 ) );
    EXPECT_FALSE( interval< int >( -1, 0 ).contains( 0 ) );
    EXPECT_TRUE( interval< int >( 1, 5 ).contains( interval< int >( 1, 5 ) ) );
    EXPECT_TRUE( interval< int >( 1, 5 ).contains( interval< int >( 1, 4 ) ) );
    EXPECT_TRUE( interval< int >( 1, 5 ).contains( interval< int >( 2, 5 ) ) );
    EXPECT_TRUE( interval< int >( 1, 5 ).contains( interval< int >( 2, 4 ) ) );
    EXPECT_FALSE( interval< int >( 1, 5 ).contains( interval< int >( 0, 5 ) ) );
    EXPECT_FALSE( interval< int >( 1, 5 ).contains( interval< int >( 1, 6 ) ) );
    EXPECT_EQ( interval< int >( 1, 5 ), interval< int >( 1, 5 ) );
    EXPECT_TRUE( interval< int >( 1, 5 ) != interval< int >( 1, 6 ) );

    EXPECT_EQ( interval< int >( 1, 5 ).hull( 6 ) , interval< int >( 1, 6 ) );
    EXPECT_EQ( interval< int >( 1, 5 ).hull( interval< int >( 12, 16 ) ) , interval< int >( 1, 16 ) );
}

template < typename T >
static void test_interval_mod()
{
    EXPECT_EQ( mod( T( -182 ), interval< T >( -180, 180 ) ), T( 178 ) );
    EXPECT_EQ( mod( T( -181 ), interval< T >( -180, 180 ) ), T( 179 ) );
    EXPECT_EQ( mod( T( -180 ), interval< T >( -180, 180 ) ), T( -180 ) );
    EXPECT_EQ( mod( T( -179 ), interval< T >( -180, 180 ) ), T( -179 ) );
    EXPECT_EQ( mod( T( -1 ), interval< T >( -180, 180 ) ), T( -1 ) );
    EXPECT_EQ( mod( T( 0 ), interval< T >( -180, 180 ) ), T( 0 ) );
    EXPECT_EQ( mod( T( 1 ), interval< T >( -180, 180 ) ), T( 1 ) );
    EXPECT_EQ( mod( T( 179 ), interval< T >( -180, 180 ) ), T( 179 ) );
    EXPECT_EQ( mod( T( 180 ), interval< T >( -180, 180 ) ), T( -180 ) );
    EXPECT_EQ( mod( T( 181 ), interval< T >( -180, 180 ) ), T( -179 ) );
    EXPECT_EQ( mod( T( -361 ), interval< T >( 0, 360 ) ), T( 359 ) );
    EXPECT_EQ( mod( T( -1 ), interval< T >( 0, 360 ) ), T( 359 ) );
    EXPECT_EQ( mod( T( 0 ), interval< T >( 0, 360 ) ), T( 0 ) );
    EXPECT_EQ( mod( T( 1 ), interval< T >( 0, 360 ) ), T( 1 ) );
    EXPECT_EQ( mod( T( 359 ), interval< T >( 0, 360 ) ), T( 359 ) );
    EXPECT_EQ( mod( T( 360 ), interval< T >( 0, 360 ) ), T( 0 ) );
    EXPECT_EQ( mod( T( 361 ), interval< T >( 0, 360 ) ), T( 1 ) );
}

} } // namespace comma { namespace math {

TEST( interval, mod )
{
    comma::math::test_interval_mod< int >();
    comma::math::test_interval_mod< double >();
}

int main( int argc, char* argv[] )
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

