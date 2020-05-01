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


#include "../../base/exception.h"
#include "../string.h"
#include "../split.h"
#include <list>
#include <gtest/gtest.h>

namespace comma {

TEST( string, join )
{
    {
        std::vector< std::string > v;
        std::string j = join( v, ',' );
        EXPECT_TRUE( j == "" );
    }
    {
        std::vector< std::string > v;
        v.push_back( "one" );
        std::string j = join( v, ',' );
        EXPECT_TRUE( j == "one" );
    }
    {
        std::vector< std::string > v;
        v.push_back( "one" );
        v.push_back( "two" );
        std::string j = join( v, ',' );
        EXPECT_TRUE( j == "one,two" );
    }
    {
        std::vector< std::string > v;
        v.push_back( "one" );
        v.push_back( "two" );
        v.push_back( "three" );
        std::string j = join( v, ',' );
        EXPECT_TRUE( j == "one,two,three" );
    }
    {
        std::vector< std::string > v;
        v.push_back( "one" );
        v.push_back( "" );
        v.push_back( "three" );
        std::string j = join( v, ',' );
        EXPECT_TRUE( j == "one,,three" );
    }
    {
        std::list< std::string > v;
        v.push_back( "one" );
        v.push_back( "two" );
        v.push_back( "three" );
        std::string j = join( v.begin(), v.end(), ',' );
        EXPECT_TRUE( j == "one,two,three" );
    }
}

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

TEST( string, escape )
{
    EXPECT_EQ( "ab", escape( "ab" ) );
    EXPECT_EQ( "ab\\\\", escape( "ab\\" ) );
    EXPECT_EQ( "\\\\ab", escape( "\\ab" ) );
    EXPECT_EQ( "a\\\\b", escape( "a\\b" ) );
    EXPECT_EQ( "a\\'b", escape( "a'b" ) );
    EXPECT_EQ( "a\\b", escape( "a\\b", "", '~' ) );
    EXPECT_EQ( "a~~b", escape( "a~b", "", '~' ) );
    EXPECT_EQ( "a\\\'b", escape( "a\'b" ) );
    EXPECT_EQ( "a~\"b", escape( "a\"b", "\"", '~' ) );
    EXPECT_EQ( "a~\'b~\'c", escape( "a\'b\'c", "\'", '~' ) );
    EXPECT_EQ( "a~\"b~\"c", escape( "a\"b\"c", "\"", '~' ) );
    EXPECT_EQ( "a~\"b\'c", escape( "a\"b\'c", "\"", '~' ) );
    EXPECT_EQ( "a~\"b;", escape( "a\"b;", "\"", '~' ) );
    EXPECT_EQ( "a~\"b~;", escape( "a\"b;", "\";", '~' ) );
    EXPECT_EQ( "ab~1~2~34~~5", escape( "ab1234~5", "123", '~' ) );
}

TEST( string, unescape )
{
    EXPECT_EQ( "ab", unescape( "ab" ) );
    EXPECT_EQ( "ab\\", unescape( "ab\\\\" ) );
    EXPECT_EQ( "\\ab", unescape( "\\\\ab", "" ) );
    EXPECT_EQ( "a\\b", unescape( "a\\\\b", "" ) );
    EXPECT_EQ( "a\\b", unescape( "a\\b" ) );
    EXPECT_EQ( "ab\\", unescape( "ab\\" ) );
    EXPECT_EQ( "a'b", unescape( "a\\'b" ) );
    EXPECT_EQ( "a'b", unescape( "a'b" ) );
    EXPECT_EQ( "a\\\\b", unescape( "a\\\\b", "", '~' ) );
    EXPECT_EQ( "a~b", unescape( "a~~b", "", '~' ) );
    EXPECT_EQ( "a\"b", unescape( "a~\"b", "\"", '~' ) );
    EXPECT_EQ( "a\"b;", unescape( "a\"b;", "\"", '~' ) );
    EXPECT_EQ( "a\"b\';", unescape( "a\"b\';", "\"\'", '~' ) );
    EXPECT_EQ( "ab1234~5", unescape( "ab~1~2~34~~5", "123", '~' ) );

    EXPECT_EQ( "ab1234~5", unescape( escape( "ab1234~5", "123", '~' ), "123", '~' ) );    
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
        std::vector< std::string > v( split_escaped( "\\\'hello\\\' world:moon", ":" ) );
        EXPECT_TRUE( v.size() == 2 );
        EXPECT_TRUE( v.at(0) == "\'hello\' world" );
        EXPECT_TRUE( v.at(1) == "moon" );
    }
    {
        std::vector< std::string > v( split_escaped( "filename;delimiter=\\;", ";" ) );
        EXPECT_TRUE( v.size() == 2 );
        EXPECT_TRUE( v.at(0) == "filename" );
        EXPECT_TRUE( v.at(1) == "delimiter=;" );
    }
    {
        std::vector< std::string > v( split_escaped( "filename;delimiter=\\;;fields=a,b,c", ";" ) );
        EXPECT_TRUE( v.size() == 3 );
        EXPECT_TRUE( v.at(0) == "filename" );
        EXPECT_TRUE( v.at(1) == "delimiter=;" );
        EXPECT_TRUE( v.at(2) == "fields=a,b,c" );
    }
}

