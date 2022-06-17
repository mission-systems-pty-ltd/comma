// Copyright (c) 2011 The University of Sydney

#include <gtest/gtest.h>
#include "../../name_value/parser.h"
#include "../../name_value/serialize.h"

struct nested
{
    int b;
    double c;
};

struct hello
{
    std::string filename;
    int a;
    nested n;
    std::string s;
};

struct nested_with_optional
{
    int c;
    boost::optional< int > d;
    nested_with_optional() : c( 0 ) {}
};

struct struct_with_optional
{
    int a;
    boost::optional< int > b;
    boost::optional< nested_with_optional > nested;
    struct_with_optional() : a( 0 ) {}
};

static std::string json_remove_quotes_wrapper( std::string json ) { comma::name_value::impl::json_remove_quotes( json ); return json; }

namespace comma { namespace visiting {

template <> struct traits< hello >
{
    template < typename Key, class Visitor >
    static void visit( Key, hello& hello, Visitor& v )
    {
        v.apply( "filename", hello.filename );
        v.apply( "a", hello.a );
        v.apply( "nested", hello.n );
        v.apply( "s", hello.s );
    }

    template < typename Key, class Visitor >
    static void visit( Key, const hello& hello, Visitor& v )
    {
        v.apply( "filename", hello.filename );
        v.apply( "a", hello.a );
        v.apply( "nested", hello.n );
        v.apply( "s", hello.s );
    }
};

template <> struct traits< nested >
{
    template < typename Key, class Visitor >
    static void visit( Key, nested& nested, Visitor& v )
    {
        v.apply( "b", nested.b );
        v.apply( "c", nested.c );
    }

    template < typename Key, class Visitor >
    static void visit( Key, const nested& nested, Visitor& v )
    {
        v.apply( "b", nested.b );
        v.apply( "c", nested.c );
    }
};

template <> struct traits< nested_with_optional >
{
    template < typename Key, class Visitor > static void visit( Key, const nested_with_optional& t, Visitor& v )
    {
        v.apply( "c", t.c );
        v.apply( "d", t.d );
    }

    template < typename Key, class Visitor > static void visit( Key, nested_with_optional& t, Visitor& v )
    {
        v.apply( "c", t.c );
        v.apply( "d", t.d );
    }
};

template <> struct traits< struct_with_optional >
{
    template < typename Key, class Visitor > static void visit( Key, const struct_with_optional& t, Visitor& v )
    {
        v.apply( "a", t.a );
        v.apply( "b", t.b );
        v.apply( "nested", t.nested );
    }

    template < typename Key, class Visitor > static void visit( Key, struct_with_optional& t, Visitor& v )
    {
        v.apply( "a", t.a );
        v.apply( "b", t.b );
        v.apply( "nested", t.nested );
    }
};

} } // namespace comma { namespace visiting {


struct config
{
    config() : size(0), alpha(0), beta(0) {}
    config( const std::string& name, int s, double a, double b ):
        filename( name ), size( s ), alpha( a ), beta( b ) {}
    std::string filename;
    int size;
    double alpha;
    double beta;
};

namespace comma { namespace visiting {

template <> struct traits< config >
{
    template < typename Key, class Visitor >
    static void visit( Key, config& config, Visitor& v )
    {
        v.apply( "filename", config.filename );
        v.apply( "size", config.size );
        v.apply( "alpha", config.alpha );
        v.apply( "beta", config.beta );
    }

    template < typename Key, class Visitor >
    static void visit( Key, const config& config, Visitor& v )
    {
        v.apply( "filename", config.filename );
        v.apply( "size", config.size );
        v.apply( "alpha", config.alpha );
        v.apply( "beta", config.beta );
    }
};

} } // namespace comma { namespace visiting {


