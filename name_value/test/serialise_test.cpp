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
#include "../../base/variant.h"
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

struct forest
{
    struct sounds
    {
        struct chirp { int a{1}; int b{2}; };
        struct whistle { int a{3}; int b{4}; };
        struct warble { int x{5}; int y{6}; };
        struct silence {};
    };

    struct naming { static std::array< std::string, 3 > names() { return { "chirp", "whistle", "warble" }; } };

    typedef comma::named_variant< naming, sounds::chirp, sounds::whistle, sounds::warble > variant_t;
    comma::make_named_variant< naming >::variant< sounds::chirp, sounds::whistle, sounds::warble >::type madeup; // todo
    std::array< variant_t, 3 > choir; // todo
    comma::named_variant< naming, sounds::chirp, sounds::whistle, sounds::warble, sounds::silence > maybesound; // todo
    comma::named_variant< naming, sounds::chirp, sounds::whistle, sounds::warble > sound;
    
};

namespace comma { namespace visiting {

template <> struct traits< forest::sounds::chirp >
{
    template < typename Key, class Visitor > static void visit( Key, forest::sounds::chirp& t, Visitor& v ) { v.apply( "a", t.a ); v.apply( "b", t.b ); }
    template < typename Key, class Visitor > static void visit( Key, const forest::sounds::chirp& t, Visitor& v ) { v.apply( "a", t.a ); v.apply( "b", t.b ); }
};

template <> struct traits< forest::sounds::whistle >
{
    template < typename Key, class Visitor > static void visit( Key, forest::sounds::whistle& t, Visitor& v ) { v.apply( "a", t.a ); v.apply( "b", t.b ); }
    template < typename Key, class Visitor > static void visit( Key, const forest::sounds::whistle& t, Visitor& v ) { v.apply( "a", t.a ); v.apply( "b", t.b ); }
};

template <> struct traits< forest::sounds::warble >
{
    template < typename Key, class Visitor > static void visit( Key, forest::sounds::warble& t, Visitor& v ) { v.apply( "x", t.x ); v.apply( "y", t.y ); }
    template < typename Key, class Visitor > static void visit( Key, const forest::sounds::warble& t, Visitor& v ) { v.apply( "x", t.x ); v.apply( "y", t.y ); }
};

template <> struct traits< forest::sounds::silence >
{
    template < typename Key, class Visitor > static void visit( Key, forest::sounds::silence& t, Visitor& v ) {}
    template < typename Key, class Visitor > static void visit( Key, const forest::sounds::silence& t, Visitor& v ) {}
};

template <> struct traits< forest >
{
    template < typename Key, class Visitor > static void visit( Key, forest& t, Visitor& v )
    {
        v.apply( "madeup", t.madeup ); // todo
        v.apply( "choir", t.choir ); // todo
        v.apply( "maybesound", t.maybesound ); // todo
        v.apply( "sound", t.sound );
    }
    template < typename Key, class Visitor > static void visit( Key, const forest& t, Visitor& v )
    {
        v.apply( "madeup", t.madeup ); // todo
        v.apply( "choir", t.choir ); // todo
        v.apply( "maybesound", t.maybesound ); // todo
        v.apply( "sound", t.sound );
    }
};

} } // namespace comma { namespace visiting {

TEST( serialise, variant )
{
    {
        forest f;
        {
            std::ostringstream oss;
            comma::write_json( f, oss, false );
            EXPECT_EQ( oss.str(), "{}" );
        }
        {
            f.sound.set( forest::sounds::chirp{11, 22} );
            std::ostringstream oss;
            comma::write_json( f, oss, false );
            EXPECT_EQ( oss.str(), "{\"sound\":{\"chirp\":{\"a\":11,\"b\":22}}}" );
        }
        {
            f.sound.set( forest::sounds::whistle{33, 44} );
            std::ostringstream oss;
            comma::write_json( f, oss, false );
            EXPECT_EQ( oss.str(), "{\"sound\":{\"whistle\":{\"a\":33,\"b\":44}}}" );
        }
        {
            f.sound.set( forest::sounds::warble{55, 66} );
            std::ostringstream oss;
            comma::write_json( f, oss, false );
            EXPECT_EQ( oss.str(), "{\"sound\":{\"warble\":{\"x\":55,\"y\":66}}}" );
        }
        {
            f.sound.reset();
            std::ostringstream oss;
            comma::write_json( f, oss, false );
            EXPECT_EQ( oss.str(), "{}" );
        }
    }
    // { // todo?
    //     forest f;
    //     std::ostringstream oss;
    //     comma::write_json( f, oss, false );
    //     EXPECT_EQ( oss.str(), "{}" );
    //     f.maybesound.set( forest::sounds::silence() );
    //     comma::write_json( f, oss, false );
    //     //EXPECT_EQ( oss.str(), "{\"sound\":{\"silence\":\"\"}}" );
    // }
}

TEST( deserialise, variant )
{
    {
        forest g;
        {
            std::istringstream iss( "{\"sound\":{\"warble\":{\"x\":55,\"y\":66}}}" );
            comma::read_json( g, iss );
            EXPECT_TRUE( g.sound.is< forest::sounds::warble >() );
            EXPECT_EQ( g.sound.get< forest::sounds::warble >().x, 55 );
            EXPECT_EQ( g.sound.get< forest::sounds::warble >().y, 66 );
            
        }
        {
            std::istringstream iss( "{\"sound\":{\"chirp\":{\"a\":77,\"b\":88}}}" );
            comma::read_json( g, iss );
            EXPECT_TRUE( g.sound.is< forest::sounds::chirp >() );
            EXPECT_EQ( g.sound.get< forest::sounds::chirp >().a, 77 );
            EXPECT_EQ( g.sound.get< forest::sounds::chirp >().b, 88 );
        }
    }
}
