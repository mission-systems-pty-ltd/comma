// This file is provided in addition to comma and is not an integral
// part of comma library.
// Copyright (c) 2018 Vsevolod Vlaskine
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
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

// comma is a generic and flexible library
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

/// @author vsevolod vlaskine

#include <functional>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include "../../application/command_line_options.h"
#include "../../base/exception.h"
#include "../../csv/stream.h"
#include "../../csv/traits.h"
#include "../../string/string.h"

static void usage( bool verbose )
{
    std::cerr << std::endl;
    std::cerr << "operations on strings" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    usage: cat input.csv | csv-strings <operation> [<options>] > output.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations" << std::endl;
    std::cerr << "    path-basename,basename" << std::endl;
    std::cerr << "    path-dirname,dirname" << std::endl;
    std::cerr << "    path-realpath,path-canonical,canonical" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --emplace; perform operation emplace" << std::endl;
    std::cerr << "    --fields=[<fields>]; will perform operation on any non-empty fields" << std::endl;
    std::cerr << "                         unless different semantics specified for operation" << std::endl;
    std::cerr << "                         default: perform operation on the first field" << std::endl;
    std::cerr << "    --strict; exit on strings on which operation does not make sense" << std::endl;
    std::cerr << std::endl;
    std::cerr << "path-basename,basename" << std::endl;
    std::cerr << "    options" << std::endl;
    std::cerr << "        --depth=<depth>; default=1; if path length less than depth, output empty string" << std::endl;
    std::cerr << "        --path-delimiter,-p=<delimiter>; default=/" << std::endl;
    std::cerr << std::endl;
    std::cerr << "path-dirname,dirname" << std::endl;
    std::cerr << "    options" << std::endl;
    std::cerr << "        --depth=<depth>; default=1; if path length less than depth, output empty string" << std::endl;
    std::cerr << "        --fixed-depth=[<depth>]; output paths of fixed depth starting from root" << std::endl;
    std::cerr << "        --path-delimiter,-p=<delimiter>; default=/" << std::endl;
    std::cerr << std::endl;
    std::cerr << "path-realpath,path-canonical,canonical" << std::endl;
    std::cerr << "    options" << std::endl;
    std::cerr << "        --base=[<path>]; base path, default: current directory" << std::endl;
    std::cerr << std::endl;
    std::cerr << "csv options:" << std::endl;
    std::cerr << comma::csv::options::usage( "", verbose ) << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

static bool strict;
static comma::csv::options csv;

namespace comma { namespace applications { namespace strings { namespace path {

struct input
{ 
    std::vector< std::string > strings;
    input( unsigned int n = 0 ): strings( n ) {}
};

} } } } // namespace comma { namespace applications { namespace strings { namespace path {

namespace comma { namespace visiting {

template <> struct traits< comma::applications::strings::path::input >
{
    template < typename K, typename V > static void visit( const K&, const comma::applications::strings::path::input& p, V& v ) { v.apply( "strings", p.strings ); }
    template < typename K, typename V > static void visit( const K&, comma::applications::strings::path::input& p, V& v ) { v.apply( "strings", p.strings ); }
};

} } // namespace comma { namespace visiting {

namespace comma { namespace applications { namespace strings { namespace path {

template < typename T >
static int run( const comma::command_line_options& options )
{
    auto v = comma::split( ::csv.fields, ',' );
    unsigned int n = 0;
    for( unsigned int i = 0; i < v.size(); ++i )
    {
        if( v[i].empty() ) { continue; }
        v[i] = "strings[" + boost::lexical_cast< std::string >( n ) + "]";
        ++n;
    }
    ::csv.fields = n == 0 ? std::string( "strings[0]" ) : comma::join( v, ',' );
    if( n == 0 ) { ++n; }
    comma::csv::input_stream< input > istream( std::cin, ::csv, input( n ) );
    std::function< void( const input& p ) > write;
    auto run_ = [&]()->int
    {
        T t( options );
        while( istream.ready() || std::cin.good() )
        {
            const input* p = istream.read();
            if( !p ) { break; }
            input r( n );
            for( unsigned int i = 0; i < p->strings.size(); ++i ) { r.strings[i] = t.convert( p->strings[i] ); }
            write( r );
            if( ::csv.flush ) { std::cout.flush(); }
        }
        return 0;
    };
    if( options.exists( "--emplace" ) )
    {
        comma::csv::passed< input > passed( istream, std::cout, ::csv.flush );
        write = [&]( const input& p ) { passed.write( p ); };
        return run_();
    }
    comma::csv::options output_csv = ::csv;
    output_csv.fields = "strings";
    if( ::csv.binary() ) { std::cerr << "csv-strings: path-" << T::name() << ": binary mode supported only for --emplace; todo, just ask" << std::endl; exit( 1 ); }
    comma::csv::output_stream< input > ostream( std::cout, output_csv, input( n ) );
    comma::csv::tied< input, input > tied( istream, ostream );
    write = [&]( const input& p ) { tied.append( p ); };
    return run_();
}

struct basename
{
    unsigned int depth;
    char delimiter;
    
    static const char* name() { return "basename"; }
    
    basename( const comma::command_line_options& options )
        : depth( options.value( "--depth", 1 ) )
        , delimiter( options.value( "--path-delimiter,-p", '/' ) )
    {
    }
    
    std::string convert( const std::string& t )
    {
        const auto& s = comma::split( t, delimiter );
        if( s.size() < depth )
        {
            if( strict ) { COMMA_THROW( comma::exception, "expected path depth at least " << depth << "; got: '" << comma::join( s, delimiter ) << "'" ); }
            return "";
        }
        return comma::join( s.end() - depth, s.end(), delimiter );
    }
};

struct dirname
{
    unsigned int depth;
    unsigned int fixed_depth;
    char delimiter;
    
    static const char* name() { return "dirname"; }
    
    dirname( const comma::command_line_options& options )
        : depth( options.value( "--depth", 1 ) )
        , fixed_depth( options.value( "--fixed-depth", 0 ) )
        , delimiter( options.value( "--path-delimiter,-p", '/' ) )
    {
        options.assert_mutually_exclusive( "--depth,--fixed-depth" );
    }
    
    std::string convert( const std::string& t )
    {
        const auto& s = comma::split( t, delimiter );
        if( fixed_depth > 0 )
        {
            if( s.size() < fixed_depth )
            {
                if( strict ) { COMMA_THROW( comma::exception, "expected path depth at least " << fixed_depth << "; got: '" << comma::join( s, delimiter ) << "'" ); }
                return "";
            }
            return comma::join( s, fixed_depth, delimiter );
        }
        if( s.size() < depth )
        {
            if( strict ) { COMMA_THROW( comma::exception, "expected path depth at least " << depth << "; got: '" << comma::join( s, '/' ) << "'" ); }
            return "";
        }
        return comma::join( s.begin(), s.end() - depth, delimiter );
    }
};

struct canonical
{
    boost::filesystem::path base;
    
    static const char* name() { return "canonical"; }
    
    canonical( const comma::command_line_options& options )
        : base( options.exists( "--base" )
        ? boost::filesystem::path( options.value< std::string >( "--base" ) )
        : boost::filesystem::current_path() )
    {
        if( ( options.value( "--path-delimiter,-p", '/' ) ) != '/' ) { COMMA_THROW( comma::exception, "path-canonical: expected path delimiter '/'; got: '" << options.value( "--path-delimiter,-p", '/' ) << "'" ); }
    }
    
    std::string convert( const std::string& s )
    {
        try { return boost::filesystem::canonical( boost::filesystem::path( s ), base ).string(); } catch( ... ) { if( strict ) { throw; } }
        return s;
    }
};

} } } } // namespace comma { namespace applications { namespace strings { namespace path {

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        const auto& unnamed = options.unnamed( "--flush,--verbose,-v,--emplace", "-.*" );
        if( unnamed.empty() ) { std::cerr << "csv-strings: please specify operation" << std::endl; return 1; }
        std::string operation = unnamed[0];
        strict = options.exists( "--strict" );
        csv = comma::csv::options( options );
        if( operation == "path-basename" || operation == "basename" ) { return comma::applications::strings::path::run< comma::applications::strings::path::basename >( options ); }
        if( operation == "path-dirname" || operation == "dirname" ) { return comma::applications::strings::path::run< comma::applications::strings::path::dirname >( options ); }
        if( operation == "path-realpath" || operation == "path-canonical" || operation == "canonical" ) { return comma::applications::strings::path::run< comma::applications::strings::path::canonical >( options ); }
        std::cerr << "csv-strings: expection operation; got: '" << operation << "'" << std::endl;
        return 1;
    }
    catch( std::exception& ex ) { std::cerr << "csv-strings: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-strings: unknown exception" << std::endl; }
    return 1;
}
