// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2022 Vsevolod Vlaskine

#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <array>
#include <map>
#include <set>
#include <vector>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include "../../base/optional.h"
#include "../../base/types.h"
#include "../apply.h"
#include "../visit.h"

namespace comma { namespace visiting { namespace test {

/// visitor that outputs a struct in a text format
class o_stream_visitor : public boost::noncopyable
{
    public:
        /// constructor
        o_stream_visitor( std::ostream& s ) : m_stream( s ) {}
    
        /// apply to the "leaf" types
        template < typename Key >
        void apply( const Key& name, const bool& value ) { m_stream << "bool:" << name << "=" << ( value ? "true" : "false" ) << " "; }
        template < typename Key >
        void apply( const Key& name, const int& value ) { m_stream << "int:" << name << "=" << value << " "; }
        template < typename Key >
        void apply( const Key& name, const float& value ) { m_stream << "float:" << name << "=" << value << " "; }
        template < typename Key >
        void apply( const Key& name, const double& value ) { m_stream << "double:" << name << "=" << value << " "; }
        template < typename Key >
        void apply( const Key& name, const std::string& value ) { m_stream << "string:" << name << "=\"" << value << "\" "; }
    
        /// apply to a structure: traverse it depth-first
        template < typename K, typename T >
        void apply( const K& name, const T& value )
        {
            if( !empty( name ) ) { m_stream << "object:" << name << "={ "; }
            else { m_stream << "{ "; }
            comma::visiting::visit( name, value, *this );
            m_stream << "}" << ( empty( name ) ? "" : " " );
        }
    
    private:
        std::ostream& m_stream;
        bool empty( const char* name ) { return *name == 0; }
        bool empty( std::size_t ) { return false; }
};

/// visitor that multiplies all the structure members by a number;
/// for simplicity of demo, assumes that the structure has only int "leaf" fields 
class multiply : public boost::noncopyable
{
    public:
        /// constructor
        multiply( int v ) : m_value( v ) {}
    
        /// multiply an int "leaf" field 
        void apply( const char*, int& value ) { value *= m_value; }
    
        /// traverse depth first
        template < typename T >
        void apply( const char* name, T& value ) { comma::visiting::visit( name, value, *this ); }
    
    private:
        int m_value;
};

struct old_plain
{
    struct nested
    {
        int a;
        double b;
        std::string c;
    };
    float blah;
    int hello;
    nested world;
};

struct optionals
{
    comma::optional< int > a;
    comma::optional< std::pair< int, int > > b;
};

} } } // namespace comma { namespace visiting { namespace test {

namespace comma { namespace visiting {

/// traits specialization for old_plain
template <> struct traits< test::old_plain >
{
    template < typename Key, typename visitor >
    static void visit( const Key&, const test::old_plain& p, visitor& v )
    {
        v.apply( "blah", p.blah );
        v.apply( "hello", p.hello );
        v.apply( "world", p.world );
    }
};

/// traits specialization for old_plain::nested
template <> struct traits< test::old_plain::nested >
{
    template < typename Key, typename visitor >
    static void visit( const Key&, const test::old_plain::nested& p, visitor& v )
    {
        v.apply( "a", p.a );
        v.apply( "b", p.b );
        v.apply( "c", p.c );
    }
};

template <> struct traits< test::optionals >
{
    template < typename Key, typename visitor >
    static void visit( const Key&, const test::optionals& p, visitor& v )
    {
        v.apply( "a", p.a );
        v.apply( "b", p.b );
    }
};

} } // namespace comma { namespace visiting {

namespace comma { namespace visiting { namespace test {

TEST( visiting, nested )
{
    {
        old_plain p;
        p.blah = 0;
        p.hello = 1;
        p.world.a = 2;
        p.world.b = 3;
        p.world.c = "HELLO";
        std::ostringstream oss;
        o_stream_visitor v( oss );
        visiting::apply( v, p );
        EXPECT_EQ( oss.str(), "{ float:blah=0 int:hello=1 object:world={ int:a=2 double:b=3 string:c=\"HELLO\" } }" );
    }
}

struct containers
{
    std::pair< std::string, int > pair;
    std::vector< std::string > vector;
    std::set< int > set;
    std::map< std::string, double > map;
    std::array< comma::int32, 3 > array;
    boost::array< comma::int32, 3 > boost_array;
};

} } } // namespace comma { namespace visiting { namespace test {

