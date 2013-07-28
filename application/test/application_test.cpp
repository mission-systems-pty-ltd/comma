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
#include <comma/application/command_line_options.h>
#include <comma/application/signal_flag.h>

TEST( application, command_line_options )
{
    std::vector< std::string > argv;
    argv.push_back( "hello" );
    argv.push_back( "-a" );
    argv.push_back( "b" );
    argv.push_back( "-c" );
    argv.push_back( "-d" );
    argv.push_back( "--a=true" );
    argv.push_back( "--b=15" );
    argv.push_back( "--c=hello" );
    argv.push_back( "--c" );
    argv.push_back( "world" );
    argv.push_back( "--help" );
    argv.push_back( "-h" );
    argv.push_back( "-s=5" );
    argv.push_back( "-t" );
    argv.push_back( "6" );
    comma::command_line_options options( argv );
    EXPECT_TRUE( options.exists( "-a" ) );
    EXPECT_TRUE( options.exists( "-c" ) );
    EXPECT_TRUE( options.exists( "-d" ) );
    EXPECT_TRUE( options.exists( "--a" ) );
    EXPECT_TRUE( options.exists( "--b" ) );
    EXPECT_TRUE( options.exists( "--c" ) );
    EXPECT_TRUE( options.exists( "--help" ) );
    EXPECT_TRUE( options.exists( "-h" ) );
    EXPECT_TRUE( !options.exists( "b" ) );
    EXPECT_TRUE( !options.exists( "-b" ) );
    EXPECT_TRUE( !options.exists( "--d" ) );
    EXPECT_TRUE( options.values< int >( "-x" ).empty() );
    EXPECT_TRUE( options.values< int >( "-x,-y,-z" ).empty() );
    EXPECT_EQ( options.optional< int >( "-x" ), boost::optional< int >() );
    EXPECT_EQ( options.optional< int >( "-x,-y,-z" ), boost::optional< int >() );
    EXPECT_EQ( options.value< std::string >( "-a" ), "b" );
    EXPECT_EQ( options.value< bool >( "--a" ), true );
    EXPECT_EQ( options.value< int >( "--b" ), 15 );
    EXPECT_EQ( options.values< std::string >( "--c" )[0], "hello" );
    EXPECT_EQ( options.values< std::string >( "--c" )[1], "world" );
    EXPECT_TRUE( options.exists( "-s" ) );
    EXPECT_TRUE( options.exists( "-t" ) );
    EXPECT_EQ( options.values< std::string >( "-s" ).size(), 1u );
    EXPECT_EQ( options.values< std::string >( "-t" ).size(), 1u );
    // TODO: definitely more tests!
}

TEST( application, unnamed )
{
    {
        std::vector< std::string > argv;
        argv.push_back( "hello" );
        argv.push_back( "free0" );
        argv.push_back( "-a" );
        argv.push_back( "free1" );
        argv.push_back( "-b" );
        argv.push_back( "-c" );
        argv.push_back( "--x" );
        argv.push_back( " hello" );
        argv.push_back( "free2" );
        argv.push_back( "--y=world" );
        argv.push_back( "free3" );
        argv.push_back( "--z" );
        argv.push_back( "blah" );
        argv.push_back( "free4" );
        argv.push_back( "free5" );
        comma::command_line_options options( argv );
        {
            std::vector< std::string > free = options.unnamed( "-a,-b,-c", "--x,--y,--z" );
            EXPECT_EQ( free.size(), 6u );
            EXPECT_EQ( free[0], "free0" );
            EXPECT_EQ( free[1], "free1" );
            EXPECT_EQ( free[2], "free2" );
            EXPECT_EQ( free[3], "free3" );
            EXPECT_EQ( free[4], "free4" );
            EXPECT_EQ( free[5], "free5" );
        }
        {
            std::vector< std::string > free = options.unnamed( "-a,-b,-c", "--.*" );
            EXPECT_EQ( free.size(), 6u );
            EXPECT_EQ( free[0], "free0" );
            EXPECT_EQ( free[1], "free1" );
            EXPECT_EQ( free[2], "free2" );
            EXPECT_EQ( free[3], "free3" );
            EXPECT_EQ( free[4], "free4" );
            EXPECT_EQ( free[5], "free5" );
        }
    }
    // TODO: definitely more tests!
}

int main( int argc, char* argv[] )
{
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
