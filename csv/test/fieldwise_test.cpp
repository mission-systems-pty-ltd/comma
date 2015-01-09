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
#include <comma/csv/ascii.h>
#include <comma/csv/impl/fieldwise.h>

TEST( fieldwise, non_typed_ascii )
{
    EXPECT_TRUE( comma::csv::fieldwise( "" ).ascii().equal( "", "" ) );
    EXPECT_TRUE( comma::csv::fieldwise( "" ).ascii().equal( "a,b", "c,d" ) );
    EXPECT_FALSE( comma::csv::fieldwise( "a" ).ascii().equal( "a,b", "c,b" ) );
    EXPECT_FALSE( comma::csv::fieldwise( ",b" ).ascii().equal( "a,c", "a,d" ) );
    EXPECT_TRUE( comma::csv::fieldwise( "a,b" ).ascii().equal( "a,b", "a,b" ) );
    EXPECT_FALSE( comma::csv::fieldwise( "a,b" ).ascii().equal( "a,c", "a,d" ) );
    EXPECT_TRUE( comma::csv::fieldwise( "x" ).ascii().equal( "a,b,c", "a,x,x" ) );
    EXPECT_TRUE( comma::csv::fieldwise( ",x" ).ascii().equal( "a,b,c", "x,b,x" ) );
    EXPECT_TRUE( comma::csv::fieldwise( ",,x" ).ascii().equal( "a,b,c", "x,x,c" ) );
    EXPECT_FALSE( comma::csv::fieldwise( "x" ).ascii().equal( "a,b,c", "x,x,x" ) );
    EXPECT_FALSE( comma::csv::fieldwise( ",x" ).ascii().equal( "a,b,c", "x,x,x" ) );
    EXPECT_FALSE( comma::csv::fieldwise( ",,x" ).ascii().equal( "a,b,c", "x,x,x" ) );
}

namespace comma { namespace csv { namespace fieldwise_test {
    
struct basic
{
    int x;
    int y;
    basic() : x( 0 ), y( 0 ) {}
};

struct nested
{
    int a;
    basic b;
    nested() : a( 0 ) {}
};

} } } // namespace comma { namespace csv { namespace fieldwise_test {

namespace comma { namespace visiting {

template <> struct traits< comma::csv::fieldwise_test::basic >
{
    template < typename Key, class Visitor > static void visit( const Key&, const comma::csv::fieldwise_test::basic& p, Visitor& v )
    {
        v.apply( "x", p.x );
        v.apply( "y", p.y );
    }

    template < typename Key, class Visitor > static void visit( const Key&, comma::csv::fieldwise_test::basic& p, Visitor& v )
    {
        v.apply( "x", p.x );
        v.apply( "y", p.y );
    }
};

template <> struct traits< comma::csv::fieldwise_test::nested >
{
    template < typename Key, class Visitor > static void visit( const Key&, const comma::csv::fieldwise_test::nested& p, Visitor& v )
    {
        v.apply( "a", p.a );
        v.apply( "b", p.b );
    }

    template < typename Key, class Visitor > static void visit( const Key&, comma::csv::fieldwise_test::nested& p, Visitor& v )
    {
        v.apply( "a", p.a );
        v.apply( "b", p.b );
    }
};

} } // namespace comma { namespace visiting {

TEST( fieldwise, typed_ascii )
{
    comma::csv::fieldwise_test::nested n;
    EXPECT_FALSE( comma::csv::fieldwise( n, "" ).ascii().equal( "a,x,y", "v,v,v" ) );
    EXPECT_TRUE( comma::csv::fieldwise( n, "" ).ascii().equal( "a,x,y", "a,x,y" ) );
    EXPECT_TRUE( comma::csv::fieldwise( n, "a,x,y", ',', false ).ascii().equal( "a,x,y", "a,x,y" ) );
    EXPECT_FALSE( comma::csv::fieldwise( n, "a,x,y", ',', false ).ascii().equal( "a,x,y", "a,x,z" ) );
    EXPECT_TRUE( comma::csv::fieldwise( n, "a,b/x,b/y" ).ascii().equal( "a,x,y", "a,x,y" ) );
    EXPECT_FALSE( comma::csv::fieldwise( n, "a,b/x,b/y" ).ascii().equal( "a,x,y", "a,x,z" ) );
    EXPECT_TRUE( comma::csv::fieldwise( n, "a,b" ).ascii().equal( "a,x,y", "a,x,y" ) );
    EXPECT_FALSE( comma::csv::fieldwise( n, "a,b" ).ascii().equal( "a,x,y", "a,x,z" ) );
    EXPECT_TRUE( comma::csv::fieldwise( n, "b" ).ascii().equal( "a,x,y", "a,x,z" ) );
    EXPECT_FALSE( comma::csv::fieldwise( n, "b" ).ascii().equal( "a,x,y", "a,z,z" ) );
}

// todo: test binary
