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


#include <comma/base/exception.h>
#include "../string.h"
#include "../split.h"
#include <gtest/gtest.h>

namespace comma {

TEST( string, split )
{
    {
        std::vector< std::string > v( split( "" ) );
        EXPECT_TRUE( v.size() == 1 );
        EXPECT_TRUE( v.at(0) == "" );
    }
    {
        std::vector< std::string > v( split( ":::", ":" ) );
        EXPECT_TRUE( v.size() == 4 );
        for( unsigned int i = 0; i < 4; ++i ) { EXPECT_TRUE( v.at(i) == "" ); }
    }
    {
        std::vector< std::string > v( split( "hello:world::moon", ":" ) );
        EXPECT_TRUE( v.size() == 4 );
        EXPECT_TRUE( v.at(0) == "hello" );
        EXPECT_TRUE( v.at(1) == "world" );
        EXPECT_TRUE( v.at(2) == "" );
        EXPECT_TRUE( v.at(3) == "moon" );
    }
    {
        std::vector< std::string > v( split( "hello:world:/moon", ":/" ) );
        EXPECT_TRUE( v.size() == 4 );
        EXPECT_TRUE( v.at(0) == "hello" );
        EXPECT_TRUE( v.at(1) == "world" );
        EXPECT_TRUE( v.at(2) == "" );
        EXPECT_TRUE( v.at(3) == "moon" );
    }
    {
        std::vector< std::string > v( split( "hello:world:/moon", "/:" ) );
        EXPECT_TRUE( v.size() == 4 );
        EXPECT_TRUE( v.at(0) == "hello" );
        EXPECT_TRUE( v.at(1) == "world" );
        EXPECT_TRUE( v.at(2) == "" );
        EXPECT_TRUE( v.at(3) == "moon" );
    }
    {
        std::vector< std::string > v( split( ":,:", ":," ) );
        EXPECT_TRUE( v.size() == 4 );
        for( unsigned int i = 0; i < 4; ++i ) { EXPECT_TRUE( v.at(i) == "" ); }
    }
}

TEST( string, split_escaped )
{
    {
        std::vector< std::string > v( split_escaped( "" ) );
        EXPECT_TRUE( v.size() == 1 );
        EXPECT_TRUE( v.at(0) == "" );
    }
    {
        std::vector< std::string > v( split_escaped( ":,:", ":," ) );
        EXPECT_TRUE( v.size() == 4 );
        for( unsigned int i = 0; i < 4; ++i ) { EXPECT_TRUE( v.at(i) == "" ); }
    }
    {
        std::vector< std::string > v( split_escaped( "abc\\", ":" ) );
        EXPECT_TRUE( v.size() == 1 );
        EXPECT_TRUE( v.at(0) == "abc\\" );
    }
    {
        std::vector< std::string > v( split_escaped( ":::", ":" ) );
        EXPECT_TRUE( v.size() == 4 );
        for( unsigned int i = 0; i < 4; ++i ) { EXPECT_TRUE( v.at(i) == "" ); }
    }
    {
        std::vector< std::string > v( split_escaped( "hello\\:world::moon", ":" ) );
        EXPECT_TRUE( v.size() == 3 );
        EXPECT_TRUE( v.at(0) == "hello:world" );
        EXPECT_TRUE( v.at(1) == "" );
        EXPECT_TRUE( v.at(2) == "moon" );
    }
    {
        std::vector< std::string > v( split_escaped( "hello\\\\:world:/moon", ":/" ) );
        EXPECT_TRUE( v.size() == 4 );
        EXPECT_TRUE( v.at(0) == "hello\\" );
        EXPECT_TRUE( v.at(1) == "world" );
        EXPECT_TRUE( v.at(2) == "" );
        EXPECT_TRUE( v.at(3) == "moon" );
    }
    {
        std::vector< std::string > v( split_escaped( "hello:\\world:/moon", "/:" ) );
        EXPECT_TRUE( v.size() == 4 );
        EXPECT_TRUE( v.at(0) == "hello" );
        EXPECT_TRUE( v.at(1) == "\\world" );
        EXPECT_TRUE( v.at(2) == "" );
        EXPECT_TRUE( v.at(3) == "moon" );
    }
    {
        std::vector< std::string > v( split_escaped( "\\\"hello\\\" world:moon", ":" ) );
        EXPECT_TRUE( v.size() == 2 );
        EXPECT_TRUE( v.at(0) == "\"hello\" world" );
        EXPECT_TRUE( v.at(1) == "moon" );
    }
    {
        std::vector< std::string > v( split_escaped( "filename;delimiter=\\;", ";" ) );
        EXPECT_TRUE( v.size() == 2 );
        EXPECT_TRUE( v.at(0) == "filename" );
        EXPECT_TRUE( v.at(1) == "delimiter=;" );
    }
}

TEST( string, split_escaped_quoted )
{
    {
        std::vector< std::string > v( split_escaped( "\"abc\"", ':', '\\', '\"' ) );
        EXPECT_TRUE( v.size() == 1 );
        EXPECT_TRUE( v.at(0) == "abc" );
    }
    {
        EXPECT_THROW( split_escaped( "a\"bc", ":", '\\', "\"" ), comma::exception );
        EXPECT_THROW( split_escaped( "abc\"", ":", '\\', "\"" ), comma::exception );
        EXPECT_THROW( split_escaped( "ab:cd\":ef", ":", '\\', "\"" ), comma::exception );
        EXPECT_THROW( split_escaped( "abc\":def", ":", '\\', "\"" ), comma::exception );
        EXPECT_THROW( split_escaped( "abc:\"def", ":", '\\', "\"" ), comma::exception );
        EXPECT_THROW( split_escaped( "\"abc\"\":def", ":", '\\', "\"" ), comma::exception );
    }
    {
        std::vector< std::string > v( split_escaped( "\"hello:world\":moon", ':', '\\', '\"' ) );
        EXPECT_TRUE( v.size() == 2 );
        EXPECT_TRUE( v.at(0) == "hello:world" );
        EXPECT_TRUE( v.at(1) == "moon" );
    }
    {
        std::vector< std::string > v( split_escaped( "\"hello\\\\:\\\"world\":moon", ':', '\\', '\"' ) );
        EXPECT_TRUE( v.size() == 2 );
        EXPECT_TRUE( v.at(0) == "hello\\:\"world" );
        EXPECT_TRUE( v.at(1) == "moon" );
    }
    {
        std::vector< std::string > v( split_escaped( "\'hello\\\\:\\\'world\':moon", ':', '\\', '\'' ) );
        EXPECT_TRUE( v.size() == 2 );
        EXPECT_TRUE( v.at(0) == "hello\\:\'world" );
        EXPECT_TRUE( v.at(1) == "moon" );
    }
}

TEST( string, strip )
{
    EXPECT_EQ( strip( "", ";" ), "" );
    EXPECT_EQ( strip( ";", ";" ), "" );
    EXPECT_EQ( strip( ";;", ";" ), "" );
    EXPECT_EQ( strip( ";;;abc", ";" ), "abc" );
    EXPECT_EQ( strip( "abc;;;", ";" ), "abc" );
    EXPECT_EQ( strip( "a;bc;;;", ";" ), "a;bc" );
    EXPECT_EQ( strip( ";;;abc;;;", ";" ), "abc" );
    EXPECT_EQ( strip( ";,;abc;;,", ";," ), "abc" );
}

} // namespace comma {

int main( int argc, char* argv[] )
{    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
