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
//    This product includes software developed by the University of Sydney.
// 4. Neither the name of the University of Sydney nor the
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
#include <comma/containers/cyclic_buffer.h>

namespace comma {

TEST( cyclic_buffer, basics )
{    
    cyclic_buffer< int > b( 5 );
    EXPECT_TRUE( b.empty() );
    EXPECT_EQ( b.size(), 0u );
    b.push( 4 );
    b.front() = 5;
    EXPECT_TRUE( !b.empty() );
    EXPECT_EQ( b.size(), 1u );
    EXPECT_EQ( b.front(), 5 );
    EXPECT_EQ( b.back(), 5 );
    b.push( 6 );
    EXPECT_TRUE( !b.empty() );
    EXPECT_EQ( b.size(), 2u );
    EXPECT_EQ( b.front(), 5 );
    EXPECT_EQ( b.back(), 6 );
    b.pop();
    EXPECT_TRUE( !b.empty() );
    EXPECT_EQ( b.size(), 1u );
    EXPECT_EQ( b.front(), 6 );
    EXPECT_EQ( b.back(), 6 );
    b.pop();
    EXPECT_TRUE( b.empty() );
    EXPECT_EQ( b.size(), 0u );
    b.pop();
    EXPECT_TRUE( b.empty() );
    EXPECT_EQ( b.size(), 0u );
}

TEST( cyclic_buffer, push_pop )
{
    cyclic_buffer< unsigned int > b( 5 );
    for( unsigned int i = 0; i < 5u; ++i )
    {
        EXPECT_EQ( b.size(), i );
        b.push( i );
        EXPECT_EQ( b.size(), i + 1 );
        EXPECT_EQ( b.back(), i );
    }
    try { b.push( 0 ); EXPECT_TRUE( false ); } catch ( ... ) {}
    EXPECT_EQ( b.size(), 5u );
    for( unsigned int i = 0; i < 5u; ++i )
    {
        EXPECT_EQ( b.size(), 5 - i );
        EXPECT_EQ( b.front(), i );
        b.pop();
        EXPECT_EQ( b.size(), 4 - i );
    }
    EXPECT_TRUE( b.empty() );
    EXPECT_EQ( b.size(), 0u );    
    
    for( unsigned int i = 0; i < 5u; ++i )
    {
        b.push( i );
    }    
    b.pop(4);
    EXPECT_EQ( b.size(), 1u );    
}

TEST( cyclic_buffer, fixed_cyclic_buffer )
{
    fixed_cyclic_buffer< unsigned int, 3 > b;
    EXPECT_EQ( b.size(), 3u );
    b[0] = 0;
    b[1] = 1;
    b[2] = 2;
    EXPECT_EQ( b[0], 0u );
    EXPECT_EQ( b[1], 1u );
    EXPECT_EQ( b[2], 2u );
    b >> 1;
    EXPECT_EQ( b[0], 1u );
    EXPECT_EQ( b[1], 2u );
    EXPECT_EQ( b[2], 0u );
    b >> 2;
    EXPECT_EQ( b[0], 0u );
    EXPECT_EQ( b[1], 1u );
    EXPECT_EQ( b[2], 2u );
    b << 1;
    EXPECT_EQ( b[0], 2u );
    EXPECT_EQ( b[1], 0u );
    EXPECT_EQ( b[2], 1u );    
    b << 3;
    EXPECT_EQ( b[0], 2u );
    EXPECT_EQ( b[1], 0u );
    EXPECT_EQ( b[2], 1u );    
}

} // namespace comma {

int main( int argc, char* argv[] )
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
