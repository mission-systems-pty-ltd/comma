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
#include <comma/name_value/serialize.h>
#include <comma/visiting/traits.h>
#include <comma/xpath/xpath.h>

struct section_one
{
    section_one() : size( 0 ) {}
    section_one( int size_ ) : size( size_ ) {}
    int size;
};

struct section_two
{
    section_two() : size( 0 ), alpha( 0 ), beta( 0 ) {}
    section_two( int size_, double alpha_, double beta_ ) : size( size_ ), alpha( alpha_ ), beta( beta_ ) {}
    int size;
    double alpha;
    double beta;
};

struct ini
{
    ini() : size( 0 ) {}
    ini( int size_, section_one one_, section_two two_ ) : size( size_ ), one( one_ ), two( two_ ) {}
    int size;
    section_one one;
    section_two two;
};


namespace comma { namespace visiting {

template <> struct traits< section_one >
{
    template < typename Key, class Visitor >
    static void visit( Key, section_one& one, Visitor& v )
    {
        v.apply( "size", one.size );
    }

    template < typename Key, class Visitor >
    static void visit( Key, const section_one& one, Visitor& v )
    {
        v.apply( "size", one.size );
    }
};

template <> struct traits< section_two >
{
    template < typename Key, class Visitor >
    static void visit( Key, section_two& two, Visitor& v )
    {
        v.apply( "size", two.size );
        v.apply( "alpha", two.alpha );
        v.apply( "beta", two.beta );
    }

    template < typename Key, class Visitor >
    static void visit( Key, const section_two& two, Visitor& v )
    {
        v.apply( "size", two.size );
        v.apply( "alpha", two.alpha );
        v.apply( "beta", two.beta );
    }
};

template <> struct traits< ini >
{
    template < typename Key, class Visitor >
    static void visit( Key, ini& ini, Visitor& v )
    {
        v.apply( "size", ini.size );
        v.apply( "one", ini.one );
        v.apply( "two", ini.two );
    }

    template < typename Key, class Visitor >
    static void visit( Key, const ini& ini, Visitor& v )
    {
        v.apply( "size", ini.size );
        v.apply( "one", ini.one );
        v.apply( "two", ini.two );
    }
};

} } // namespace comma  { namespace visiting {