TEST( string, split_escaped_quoted )
{
    {
        std::vector< std::string > v( split_escaped( "\"abc\"", ':', "\"", '\\' ) );
        EXPECT_TRUE( v.size() == 1 );
        EXPECT_TRUE( v.at(0) == "abc" );
    }
    {
        EXPECT_THROW( split_escaped( "a\"bc", ":", "\"", '\\' ), comma::exception );
        EXPECT_THROW( split_escaped( "\"abc\"\":def", ":", "\"", '\\' ), comma::exception );
    }
    {
        std::vector< std::string > v( split_escaped( "\"hello:world\":moon", ':', "\"", '\\' ) );
        EXPECT_TRUE( v.size() == 2 );
        EXPECT_TRUE( v.at(0) == "hello:world" );
        EXPECT_TRUE( v.at(1) == "moon" );
    }
    {
        std::vector< std::string > v( split_escaped( "\"hello\\\\:\\\"world\":moon", ':', "\"", '\\' ) );
        EXPECT_TRUE( v.size() == 2 );
        EXPECT_TRUE( v.at(0) == "hello\\:\"world" );
        EXPECT_TRUE( v.at(1) == "moon" );
    }
    {
        std::vector< std::string > v( split_escaped( "\'hello\\\\:\\\'world\':moon", ':', "\'", '\\') );
        EXPECT_TRUE( v.size() == 2 );
        EXPECT_TRUE( v.at(0) == "hello\\:\'world" );
        EXPECT_TRUE( v.at(1) == "moon" );
    }
    {
        std::vector< std::string > v( split_escaped( "filename;delimiter=\';\'", ";" ) );
        EXPECT_TRUE( v.size() == 2 );
        EXPECT_TRUE( v.at(0) == "filename" );
        EXPECT_TRUE( v.at(1) == "delimiter=;" );
    }
    {
        std::vector< std::string > v( split_escaped( "filename;delimiter=\';\';fields=a,b,c", ";" ) );
        EXPECT_TRUE( v.size() == 3 );
        EXPECT_TRUE( v.at(0) == "filename" );
        EXPECT_TRUE( v.at(1) == "delimiter=;" );
        EXPECT_TRUE( v.at(2) == "fields=a,b,c" );
    }
    {
        std::vector< std::string > v( split_escaped( "b='y';c/d=\\z;e=\\;;a='x;'", ';', "\'", '\\') );
        EXPECT_TRUE( v.size() == 4 );
        EXPECT_TRUE( v.at(0) == "b=y" );
        EXPECT_TRUE( v.at(1) == "c/d=\\z" );
        EXPECT_TRUE( v.at(2) == "e=;" );
        EXPECT_TRUE( v.at(3) == "a=x;" );
    }
}

