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

struct nested
{
    nested() : number( 0 ) {}
    std::string name;
    int number;
};

struct config
{
    config() : size(0), alpha(0), beta(0) {}
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

namespace comma { namespace test { namespace read {

static const std::string json =
    "{"
    " \"name\": \"dummy\", "
    " \"size\": \"10\", "
    " \"nest\": {"
    "             \"name\": \"nested\", "
    "             \"number\": \"20\" "
    "           }, "
    " \"alpha\": \"1.5\", "
    " \"beta\": \"2.5\" "
    "}";
    
static const std::string json_root = "{ \"root\": " +  json + "}";

static const std::string xml = 
    " <name>dummy</name> "
    " <size>10</size> "
    " <nest> "
    "         <name>nested</name> "
    "         <number>20</number> "
    " </nest> "
    " <alpha>1.5</alpha> "
    " <beta>2.5</beta> ";
    
static const std::string xml_root =  "<root> " + xml + " </root>";

static const std::string name_value =
    " name=dummy "
    " size=10 "
    " nest={"
    "        name=nested "
    "        number=20 "
    "      } "
    " alpha=1.5 "
    " beta=2.5 ";
    
static const std::string name_value_root = "root={" +  name_value + "}";

static const std::string corrupted = "< = {";

void test_config( const config& c )
{
    EXPECT_EQ( "dummy", c.name );
    EXPECT_EQ( 10, c.size );
    EXPECT_EQ( "nested", c.nest.name );
    EXPECT_EQ( 20, c.nest.number );
    EXPECT_DOUBLE_EQ( 1.5, c.alpha );
    EXPECT_DOUBLE_EQ( 2.5, c.beta );    
}

void test_interface( std::istringstream& iss )
{
    { config c; comma::read< config >( c, iss ); test_config( c ); }
    { config c; comma::read< config >( c, iss, true ); test_config( c ); }
    { config c = comma::read< config >( iss ); test_config( c ); }
    { config c = comma::read< config >( iss, true ); test_config( c ); }
}

void test_interface( std::istringstream& iss, const char *root )
{
    { config c; comma::read< config >( c, iss, root ); test_config( c ); }
    { config c; comma::read< config >( c, iss, root, true ); test_config( c ); }
    { config c; comma::read< config >( c, iss, xpath( root ) ); test_config( c ); }
    { config c; comma::read< config >( c, iss, xpath( root ), true ); test_config( c ); }
    { config c = comma::read< config >( iss, root ); test_config( c ); }
    { config c = comma::read< config >( iss, root, true ); test_config( c ); }
    { config c = comma::read< config >( iss, xpath( root ) ); test_config( c ); }
    { config c = comma::read< config >( iss, xpath( root ), true ); test_config( c ); }
}

TEST ( name_value_read, read_json )
{
    { std::istringstream iss( json ); test_interface( iss ); }
    { std::istringstream iss( json_root ); test_interface( iss, "root" ); }
}

TEST ( name_value_read, read_xml )
{
    { std::istringstream iss( xml ); test_interface( iss ); }
    { std::istringstream iss( xml_root ); test_interface( iss, "root" ); }
}

TEST ( name_value_read, read_name_value )
{
    { std::istringstream iss( name_value ); test_interface( iss ); }
    { std::istringstream iss( name_value_root ); test_interface( iss, "root" ); }
}

TEST ( name_value_read, read_corrupted )
{
    std::istringstream iss( corrupted );
    config c; 
    ASSERT_THROW( comma::read< config >( c, iss ), comma::exception );
}


} } } // namespace comma { namespace test { namespace read {
