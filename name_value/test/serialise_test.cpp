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
#include "../../name_value/serialize.h"
#include "../../visiting/traits.h"
#include "../../xpath/xpath.h"

struct nested
{
    nested() : number( 0 ) {}
    nested( std::string name_, int number_ ) : name( name_), number( number_ ) {}
    std::string name;
    int number;
};

struct config
{
    config() : size( 0 ), alpha( 0 ), beta( 0 ) {}
    config( std::string name_, int size_, nested nest_, double alpha_, double beta_ ) : name( name_ ), size( size_ ), nest( nest_ ), alpha( alpha_ ), beta( beta_ ) {}
    std::string name;
    int size;
    nested nest;
    double alpha;
    double beta;
};

namespace comma { namespace visiting {

template <> struct traits< nested >
{
    template < typename Key, class Visitor >
    static void visit( Key, nested& nest, Visitor& v )
    {
        v.apply( "name", nest.name );
        v.apply( "number", nest.number );
    }

    template < typename Key, class Visitor >
    static void visit( Key, const nested& nest, Visitor& v )
    {
        v.apply( "name", nest.name );
        v.apply( "number", nest.number );
    }
};

template <> struct traits< config >
{
    template < typename Key, class Visitor >
    static void visit( Key, config& config, Visitor& v )
    {
        v.apply( "name", config.name );
        v.apply( "size", config.size );
        v.apply( "nest", config.nest );
        v.apply( "alpha", config.alpha );
        v.apply( "beta", config.beta );
    }

    template < typename Key, class Visitor >
    static void visit( Key, const config& config, Visitor& v )
    {
        v.apply( "name", config.name );
        v.apply( "size", config.size );
        v.apply( "nest", config.nest );
        v.apply( "alpha", config.alpha );
        v.apply( "beta", config.beta );
    }
};

} } // namespace comma { namespace visiting {

namespace comma { namespace test { namespace serialise {

static const config d( "config", 10, nested( "nest", -1 ), 1.5, 2.5 );
void test_config( const config& c )
{
    EXPECT_EQ( "config", c.name );
    EXPECT_EQ( 10, c.size );
    EXPECT_EQ( "nest", c.nest.name );
    EXPECT_EQ( -1, c.nest.number );
    EXPECT_DOUBLE_EQ( 1.5, c.alpha );
    EXPECT_DOUBLE_EQ( 2.5, c.beta );
}

TEST( serialise, json )
{
    {
        std::stringstream ss;
        comma::write_json< config >( d, ss );
        config c = comma::read_json< config >( ss );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_json< config >( d, ss );
        config c;
        comma::read_json< config >( c, ss );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_json< config >( d, ss, "root/item" );
        config c = comma::read_json< config >( ss, "root/item" );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_json< config >( d, ss, "root/item" );
        config c;
        comma::read_json< config >( c, ss, "root/item" );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_json< config >( d, ss, comma::xpath( "root/item" ) );
        config c = comma::read_json< config >( ss, comma::xpath( "root/item" ) );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_json< config >( d, ss, comma::xpath( "root/item" ) );
        config c;
        comma::read_json< config >( c, ss, comma::xpath( "root/item" ) );
        test_config( c );
    }

    {
        std::stringstream ss;
        comma::write_json< config >( d, ss );
        config c = comma::read_json< config >( ss, true );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_json< config >( d, ss );
        config c;
        comma::read_json< config >( c, ss, true );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_json< config >( d, ss, "root/item" );
        config c = comma::read_json< config >( ss, "root/item", true );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_json< config >( d, ss, "root/item" );
        config c;
        comma::read_json< config >( c, ss, "root/item", true );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_json< config >( d, ss, comma::xpath( "root/item" ) );
        config c = comma::read_json< config >( ss, comma::xpath( "root/item" ), true );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_json< config >( d, ss, comma::xpath( "root/item" ) );
        config c;
        comma::read_json< config >( c, ss, comma::xpath( "root/item" ), true );
        test_config( c );
    }
}

TEST( serialise, xml )
{
    {
        std::stringstream ss;
        comma::write_xml< config >( d, ss );
        config c = comma::read_xml< config >( ss );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_xml< config >( d, ss );
        config c;
        comma::read_xml< config >( c, ss );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_xml< config >( d, ss, "root/item" );
        config c = comma::read_xml< config >( ss, "root/item" );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_xml< config >( d, ss, "root/item" );
        config c;
        comma::read_xml< config >( c, ss, "root/item" );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_xml< config >( d, ss, comma::xpath( "root/item" ) );
        config c = comma::read_xml< config >( ss, comma::xpath( "root/item" ) );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_xml< config >( d, ss, comma::xpath( "root/item" ) );
        config c;
        comma::read_xml< config >( c, ss, comma::xpath( "root/item" ) );
        test_config( c );
    }

    {
        std::stringstream ss;
        comma::write_xml< config >( d, ss );
        config c = comma::read_xml< config >( ss, true );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_xml< config >( d, ss );
        config c;
        comma::read_xml< config >( c, ss, true );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_xml< config >( d, ss, "root/item" );
        config c = comma::read_xml< config >( ss, "root/item", true );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_xml< config >( d, ss, "root/item" );
        config c;
        comma::read_xml< config >( c, ss, "root/item", true );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_xml< config >( d, ss, comma::xpath( "root/item" ) );
        config c = comma::read_xml< config >( ss, comma::xpath( "root/item" ), true );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_xml< config >( d, ss, comma::xpath( "root/item" ) );
        config c;
        comma::read_xml< config >( c, ss, comma::xpath( "root/item" ), true );
        test_config( c );
    }
}

TEST( serialise, path_value )
{
    {
        std::stringstream ss;
        comma::write_path_value< config >( d, ss );
        config c = comma::read_path_value< config >( ss );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_path_value< config >( d, ss );
        config c;
        comma::read_path_value< config >( c, ss );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_path_value< config >( d, ss, "root/item" );
        config c = comma::read_path_value< config >( ss, "root/item" );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_path_value< config >( d, ss, "root/item" );
        config c;
        comma::read_path_value< config >( c, ss, "root/item" );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_path_value< config >( d, ss, comma::xpath( "root/item" ) );
        config c = comma::read_path_value< config >( ss, comma::xpath( "root/item" ) );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_path_value< config >( d, ss, comma::xpath( "root/item" ) );
        config c;
        comma::read_path_value< config >( c, ss, comma::xpath( "root/item" ) );
        test_config( c );
    }

    {
        std::stringstream ss;
        comma::write_path_value< config >( d, ss );
        config c = comma::read_path_value< config >( ss, true );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_path_value< config >( d, ss );
        config c;
        comma::read_path_value< config >( c, ss, true );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_path_value< config >( d, ss, "root/item" );
        config c = comma::read_path_value< config >( ss, "root/item", true );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_path_value< config >( d, ss, "root/item" );
        config c;
        comma::read_path_value< config >( c, ss, "root/item", true );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_path_value< config >( d, ss, comma::xpath( "root/item" ) );
        config c = comma::read_path_value< config >( ss, comma::xpath( "root/item" ), true );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_path_value< config >( d, ss, comma::xpath( "root/item" ) );
        config c;
        comma::read_path_value< config >( c, ss, comma::xpath( "root/item" ), true );
        test_config( c );
    }
    {
        std::stringstream ss;
        comma::write_path_value< config >( d, ss, "root/item", true, "hello/world" );
        config c;
        comma::read_path_value< config >( c, ss, "hello/world/root/item", true );
        test_config( c );
    }
}

} } } // namespace comma { namespace test { namespace serialise {
