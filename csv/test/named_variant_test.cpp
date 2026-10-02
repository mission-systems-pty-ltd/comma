// This file is part of comma, a generic and flexible library
// Copyright (c) 2026 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#include <gtest/gtest.h>
#include <iostream>
#include "../../csv/named_variant/binary.h"
#include "../../csv/named_variant/ascii.h"
#include "../../csv/named_variant/stream.h"
#include "../../csv/names.h"
#include "../../string/string.h"
#include "../../visiting/traits.h"

namespace comma { namespace csv { namespace test {

struct a { int x{0}; };
struct b { int y{0}; };
struct n { int k{0}; int m{0}; };
struct c { n z; };

struct naming { static std::array< std::string, 4 > names() { return { "a", "b", "c", "d" }; } };

typedef comma::named_variant< naming, a, b, c > variant_t;

} } } // namespace comma { namespace csv { namespace test {

namespace comma { namespace visiting {

template <> struct traits< comma::csv::test::a > { template < typename Key, class Visitor > static void visit( const Key& k, const comma::csv::test::a& p, Visitor& v ) { v.apply( "x", p.x ); } };
template <> struct traits< comma::csv::test::b > { template < typename Key, class Visitor > static void visit( const Key& k, const comma::csv::test::b& p, Visitor& v ) { v.apply( "y", p.y ); } };
template <> struct traits< comma::csv::test::n > { template < typename Key, class Visitor > static void visit( const Key& k, const comma::csv::test::n& p, Visitor& v ) { v.apply( "k", p.k ); v.apply( "m", p.m ); } };
template <> struct traits< comma::csv::test::c > { template < typename Key, class Visitor > static void visit( const Key& k, const comma::csv::test::c& p, Visitor& v ) { v.apply( "z", p.z ); } };

} } // namespace comma { namespace visiting {

namespace comma { namespace csv { namespace test {

TEST( csv_named_variant, names )
{
    EXPECT_EQ( comma::join( comma::csv::names< variant_t >( variant_t{}.touch_at( "a" ) ), ',' ), "a/x" );
    EXPECT_EQ( comma::join( comma::csv::names< variant_t >( variant_t{}.touch_at( "b" ) ), ',' ), "b/y" );
    EXPECT_EQ( comma::join( comma::csv::names< variant_t >( variant_t{}.touch_at( "c" ) ), ',' ), "c/z/k,c/z/m" );
}

} } } // namespace comma { namespace csv { namespace test {