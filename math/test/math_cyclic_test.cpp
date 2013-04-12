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
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by the The University of Sydney.
// 4. Neither the name of the The University of Sydney nor the
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
#include <comma/math/cyclic.h>

namespace comma { namespace math {

TEST( cyclic, comparison )
{
    interval< int > i( 5, 10 );
    EXPECT_EQ( cyclic< int >( i ), cyclic< int >( i ) );
    EXPECT_EQ( cyclic< int >( i, 7 ), cyclic< int >( i, 7 ) );
    EXPECT_TRUE( cyclic< int >( i ) != cyclic< int >( interval< int >( 6, 10 ) ) );
    EXPECT_TRUE( cyclic< int >( i, 7 ) != cyclic< int >( interval< int >( 6, 10 ), 7 ) );
    EXPECT_TRUE( cyclic< int >( i, 7 ).between( cyclic< int >( i, 7 ), cyclic< int >( i, 8 ) ) );
    EXPECT_TRUE( cyclic< int >( i, 7 ).between( cyclic< int >( i, 6 ), cyclic< int >( i, 8 ) ) );
    EXPECT_TRUE( cyclic< int >( i, 7 ).between( cyclic< int >( i, 6 ), cyclic< int >( i, 5 ) ) );
    EXPECT_TRUE( cyclic< int >( i, 7 ).between( cyclic< int >( i, 9 ), cyclic< int >( i, 8 ) ) );
    EXPECT_TRUE( !cyclic< int >( i, 7 ).between( cyclic< int >( i, 7 ), cyclic< int >( i, 7 ) ) );
    EXPECT_TRUE( !cyclic< int >( i, 7 ).between( cyclic< int >( i, 9 ), cyclic< int >( i, 7 ) ) );
    EXPECT_TRUE( !cyclic< int >( i, 7 ).between( cyclic< int >( i, 8 ), cyclic< int >( i, 6 ) ) );
    EXPECT_TRUE( !cyclic< int >( i, 7 ).between( cyclic< int >( i, 6 ), cyclic< int >( i, 7 ) ) );
}


TEST( cyclic, increment_decrement )
{
    cyclic< int > a( interval< int >( 5, 10 ) );
    EXPECT_EQ( a(), 5 );
    EXPECT_EQ( ( ++a )(), 6 );
    EXPECT_EQ( ( ++a )(), 7 );
    EXPECT_EQ( ( ++a )(), 8 );
    EXPECT_EQ( ( ++a )(), 9 );
    EXPECT_EQ( ( ++a )(), 5 );
    EXPECT_EQ( ( ++a )(), 6 );
    EXPECT_EQ( ( --a )(), 5 );
    EXPECT_EQ( ( --a )(), 9 );
    EXPECT_EQ( ( --a )(), 8 );
    EXPECT_EQ( ( --a )(), 7 );
    EXPECT_EQ( ( --a )(), 6 );
    EXPECT_EQ( ( --a )(), 5 );
}

TEST( cyclic, addition_subtraction )
{
    {
        EXPECT_EQ( ( cyclic< int >( 5, 10 ) )(), 5 );
        EXPECT_EQ( ( cyclic< int >( 5, 10 ) + 2 )(), 7 );
        EXPECT_EQ( ( cyclic< int >( 5, 10 ) + 7 )(), 7 );
        EXPECT_EQ( ( cyclic< int >( 5, 10 ) - 0 )(), 5 );
        EXPECT_EQ( ( cyclic< int >( 5, 10 ) - 2 )(), 8 );
        EXPECT_EQ( ( cyclic< int >( 5, 10 ) - 7 )(), 8 );
        EXPECT_EQ( ( ( cyclic< int >( 5, 10 ) + 3 ) - 2 )(), 6 );
    }    
    {
        cyclic< int > a( interval< int >( 5, 10 ) );
        cyclic< int > b( interval< int >( 5, 10 ) );
        EXPECT_EQ( a + b, a );
        EXPECT_EQ( b + a, a );
        EXPECT_EQ( b + b + b + b, b );
        EXPECT_EQ( a - b, a );
        EXPECT_EQ( b - b - b - b, b );
        a = 7;
        EXPECT_EQ( a + b, a );
        EXPECT_EQ( b + a, a );
        EXPECT_EQ( a - b, a );
        EXPECT_EQ( a - 20, a );
    }
    {
        cyclic< int > a( 0, 4 );
        cyclic< int > b( 0, 4 );
        EXPECT_EQ( ( a - b )(), 0 );
        EXPECT_EQ( ( ( a + 1 ) - b )(), 1 );
        EXPECT_EQ( ( ( a + 2 ) - b )(), 2 );
        EXPECT_EQ( ( ( a + 3 ) - b )(), 3 );
        EXPECT_EQ( ( ( a + 4 ) - b )(), 0 );
        EXPECT_EQ( ( a - ( b + 1 ) )(), 3 );
        EXPECT_EQ( ( a - ( b + 2 ) )(), 2 );
        EXPECT_EQ( ( a - ( b + 3 ) )(), 1 );
        EXPECT_EQ( ( a - ( b + 4 ) )(), 0 );
        EXPECT_EQ( ( ( a + 1 ) - ( b + 1 ) )(), 0 );
        EXPECT_EQ( ( ( a + 1 ) - ( b + 2 ) )(), 3 );
        EXPECT_EQ( ( ( a + 1 ) - ( b + 3 ) )(), 2 );
        EXPECT_EQ( ( ( a + 1 ) - ( b + 4 ) )(), 1 );
    }
    {
        cyclic< int > a( 0, 5 );
        cyclic< int > b( 0, 5 );
        EXPECT_EQ( ( b - 1 )(), 4 );
        EXPECT_EQ( ( b - ( a + 1 ) )(), 4 );
        EXPECT_EQ( ( b - ( ++a ) )(), 4 );
        EXPECT_EQ( ( b - cyclic< int >( interval< int >( 0, 5 ), 1 ) )(), 4 );
    }
    {
        cyclic< unsigned int > a( 0, 5 );
        cyclic< unsigned int > b( 0, 5 );
        EXPECT_EQ( ( b - 1 )(), 4u );
        EXPECT_EQ( ( b - ( a + 1 ) )(), 4u );
        EXPECT_EQ( ( b - ( ++a ) )(), 4u );
    }
}

} } // namespace comma { namespace math {
