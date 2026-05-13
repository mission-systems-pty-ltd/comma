// Copyright (c) 2023 Vsevolod Vlaskine

#if __cplusplus >= 201703L
    #include <variant>
#else
    // ...
#endif
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
        EXPECT_FALSE( v );
        v.set< int >( 5 );
        EXPECT_TRUE( v );
        v.reset();
        EXPECT_FALSE( v );
        v.set< float >( 5 );
        EXPECT_TRUE( v );
        v.reset();
        EXPECT_FALSE( v );
        v.set< double >( 5 );
        EXPECT_TRUE( v );
        v.reset();
        EXPECT_FALSE( v );
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
        variant_t v;
        EXPECT_EQ( v.index(), 3 );
        v.set< int >( 5 );
        EXPECT_EQ( v.index(), 0 );
        v.set< float >( 5 );
        EXPECT_EQ( v.index(), 1 );
        v.set< double >( 5 );
        EXPECT_EQ( v.index(), 2 );
        v.reset();
        EXPECT_EQ( v.index(), 3 );
    }
    #if __cplusplus >= 201703L
        {
            typedef comma::variant< int, float, std::string > variant_t;
            typedef std::variant< std::string, double, int, float > target_t;
            {
                variant_t v;
                v.as< target_t >();
            }
            {
                variant_t v;
                v.set< int >( 5 );
                EXPECT_EQ( std::get< int >( v.as< target_t >() ), 5 );
            }
            {
                variant_t v;
                v.set< float >( 10 );
                EXPECT_EQ( std::get< float >( v.as< target_t >() ), 10 );
            }
            {
                variant_t v;
                v.set< std::string >( "abc" );
                EXPECT_EQ( std::get< std::string >( v.as< target_t >() ), "abc" );
            }
        }
    #endif // #if __cplusplus >= 201703L
    {
        typedef comma::variant< int, float, std::string > variant_t;
        variant_t v;
        EXPECT_TRUE( ( std::is_same< decltype( v.at< 0 >() ), int >::value ) );
        EXPECT_TRUE( ( std::is_same< decltype( v.at< 1 >() ), float >::value ) );
        EXPECT_TRUE( ( std::is_same< decltype( v.at< 2 >() ), std::string >::value ) );
        EXPECT_EQ( v.touch_at( 0 ).index(), 0 );
        EXPECT_EQ( v.touch_at( 1 ).index(), 1 );
        EXPECT_EQ( v.touch_at( 2 ).index(), 2 );
        // EXPECT_TRUE( ( std::is_same< decltype( v.at( 0 ) ), int >::value ) );
        // EXPECT_TRUE( ( std::is_same< decltype( v.at( 1 ) ), float >::value ) );
        // EXPECT_TRUE( ( std::is_same< decltype( v.at( 2 ) ), std::string >::value ) );
    }
}

TEST( base, named_variant )
{
    {
        struct naming { static std::array< std::string, 4 > names() { return { "a", "b", "c", "d" }; } };
        typedef comma::named_variant< naming, int, float, double, std::string > variant_t;
        EXPECT_EQ( variant_t::name_of< int >(), "a" );
        EXPECT_EQ( variant_t::name_of< float >(), "b" );
        EXPECT_EQ( variant_t::name_of< double >(), "c" );
        EXPECT_EQ( variant_t::name_of< std::string >(), "d" );
        EXPECT_EQ( variant_t::index_of( "a" ), 0 );
        EXPECT_EQ( variant_t::index_of( "b" ), 1 );
        EXPECT_EQ( variant_t::index_of( "c" ), 2 );
        EXPECT_EQ( variant_t::index_of( "d" ), 3 );
        variant_t v;
        EXPECT_TRUE( v.touch_at( 0 ).is< int >() );
        EXPECT_TRUE( v.touch_at( 1 ).is< float >() );
        EXPECT_TRUE( v.touch_at( 2 ).is< double >() );
        EXPECT_TRUE( v.touch_at( 3 ).is< std::string >() );
        EXPECT_TRUE( v.touch_at( "a" ).is< int >() );
        EXPECT_TRUE( v.touch_at( "b" ).is< float >() );
        EXPECT_TRUE( v.touch_at( "c" ).is< double >() );
        EXPECT_TRUE( v.touch_at( "d" ).is< std::string >() );
    }
    {
        struct naming { static std::array< std::string, 3 > names() { return { "a", "b", "c" }; } };
        comma::named_variant< naming, int, float, double > v;
        EXPECT_FALSE( v );
        v.set< int >( 5 );
        EXPECT_TRUE( v );
        EXPECT_EQ( v.name(), "a" );
        v.reset();
        EXPECT_FALSE( v );
        v.set< float >( 5 );
        EXPECT_TRUE( v );
        EXPECT_EQ( v.name(), "b" );
        v.reset();
        EXPECT_FALSE( v );
        v.set< double >( 5 );
        EXPECT_TRUE( v );
        EXPECT_EQ( v.name(), "c" );
        v.reset();
        EXPECT_FALSE( v );
        EXPECT_THROW( v.name(), comma::exception );
    }
}

} // namespace comma {

int main( int argc, char* argv[] )
{    
    ::testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
