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
}

// todo

namespace comma { namespace csv { namespace fieldwise_test {
    
struct test_struct
{
    int x;
    int y;
    test_struct() : x( 0 ), y( 0 ) {}
};

} } } // namespace comma { namespace csv { namespace ascii_test {

namespace comma { namespace visiting {

template <> struct traits< comma::csv::fieldwise_test::test_struct >
{
    template < typename Key, class Visitor > static void visit( const Key&, const comma::csv::fieldwise_test::test_struct& p, Visitor& v )
    {
        v.apply( "x", p.x );
        v.apply( "y", p.y );
    }

    template < typename Key, class Visitor > static void visit( const Key&, comma::csv::fieldwise_test::test_struct& p, Visitor& v )
    {
        v.apply( "x", p.x );
        v.apply( "y", p.y );
    }
};

// todo

} } // namespace comma { namespace visiting {

   