namespace comma { namespace test { namespace serialise {

TEST( serialise_ini, simple )
{
    std::string ini =
    "alpha=0.5\n"
    "beta=0.6\n"
    "size=10";
    std::istringstream iss( ini );
    section_two two;
    comma::read_ini< section_two >( two, iss );
    EXPECT_EQ( 10, two.size );
    EXPECT_DOUBLE_EQ( 0.5, two.alpha );
    EXPECT_DOUBLE_EQ( 0.6, two.beta );
}

TEST( serialise_ini, sections )
{
    std::string s =
    "size=-1\n"
    "[one]\n"
    "    size=2\n"
    "; comment\n"
    "[two]\n"
    "    size=10\n"
    "    alpha=1.5\n"
    "    beta=1.6";
    std::istringstream iss( s );
    ini c = comma::read_ini< ini >( iss );
    EXPECT_EQ( -1, c.size );
    EXPECT_EQ( 2, c.one.size );
    EXPECT_EQ( 10, c.two.size );
    EXPECT_DOUBLE_EQ( 1.5, c.two.alpha );
    EXPECT_DOUBLE_EQ( 1.6, c.two.beta );
}

TEST( serialise_ini, defaults )
{
    std::string s =
    "size=-1\n"
    "[two]\n"
    "    size=10\n"
    "    beta=1.6";
    std::istringstream iss( s );
    ini c = comma::read_ini< ini >( iss );
    EXPECT_EQ( -1, c.size );
    EXPECT_EQ( 0, c.one.size );
    EXPECT_EQ( 10, c.two.size );
    EXPECT_DOUBLE_EQ( 0, c.two.alpha );
    EXPECT_DOUBLE_EQ( 1.6, c.two.beta );
}

TEST( serialise_ini, add_section )
{
    std::stringstream ss;
    section_two two( -10, -0.1, -0.2 );
    comma::write_ini< section_two >( two, ss, "two" );
    section_two t = comma::read_ini< section_two >( ss, "two" );
    EXPECT_EQ( -10, t.size );
    EXPECT_DOUBLE_EQ( -0.1, t.alpha );
    EXPECT_DOUBLE_EQ( -0.2, t.beta );
}

TEST( serialise_ini, throw_on_nested_sections )
{
    {
        std::stringstream ss;
        ini d( 15, section_one( -5 ), section_two( -10, -0.1, -0.2 ) );
        ASSERT_THROW( comma::write_ini< ini >( d, ss, "root" ), boost::property_tree::ptree_error );
    }
    {
        std::stringstream ss;
        section_two two( -10, -0.1, -0.2 );
        ASSERT_THROW( comma::write_ini< section_two >( two, ss, "root/item" ), boost::property_tree::ptree_error );
    }
}


static const section_two sample_section_two( -10, -0.1, -0.2 );
static const ini sample_ini( 15, section_one( -5 ), sample_section_two );

void test_section_two( const section_two& t )
{
    EXPECT_EQ( -10, t.size );
    EXPECT_DOUBLE_EQ( -0.1, t.alpha );
    EXPECT_DOUBLE_EQ( -0.2, t.beta );
}

void test_ini( const ini& i )
{
    EXPECT_EQ( 15, i.size );
    EXPECT_EQ( -5, i.one.size );
    test_section_two( i.two );
}

TEST( serialise, ini )
{
    {
        std::stringstream ss;
        comma::write_ini< ini >( sample_ini, ss );
        ini i = comma::read_ini< ini >( ss );
        test_ini( i );
    }
    {
        std::stringstream ss;
        comma::write_ini< ini >( sample_ini, ss );
        ini i;
        comma::read_ini< ini >( i, ss );
        test_ini( i );
    }
    {
        std::stringstream ss;
        comma::write_ini< ini >( sample_ini, ss );
        ini i = comma::read_ini< ini >( ss, true );
        test_ini( i );
    }
    {
        std::stringstream ss;
        comma::write_ini< ini >( sample_ini, ss );
        ini i;
        comma::read_ini< ini >( i, ss, true );
        test_ini( i );
    }

    {
        std::stringstream ss;
        comma::write_ini< section_two >( sample_section_two, ss, "two" );
        section_two two = comma::read_ini< section_two >( ss, "two" );
        test_section_two( two );
    }
    {
        std::stringstream ss;
        comma::write_ini< section_two >( sample_section_two, ss, comma::xpath( "two" ) );
        section_two two = comma::read_ini< section_two >( ss, comma::xpath( "two" ) );
        test_section_two( two );
    }
    {
        std::stringstream ss;
        comma::write_ini< section_two >( sample_section_two, ss, "two" );
        section_two two = comma::read_ini< section_two >( ss, "two", true );
        test_section_two( two );
    }
    {
        std::stringstream ss;
        comma::write_ini< section_two >( sample_section_two, ss, comma::xpath( "two" ) );
        section_two two = comma::read_ini< section_two >( ss, comma::xpath( "two" ), true );
        test_section_two( two );
    }

    {
        std::stringstream ss;
        comma::write_ini< section_two >( sample_section_two, ss, "two" );
        section_two two;
        comma::read_ini< section_two >( two, ss, "two" );
        test_section_two( two );
    }
    {
        std::stringstream ss;
        comma::write_ini< section_two >( sample_section_two, ss, comma::xpath( "two" ) );
        section_two two;
        comma::read_ini< section_two >( two, ss, comma::xpath( "two" ) );
        test_section_two( two );
    }
    {
        std::stringstream ss;
        comma::write_ini< section_two >( sample_section_two, ss, "two" );
        section_two two;
        comma::read_ini< section_two >( two, ss, "two", true );
        test_section_two( two );
    }
    {
        std::stringstream ss;
        comma::write_ini< section_two >( sample_section_two, ss, comma::xpath( "two" ) );
        section_two two;
        comma::read_ini< section_two >( two, ss, comma::xpath( "two" ), true );
        test_section_two( two );
    }
}

} } } // namespace comma { namespace test { namespace serialise {
