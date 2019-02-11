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
#include <sstream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
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

} } } // namespace comma { namespace visiting { namespace test {

namespace comma { namespace visiting {

/// traits specialization for old_plain
template <>
struct traits< test::old_plain >
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
template <>
struct traits< test::old_plain::nested >
{
    template < typename Key, typename visitor >
    static void visit( const Key&, const test::old_plain::nested& p, visitor& v )
    {
        v.apply( "a", p.a );
        v.apply( "b", p.b );
        v.apply( "c", p.c );
    }
};

} } // namespace comma { namespace visiting {

namespace comma { namespace visiting { namespace test {

TEST( visiting, nestedConst )
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
    boost::array< comma::int32, 3 > array;
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
    }
    
    template < typename Key, typename visitor >
    static void visit( const Key&, test::containers& p, visitor& v )
    {
        v.apply( "pair", p.pair );
        v.apply( "vector", p.vector );
        v.apply( "set", p.set );
        v.apply( "map", p.map );
        v.apply( "array", p.array );
    }
};

} } // namespace comma { namespace visiting {

namespace comma { namespace visiting { namespace test {

TEST( visiting, containter )
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
    {
        std::ostringstream oss;
        o_stream_visitor v( oss );
        visiting::apply( v, p );
        EXPECT_EQ( oss.str(), "{ object:pair={ string:first=\"blah\" int:second=111 } object:vector={ string:0=\"first\" string:1=\"second\" } object:set={ int:0=111 int:1=222 } object:map={ double:jupiter=888 double:saturn=999 } object:array={ int:0=0 int:1=1 int:2=2 } }" );
        //std::cerr << oss.str() << std::endl;
    }
}

} } } /// namespace comma { namespace visiting { namespace test {

namespace comma { namespace visiting { namespace test {

struct struct_with_containers
{
    std::size_t size;
    std::vector< double > vector;
    boost::array< std::size_t, 5 > array;
};

} } } // namespace comma { namespace visiting { namespace test {

int main( int argc, char* argv[] )
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
