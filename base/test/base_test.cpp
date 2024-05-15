// Copyright (c) 2023 Vsevolod Vlaskine

#include "../exception.h"
#include "../variant.h"
#include <gtest/gtest.h>

namespace comma {

TEST( base, exception )
{
    COMMA_ASSERT( true, "all good" );
    COMMA_ASSERT( 2 * 2 == 4, "all good" );
    EXPECT_THROW( COMMA_ASSERT( false, "all bad" ), comma::exception );
    EXPECT_THROW( COMMA_ASSERT( 2 * 2 == 5, "all bad" ), comma::exception );
    COMMA_THROW_IF( false, "all good" );
    COMMA_THROW_IF( 2 * 2 == 5, "all good" );
    EXPECT_THROW( COMMA_THROW_IF( true, "all bad" ), comma::exception );
    EXPECT_THROW( COMMA_THROW_IF( 2 * 2 == 4, "all bad" ), comma::exception );
}

TEST( base, variant )
{
    {
        comma::impl::variant< int, float, double > v;
        v.t = 1;
        v.values.t = 2;
        v.values.values.t = 3;
    }
    {
        comma::impl::variant< int, float, double > v;
        EXPECT_FALSE( v.is< int >() );
        EXPECT_FALSE( v.is< float >() );
        EXPECT_FALSE( v.is< double >() );
        v.set< int >( 5 );
        EXPECT_EQ( *v.t, 5 );
        EXPECT_TRUE( v.is< int >() );
        EXPECT_FALSE( v.is< float >() );
        EXPECT_FALSE( v.is< double >() );
        v.set< float >( 5 );
        EXPECT_EQ( *v.values.t, 5 );
        EXPECT_FALSE( v.is< int >() );
        EXPECT_TRUE( v.is< float >() );
        EXPECT_FALSE( v.is< double >() );
        v.set< double >( 5 );
        EXPECT_EQ( *v.values.values.t, 5 );
        EXPECT_FALSE( v.is< int >() );
        EXPECT_FALSE( v.is< float >() );
        EXPECT_TRUE( v.is< double >() );
        v.set< int >( 5 );
        EXPECT_EQ( *v.t, 5 );
        EXPECT_TRUE( v.is< int >() );
        EXPECT_FALSE( v.is< float >() );
        EXPECT_FALSE( v.is< double >() );
    }
    {
        comma::variant< int, float, double > v;
        EXPECT_FALSE( v.is< int >() );
        EXPECT_FALSE( v.is< float >() );
        EXPECT_FALSE( v.is< double >() );
        v.set< int >( 5 );
        EXPECT_EQ( v.get< int >(), 5 );
        EXPECT_TRUE( v.optional< int >() );
        EXPECT_FALSE( v.optional< float >() );
        EXPECT_FALSE( v.optional< double >() );
        EXPECT_EQ( *v.optional< int >(), 5 );
        EXPECT_TRUE( v.is< int >() );
        EXPECT_FALSE( v.is< float >() );
        EXPECT_FALSE( v.is< double >() );
        v.set< float >( 5 );
        EXPECT_EQ( v.get< float >(), 5 );
        EXPECT_FALSE( v.optional< int >() );
        EXPECT_TRUE( v.optional< float >() );
        EXPECT_FALSE( v.optional< double >() );
        EXPECT_EQ( *v.optional< float >(), 5 );
        EXPECT_FALSE( v.is< int >() );
        EXPECT_TRUE( v.is< float >() );
        EXPECT_FALSE( v.is< double >() );
        v.set< double >( 5 );
        EXPECT_EQ( v.get< double >(), 5 );
        EXPECT_FALSE( v.optional< int >() );
        EXPECT_FALSE( v.optional< float >() );
        EXPECT_TRUE( v.optional< double >() );
        EXPECT_EQ( *v.optional< double >(), 5 );
        EXPECT_FALSE( v.is< int >() );
        EXPECT_FALSE( v.is< float >() );
        EXPECT_TRUE( v.is< double >() );
        v.set< int >( 5 );
        EXPECT_EQ( v.get< int >(), 5 );
        EXPECT_TRUE( v.optional< int >() );
        EXPECT_FALSE( v.optional< float >() );
        EXPECT_FALSE( v.optional< double >() );
        EXPECT_EQ( *v.optional< float >(), 5 );
        EXPECT_EQ( *v.optional< int >(), 5 );
        EXPECT_TRUE( v.is< int >() );
        EXPECT_FALSE( v.is< float >() );
        EXPECT_FALSE( v.is< double >() );
    }
    {
        { auto size = comma::variant< int >::size; EXPECT_EQ( size, 1 ); }
        { auto size = comma::variant< int, float >::size; EXPECT_EQ( size, 2 ); }
        { auto size = comma::variant< int, float, double >::size; EXPECT_EQ( size, 3 ); }
    }
    {
        typedef comma::variant< int, float, double > variant_t;
        EXPECT_EQ( variant_t::index_of< int >(), 0 );
        EXPECT_EQ( variant_t::index_of< float >(), 1 );
        EXPECT_EQ( variant_t::index_of< double >(), 2 );
    }
}

TEST( base, named_variant )
{
    {
        struct naming { static std::array< std::string, 3 > names() { return { "a", "b", "c" }; } };
        typedef comma::named_variant< naming, int, float, double > variant_t;
        variant_t v;
        EXPECT_EQ( variant_t::name_of< int >(), "a" );
        EXPECT_EQ( variant_t::name_of< float >(), "b" );
        EXPECT_EQ( variant_t::name_of< double >(), "c" );
    }
}

} // namespace comma {

int main( int argc, char* argv[] )
{    
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