namespace comma { namespace visiting {

template <>
struct traits< test::containers >
{
    template < typename Key, typename visitor >
    static void visit( const Key&, const test::containers& p, visitor& v )
    {
        v.apply( "pair", p.pair );
        v.apply( "vector", p.vector );
        v.apply( "set", p.set );
        v.apply( "map", p.map );
        v.apply( "array", p.array );
        v.apply( "boost_array", p.boost_array );
    }
    
    template < typename Key, typename visitor >
    static void visit( const Key&, test::containers& p, visitor& v )
    {
        v.apply( "pair", p.pair );
        v.apply( "vector", p.vector );
        v.apply( "set", p.set );
        v.apply( "map", p.map );
        v.apply( "array", p.array );
        v.apply( "boost_array", p.boost_array );
    }
};

} } // namespace comma { namespace visiting {

namespace comma { namespace visiting { namespace test {

TEST( visiting, container )
{
    containers p;
    p.pair = std::make_pair( "blah", 111 );
    p.vector.push_back( "first" );
    p.vector.push_back( "second" );
    p.set.insert( 111 );
    p.set.insert( 222 );
    p.map.insert( std::make_pair( "jupiter", 888 ) );
    p.map.insert( std::make_pair( "saturn", 999 ) );        
    for( std::size_t i = 0; i < 3; i++ ) { p.array[i] = i; }
    for( std::size_t i = 0; i < 3; i++ ) { p.boost_array[i] = i + 3; }
    {
        std::ostringstream oss;
        o_stream_visitor v( oss );
        visiting::apply( v, p );
        EXPECT_EQ( oss.str(), "{ object:pair={ string:first=\"blah\" int:second=111 } object:vector={ string:0=\"first\" string:1=\"second\" } object:set={ int:0=111 int:1=222 } object:map={ double:jupiter=888 double:saturn=999 } object:array={ int:0=0 int:1=1 int:2=2 } object:boost_array={ int:0=3 int:1=4 int:2=5 } }" );
        //std::cerr << oss.str() << std::endl;
    }
}

TEST( visiting, tuple )
{
    std::tuple< int, double, std::string > t{ 5, 10, "hello" };
    std::ostringstream oss;
    o_stream_visitor v( oss );
    visiting::apply( v, t );
    EXPECT_EQ( oss.str(), "{ int:0=5 double:1=10 string:2=\"hello\" }" ); // EXPECT_EQ( oss.str(), "{ int:elem_0=5 double:elem_1=10 string:elem_2=\"hello\" }" );
}

TEST( visiting, optional )
{
    {
        test::optionals t;
        t.a.value = 0; // quick and dirty
        t.b.value = {0, 0}; // quick and dirty
        std::ostringstream oss;
        o_stream_visitor v( oss );
        visiting::apply( v, t );
        EXPECT_EQ( oss.str(), "{ object:a={ int:value=0 bool:is_set=false } object:b={ object:value={ int:first=0 int:second=0 } bool:is_set=false } }" );
    }
    {
        test::optionals t;
        t.a.value = 0; // quick and dirty
        t.b.value = {0, 0}; // quick and dirty
        t.a = 5;
        std::ostringstream oss;
        o_stream_visitor v( oss );
        visiting::apply( v, t );
        EXPECT_EQ( oss.str(), "{ object:a={ int:value=5 bool:is_set=true } object:b={ object:value={ int:first=0 int:second=0 } bool:is_set=false } }" );
    }
    {
        test::optionals t;
        t.a.value = 0; // quick and dirty
        t.b.value = {0, 0}; // quick and dirty
        t.b = std::make_pair( 3, 4 );
        std::ostringstream oss;
        o_stream_visitor v( oss );
        visiting::apply( v, t );
        EXPECT_EQ( oss.str(), "{ object:a={ int:value=0 bool:is_set=false } object:b={ object:value={ int:first=3 int:second=4 } bool:is_set=true } }" );
    }
}

} } } /// namespace comma { namespace visiting { namespace test {

int main( int argc, char* argv[] )
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