namespace comma { namespace name_value { namespace test {

TEST( name_value, get )
{
    parser named_values("filename,,,,,s");
    hello h = named_values.get<hello>("test.csv;a=2;nested/b=3;nested/c=0.2;unNamed2;world");
    hello expected;
    expected.filename = "test.csv";
    expected.a = 2;
    expected.n.b = 3;
    expected.n.c = 0.2;
    expected.s = "world";
    parser named_values2("filename",'-', ':', false);
    h = named_values2.get("test.csv-a:2-b:3-c:0.2-s:world", hello());
    EXPECT_THROW( h = named_values.get<hello>("a=b=c;a=2;nested/b=3;nested/c=0.2;unNamed2;s=world"), comma::exception );
    EXPECT_THROW( h = named_values.get<hello>("file=test.csv;a=2;nested/b=3;nested/c=0.2;unNamed2;s=world"), comma::exception );
}

TEST( name_value, put )
{
    parser named_values;
    std::string string = "a=2;nested/b=3;nested/c=0.2;s=world";
    hello h = named_values.get<hello>( string );

    string = "filename=;" + string;
    EXPECT_EQ( named_values.put(h), string );

    parser named_values2('-',':',false);
    string = "a:2-b:3-c:0.2-s:world";
    h = named_values2.get<hello>( string );

    string = "filename:-" + string;
    EXPECT_EQ( named_values2.put(h), string );
}

TEST( name_value, map )
{
    {
        map m( "" );
        EXPECT_TRUE( !m.exists( "a" ) );
    }
    {
        map m( "a=x;b=y;c/d=z" );
        EXPECT_TRUE( m.exists( "a" ) );
        EXPECT_TRUE( m.exists( "b" ) );
        EXPECT_TRUE( m.exists( "c/d" ) );
        EXPECT_TRUE( !m.exists( "r" ) );
        EXPECT_EQ( m.value< std::string >( "a" ), "x" );
        EXPECT_EQ( m.value< std::string >( "b" ), "y" );
        EXPECT_EQ( m.value< std::string >( "c/d" ), "z" );
    }
    {
        map m( "b;c=false;d=0" );
        EXPECT_TRUE( m.exists( "b" ) );
        EXPECT_TRUE( m.exists( "c" ) );
        EXPECT_TRUE( m.exists( "d" ) );
        EXPECT_EQ( m.value< std::string >( "b" ), "" );
        EXPECT_EQ( m.value< bool >( "b" ), true );
        EXPECT_EQ( m.value< bool >( "c" ), false );
        EXPECT_EQ( m.value< bool >( "d" ), false );
    }
    {
        EXPECT_EQ( map( ";a=1" ).value< int >( "a" ), 1 );
    }
}

TEST( name_value, map_escaped )
{
    {
        map m( "" );
        EXPECT_TRUE( !m.exists( "a" ) );
    }
    {
        map m( "b='y';c/d=\\z;e=\\;;a='x;'" );
        EXPECT_TRUE( m.exists( "a" ) );
        EXPECT_TRUE( m.exists( "b" ) );
        EXPECT_TRUE( m.exists( "c/d" ) );
        EXPECT_TRUE( m.exists( "e" ) );
        EXPECT_TRUE( !m.exists( "r" ) );
        EXPECT_EQ( m.value< std::string >( "a" ), "x;" );
        EXPECT_EQ( m.value< std::string >( "b" ), "y" );
        EXPECT_EQ( m.value< std::string >( "c/d" ), "\\z" );
        EXPECT_EQ( m.value< std::string >( "e" ), ";" );
    }
}

TEST( name_value, exists )
{
    {
        EXPECT_TRUE( map( "", "filename" ).exists( "filename" ) );
        EXPECT_EQ( map( "", "filename" ).value< std::string >( "filename" ), "" );
        EXPECT_TRUE( map( "blah", "filename" ).exists( "filename" ) );
        EXPECT_EQ( map( "blah", "filename" ).value< std::string >( "filename" ), "blah" );
        EXPECT_TRUE( map( "blah;hello=world", "filename" ).exists( "filename" ) );
        EXPECT_EQ( map( "blah;hello=world", "filename" ).value< std::string >( "filename" ), "blah" );
        EXPECT_TRUE( map( ";hello=world", "filename" ).exists( "filename" ) );
        EXPECT_THROW( map( "hello=world", "filename" ).exists( "filename" ), comma::exception );
    }
    {
        EXPECT_TRUE( !map( "", ",filename" ).exists( "filename" ) );
        EXPECT_TRUE( map( ";blah", ",filename" ).exists( "filename" ) );
        EXPECT_THROW( map( "blah;hello=world", ",filename" ).exists( "filename" ), comma::exception );
        EXPECT_THROW( map( ";hello=world", ",filename" ).exists( "filename" ), comma::exception );
        EXPECT_TRUE( !map( "hello=world", ",filename" ).exists( "filename" ) );
    }
    {
        parser named_values( ",a/b/c" );
        EXPECT_TRUE( !map( "", ",a/b/c" ).exists( "a/b/c" ) );
        EXPECT_TRUE( map( ";blah", ",a/b/c" ).exists( "a/b/c" ) );
        EXPECT_THROW( map( "blah;hello=world", ",a/b/c" ).exists( "a/b/c" ), comma::exception );
        EXPECT_THROW( map( ";hello=world", ",a/b/c" ).exists( "a/b/c" ), comma::exception );
        EXPECT_TRUE( !map( "hello=world", ",a/b/c" ).exists( "a/b/c" ) );
    }
    {
        EXPECT_TRUE( !map( "" ).exists( "c" ) );
        EXPECT_TRUE( map( ";c=x" ).exists( "c" ) );
        EXPECT_TRUE( map( "blah;c=x" ).exists( "c" ) );
        EXPECT_TRUE( map( ";c=x" ).exists( "c" ) );
        EXPECT_TRUE( map( "c" ).exists( "c" ) );
    }
}

TEST( name_value, serialize_json )
{
    EXPECT_EQ( "{ \"a\": \"a\", \"b\": \"b\\\\\" }", json_remove_quotes_wrapper( "{ \"a\": \"a\", \"b\": \"b\\\\\" }" ) );
    EXPECT_EQ( "{ \"a\": \"a\", \"b\": \"b\\\"b\" }", json_remove_quotes_wrapper( "{ \"a\": \"a\", \"b\": \"b\\\"b\" }" ) );
    EXPECT_EQ( "{ \"a\": \"a\", \"b\": \"b\\\\c\" }", json_remove_quotes_wrapper( "{ \"a\": \"a\", \"b\": \"b\\\\c\" }" ) );
    EXPECT_EQ( "{ \"a\": \"a\", \"b\": \"b\\\\\\\"b\" }", json_remove_quotes_wrapper( "{ \"a\": \"a\", \"b\": \"b\\\\\\\"b\" }" ) );
    EXPECT_EQ( "{ \"a\": 1, \"b\": \"val\" }", json_remove_quotes_wrapper( "{ \"a\": \"1\", \"b\": \"val\" }" ) );
    EXPECT_EQ( "{ \"a\": [ 4.44, true, \"e\" ] }", json_remove_quotes_wrapper( "{ \"a\": [ \"4.44\", \"true\", \"e\" ] }" ) );

    EXPECT_EQ( "{ \"a\": [ { \"b\": { \"c\": [ { \"d\": 1, \"e\": false }, { \"d\":2, \"e\": true } ] } } ], \"p\": { \"q\": { \"r\": 3.9e8, \"s\": \"t\"} }, \"x\": { \"y\": [ \"z\", 0.1e-3 ] } }", json_remove_quotes_wrapper( "{ \"a\": [ { \"b\": { \"c\": [ { \"d\": \"1\", \"e\": \"false\" }, { \"d\":2, \"e\": \"true\" } ] } } ], \"p\": { \"q\": { \"r\": \"3.9e8\", \"s\": \"t\"} }, \"x\": { \"y\": [ \"z\", \"0.1e-3\" ] } }" ) );

    //minify
    EXPECT_EQ( "{\"a\":[{\"b\":{\"c\":[{\"d\":1,\"e\":false},{\"d\":2,\"e\":true}]}}],\"p\":{\"q\":{\"r\":3.9e8,\"s\":\"t\"}},\"x\":{\"y\":[\"z\",0.1e-3]}}", json_remove_quotes_wrapper( "{\"a\":[{\"b\":{\"c\":[{\"d\":\"1\",\"e\":\"false\"},{\"d\":\"2\",\"e\":\"true\"}]}}],\"p\":{\"q\":{\"r\":\"3.9e8\",\"s\":\"t\"}},\"x\":{\"y\":[\"z\",\"0.1e-3\"]}}" ) );

}

TEST( name_value, optional )
{
    {
        struct_with_optional s = name_value::parser().get< struct_with_optional >( "a=1;b=2;nested/c=3;nested/d=4" );
        EXPECT_TRUE( bool( s.b ) );
        EXPECT_TRUE( bool( s.nested ) );
        EXPECT_TRUE( bool( s.nested->d ) );
        EXPECT_EQ( s.a, 1 );
        EXPECT_EQ( *s.b, 2 );
        EXPECT_EQ( s.nested->c, 3 );
        EXPECT_EQ( *s.nested->d, 4 );
    }
    {
        struct_with_optional s = name_value::parser().get< struct_with_optional >( "a=1;nested/c=3" );
        EXPECT_TRUE( !s.b );
        EXPECT_TRUE( bool( s.nested ) );
        EXPECT_TRUE( !s.nested->d );
        EXPECT_EQ( s.a, 1 );
        EXPECT_EQ( s.nested->c, 3 );
    }
    {
        struct_with_optional s = name_value::parser( ';', '=', false ).get< struct_with_optional >( "a=1;c=3" );
        EXPECT_TRUE( !s.b );
        EXPECT_TRUE( bool( s.nested ) );
        EXPECT_TRUE( !s.nested->d );
        EXPECT_EQ( s.a, 1 );
        EXPECT_EQ( s.nested->c, 3 );
    }
    {
        EXPECT_EQ( name_value::parser().put( name_value::parser().get< struct_with_optional >( "a=1;b=2;nested/c=3;nested/d=4" ) ), "a=1;b=2;nested/c=3;nested/d=4" );
        EXPECT_EQ( name_value::parser().put( name_value::parser().get< struct_with_optional >( "a=1;nested/c=3" ) ), "a=1;nested/c=3" );
        EXPECT_EQ( name_value::parser().put( name_value::parser().get< struct_with_optional >( "a=1" ) ), "a=1" );
        // todo: more tests
    }
}

TEST( name_value, as_vector )
{
    {
        const auto& v = name_value::map::as_vector( "a=1;b;c=2;nested/d=blah" );
        EXPECT_EQ( 4, v.size() );
        EXPECT_EQ( "a", v[0].first );
        EXPECT_EQ( "1", v[0].second );
        EXPECT_EQ( "b", v[1].first );
        EXPECT_EQ( "", v[1].second );
        EXPECT_EQ( "c", v[2].first );
        EXPECT_EQ( "2", v[2].second );
        EXPECT_EQ( "nested/d", v[3].first );
        EXPECT_EQ( "blah", v[3].second );
    }
}

TEST( name_value, unique )
{
    void( name_value::map( "a=1;b;c=2", ';', '=', true ) );
    EXPECT_THROW( name_value::map( "a=1;b;a=2;c=2", ';', '=', true ), comma::exception );
    EXPECT_THROW( name_value::map( "a=1;b;b;c=2", ';', '=', true ), comma::exception );
}

TEST( name_value, allowed_names )
{
    void( name_value::map( "a=1;b;c=2", ';', '=', false, "a,b,c" ) );
    EXPECT_THROW( name_value::map( "a=1;b;x;a=2;c=2", ';', '=', false, "a,b,c" ), comma::exception );
    EXPECT_THROW( name_value::map( "a=1;b;x=5;b;c=2", ';', '=', false, "a,b,c" ), comma::exception );
}

} } }

int main( int argc, char* argv[] )
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
