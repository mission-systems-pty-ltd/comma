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
#include <comma/xpath/xpath.h>
#include <comma/base/exception.h>

TEST( xpath, contruction )
{
    {
        comma::xpath x;
        EXPECT_EQ( x.to_string(), "" );
        comma::xpath y( "hello/world[1]/moon[2]/cloud[5]/bye" );
        EXPECT_EQ( y.to_string(), "hello/world[1]/moon[2]/cloud[5]/bye" );
        x = y;
        EXPECT_EQ( x.to_string(), y.to_string() );
        EXPECT_EQ( comma::xpath( y ).to_string(), "hello/world[1]/moon[2]/cloud[5]/bye" );
        EXPECT_THROW( comma::xpath( "hello[5" ), comma::exception );
        EXPECT_THROW( comma::xpath( "[5]" ), comma::exception );
        EXPECT_THROW( comma::xpath( "hello/[5]" ), comma::exception );
    }
    {
        EXPECT_EQ( comma::xpath(), comma::xpath( "" ) );
        EXPECT_EQ( comma::xpath( "/" ), comma::xpath( "///" ) );
        EXPECT_EQ( comma::xpath( "hello/" ), comma::xpath( "hello" ) );
    }
}

TEST( xpath, comparisons )
{
    EXPECT_TRUE( !( comma::xpath() < comma::xpath() ) );
    EXPECT_TRUE( !( comma::xpath() < comma::xpath( "/" ) ) );
    EXPECT_TRUE( !( comma::xpath( "/" ) < comma::xpath( "/" ) ) );
    EXPECT_TRUE( !( comma::xpath() < comma::xpath( "hello" ) ) );
    EXPECT_TRUE( !( comma::xpath( "hello" ) < comma::xpath( "world" ) ) );
    EXPECT_TRUE( !( comma::xpath( "world" ) < comma::xpath( "hello" ) ) );
    EXPECT_TRUE( !( comma::xpath( "hello" ) < comma::xpath( "hello[10]" ) ) );
    EXPECT_TRUE( !( comma::xpath( "hello[10]/world" ) < comma::xpath( "hello/world" ) ) );
    EXPECT_TRUE( !( comma::xpath( "hello/world[0]" ) < comma::xpath( "hello/world[1]" ) ) );
    EXPECT_TRUE( !( comma::xpath( "world/cloud[7]/hello[10]" ) < comma::xpath( "world/cloud[8]/hello" ) ) );
    EXPECT_TRUE( !( comma::xpath( "world/cloud[7]/hello" ) < comma::xpath( "world/cloud/hello" ) ) );
    
    EXPECT_TRUE( comma::xpath( "/" ) < comma::xpath() );
    EXPECT_TRUE( comma::xpath( "hello" ) < comma::xpath() );
    EXPECT_TRUE( comma::xpath( "hello[10]" ) < comma::xpath( "hello" ) );
    EXPECT_TRUE( comma::xpath( "/hello[10]" ) < comma::xpath( "/hello" ) );
    EXPECT_TRUE( comma::xpath( "hello[10]/x" ) < comma::xpath( "hello" ) );
    EXPECT_TRUE( comma::xpath( "world/cloud[7]/hello[10]" ) < comma::xpath( "world/cloud[7]/hello" ) );
}

TEST( xpath, concatenation )
{
    EXPECT_EQ( comma::xpath() / comma::xpath(), comma::xpath() );
    EXPECT_EQ( comma::xpath( "hello" ) / comma::xpath(), comma::xpath( "hello" ) );
    EXPECT_EQ( comma::xpath() / comma::xpath( "hello" ), comma::xpath( "hello" ) );
    EXPECT_EQ( comma::xpath( "/" ) / comma::xpath(), comma::xpath( "/" ) );
    EXPECT_EQ( comma::xpath( "/" ) / comma::xpath( "/" ), comma::xpath( "/" ) );
    EXPECT_EQ( comma::xpath( "hello" ) / comma::xpath( "world" ), comma::xpath( "hello/world" ) );
    EXPECT_EQ( comma::xpath( "hello" ) / comma::xpath( "/world" ), comma::xpath( "hello/world" ) );
    EXPECT_EQ( comma::xpath( "hello[1]" ) / comma::xpath( "world[2]" ), comma::xpath( "hello[1]/world[2]" ) );
}

TEST( xpath, tail )
{
    EXPECT_EQ( comma::xpath().tail(), comma::xpath() );
    EXPECT_EQ( comma::xpath( "/" ).tail(), comma::xpath() );
    EXPECT_EQ( comma::xpath( "hello" ).tail(), comma::xpath() );
    EXPECT_EQ( comma::xpath( "/hello" ).tail(), comma::xpath( "hello" ) );
    EXPECT_EQ( comma::xpath( "/hello[5]" ).tail(), comma::xpath( "hello[5]" ) );
    EXPECT_EQ( comma::xpath( "hello[5]/world" ).tail(), comma::xpath( "world" ) );
    EXPECT_EQ( comma::xpath( "/hello[5]/world" ).tail(), comma::xpath( "hello[5]/world" ) );
}

TEST( xpath, head )
{
    EXPECT_EQ( comma::xpath().head(), comma::xpath() );
    EXPECT_EQ( comma::xpath( "/" ).head(), comma::xpath( "/" ) );
    EXPECT_EQ( comma::xpath( "hello" ).head(), comma::xpath() );
    EXPECT_EQ( comma::xpath( "/hello" ).head(), comma::xpath( "/" ) );
    EXPECT_EQ( comma::xpath( "hello/world" ).head(), comma::xpath( "hello" ) );
    EXPECT_EQ( comma::xpath( "/hello/world" ).head(), comma::xpath( "/hello" ) );
    EXPECT_EQ( comma::xpath( "hello[1]/world[2]" ).head(), comma::xpath( "hello[1]" ) );
}

int main( int argc, char* argv[] )
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