TEST( string, split_bracketed )
{
    {
        std::vector< std::string > v( split_bracketed( "" ) );
        EXPECT_EQ( 1u, v.size() );
        EXPECT_EQ( "", v[0] );
    }
    {
        std::vector< std::string > v( split_bracketed( "()", ',' ) );
        EXPECT_EQ( 1u, v.size() );
        EXPECT_EQ( "", v[0] );
    }
    {
        std::vector< std::string > v( split_bracketed( "(),(),()", ',' ) );
        EXPECT_EQ( 3u, v.size() );
        EXPECT_EQ( "", v[0] );
        EXPECT_EQ( "", v[1] );
        EXPECT_EQ( "", v[2] );
    }
    {
        std::vector< std::string > v( split_bracketed( ")()", ',', '(', ')', false ) );
        EXPECT_EQ( 1u, v.size() );
        EXPECT_EQ( ")()", v[0] );
    }
    {
        std::vector< std::string > v( split_bracketed( "(),(,),(,)", ',' ) );
        EXPECT_EQ( 3u, v.size() );
        EXPECT_EQ( "", v[0] );
        EXPECT_EQ( ",", v[1] );
        EXPECT_EQ( ",", v[2] );
    }
    {
        std::vector< std::string > v( split_bracketed( "a,[,b,[c]],d", ',', '[', ']' ) );
        EXPECT_EQ( 3u, v.size() );
        EXPECT_EQ( "a", v[0] );
        EXPECT_EQ( ",b,[c]", v[1] );
        EXPECT_EQ( "d", v[2] );
    }
    {
        std::vector< std::string > v( split_bracketed( "a,( b, c, d ),( f ( g, h ) ), i", ',' ) );
        EXPECT_EQ( 4u, v.size() );
        EXPECT_EQ( "a", v[0] );
        EXPECT_EQ( " b, c, d ", v[1] );
        EXPECT_EQ( " f ( g, h ) ", v[2] );
        EXPECT_EQ( " i", v[3] );
    }
    {
        std::vector< std::string > v( split_bracketed( "a,( b, c, d ),( f ( g, h ) ), i", ',', '(', ')', false ) );
        EXPECT_EQ( 4u, v.size() );
        EXPECT_EQ( "a", v[0] );
        EXPECT_EQ( "( b, c, d )", v[1] );
        EXPECT_EQ( "( f ( g, h ) )", v[2] );
        EXPECT_EQ( " i", v[3] );
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

TEST( string, common_head )
{
    EXPECT_EQ( "", common_front( "", "" ) );
    EXPECT_EQ( "", common_front( "", "a" ) );
    EXPECT_EQ( "", common_front( "", "ab" ) );
    EXPECT_EQ( "", common_front( "", "abc" ) );
    EXPECT_EQ( "", common_front( "a", "" ) );
    EXPECT_EQ( "", common_front( "ab", "" ) );
    EXPECT_EQ( "", common_front( "abc", "" ) );
    EXPECT_EQ( "", common_front( "a", "b" ) );
    EXPECT_EQ( "", common_front( "abc", "def" ) );
    EXPECT_EQ( "a", common_front( "ab", "ac" ) );
    EXPECT_EQ( "ab", common_front( "abc", "abd" ) );
}

TEST( string, common_head_delimiter )
{
    EXPECT_EQ( common_front( "", "", '/' ), "" );
    EXPECT_EQ( common_front( "a", "b", '/' ), "" );
    EXPECT_EQ( common_front( "ab", "cd", '/' ), "" );
    EXPECT_EQ( common_front( "ab", "abc", '/' ), "" );
    EXPECT_EQ( common_front( "/", "/", '/' ), "/" );
    EXPECT_EQ( common_front( "/a", "/b", '/' ), "/" );
    EXPECT_EQ( common_front( "/ab", "/cd", '/' ), "/" );
    EXPECT_EQ( common_front( "/ab", "/abc", '/' ), "/" );
    EXPECT_EQ( common_front( "/ab/", "/abc", '/' ), "/" );
    EXPECT_EQ( common_front( "/ab/", "/abc/", '/' ), "/" );
    EXPECT_EQ( common_front( "a/b", "a/c", '/' ), "a" );
    EXPECT_EQ( common_front( "a/b/", "a/c", '/' ), "a" );
    EXPECT_EQ( common_front( "a/b", "a/c/", '/' ), "a" );
    EXPECT_EQ( common_front( "a/b/", "a/c/", '/' ), "a" );
    EXPECT_EQ( common_front( "/a/b", "/a/c", '/' ), "/a" );
    EXPECT_EQ( common_front( "ab/cd", "ab/cd", '/' ), "ab/cd" );
    EXPECT_EQ( common_front( "ab/cd/", "ab/cd/", '/' ), "ab/cd" );
    EXPECT_EQ( common_front( "ab/cd/ef", "ab/cd/xy", '/' ), "ab/cd" );
    EXPECT_EQ( common_front( "ab/cd/", "ab/cd", '/' ), "ab/cd" );
    EXPECT_EQ( common_front( "ab/cd", "ab/cd/", '/' ), "ab/cd" );
    EXPECT_EQ( common_front( "ab/cd/ef", "ab/cd", '/' ), "ab/cd" );
    EXPECT_EQ( common_front( "ab/cd", "ab/cd/ef", '/' ), "ab/cd" );
    EXPECT_EQ( common_front( "ab/cd/ef/", "ab/cd", '/' ), "ab/cd" );
    EXPECT_EQ( common_front( "ab/cd/ef/", "ab/cd/", '/' ), "ab/cd" );
    EXPECT_EQ( common_front( "ab/cd/ef", "ab/cd/", '/' ), "ab/cd" );
}

} // namespace comma {

int main( int argc, char* argv[] )
{    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
