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

/// @author vsevolod vlaskine

#include <deque>
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
    std::cerr << '\n';
    std::cerr << "operations on strings\n";
    std::cerr << '\n';
    std::cerr << "    usage: cat input.csv | csv-strings <operation> [<options>] > output.csv\n";
    std::cerr << '\n';
    std::cerr << "operations\n";
    std::cerr << "    add\n";
    std::cerr << "    path-basename,basename\n";
    std::cerr << "    path-common\n";
    std::cerr << "    path-dirname,dirname\n";
    std::cerr << "    path-real,path-canonical\n";
    std::cerr << '\n';
    std::cerr << "options\n";
    std::cerr << "    --emplace; perform operation emplace\n";
    std::cerr << "    --fields=[<fields>]; will perform operation on any non-empty fields\n";
    std::cerr << "                         unless different semantics specified for operation\n";
    std::cerr << "                         default: perform operation on the first field\n";
    std::cerr << "    --strict; exit on strings on which operation does not make sense\n";
    std::cerr << '\n';
    std::cerr << "add\n";
    std::cerr << "    options\n";
    std::cerr << "        --prefix=[<prefix>]; add prefix\n";
    std::cerr << "        --suffix=[<suffix>]; add suffix\n";
    std::cerr << '\n';
    std::cerr << "path-basename,basename\n";
    std::cerr << "    options\n";
    std::cerr << "        --head=<depth>; default=0; number of path elements at the beginning of the path to remove\n";
    std::cerr << "        --tail=<depth>; default=1; number of path elements at the end of the path to keep\n";
    std::cerr << "        --path-delimiter,-p=<delimiter>; default: '/'\n";
    std::cerr << '\n';
    std::cerr << "path-common\n";
    std::cerr << "    options\n";
    std::cerr << "        --once; output only the common path, do not append or emplace\n";
    std::cerr << "        --dirname-on-single-record; if only one input record, output its 'dirname'; e.g: on a single\n";
    std::cerr << "                                    input record 'a/b/c' output 'a/b'\n";
    std::cerr << "        --dirname-on-full-match; if there is an input record that fully matches the common path\n";
    std::cerr << "                                 output its 'dirname'; e.g: on 'a/b/c' and 'a/b/c/d' output 'a/b'\n";
    std::cerr << "        --path-delimiter,-p=<delimiter>; default: '/'\n";
    std::cerr << '\n';
    std::cerr << "path-dirname,dirname\n";
    std::cerr << "    options\n";
    std::cerr << "        --head=<depth>; default=0; number of path elements at the beginning of the path to keep\n";
    std::cerr << "        --tail=<depth>; default=1; number of path elements at the end of the path to remove\n";
    std::cerr << "        --path-delimiter,-p=<delimiter>; default: '/'\n";
    std::cerr << '\n';
    std::cerr << "path-real,path-canonical\n";
    std::cerr << "    options\n";
    std::cerr << "        --base=[<path>]; base path, default: current directory\n";
    std::cerr << '\n';
    std::cerr << "csv options\n";
    std::cerr << comma::csv::options::usage( "", verbose ) << '\n';
    std::cerr << '\n';
    exit( EXIT_SUCCESS );
}

static bool strict;
static comma::csv::options csv;

namespace comma { namespace applications { namespace strings {

template < typename T >
struct record
{ 
    std::vector< T > values;
    record( unsigned int n = 0 ) : values( n ) {}
};

typedef record< std::string > input;

} } } // namespace comma { namespace applications { namespace strings {

namespace comma { namespace visiting {

template < typename T > struct traits< comma::applications::strings::record< T > >
{
    template < typename K, typename V > static void visit( const K&, const comma::applications::strings::record< T >& p, V& v ) { v.apply( "values", p.values ); }
    template < typename K, typename V > static void visit( const K&, comma::applications::strings::record< T >& p, V& v ) { v.apply( "values", p.values ); }
};

} } // namespace comma { namespace visiting {

namespace comma { namespace applications { namespace strings {

template < typename T >
static int run( const comma::command_line_options& options )
{
    if( ::csv.binary() && !options.exists( "--emplace") ) { std::cerr << "csv-strings: path-" << T::name() << ": binary mode supported only for --emplace; todo, just ask\n"; exit( 1 ); }
    auto v = comma::split( ::csv.fields, options.value( "--delimiter", ',' ) );
    unsigned int n = 0;
    for( unsigned int i = 0; i < v.size(); ++i )
    {
        if( v[i].empty() ) { continue; }
        v[i] = "values[" + boost::lexical_cast< std::string >( n ) + "]";
        ++n;
    }
    ::csv.fields = n == 0 ? std::string( "values[0]" ) : comma::join( v, ',' );
    if( n == 0 ) { ++n; }
    comma::csv::input_stream< input > istream( std::cin, ::csv, input( n ) );
    std::function< void( const typename T::output_t& p ) > write;
    auto run_ = [&]() -> int
    {
        T t( options );
        while( istream.ready() || std::cin.good() )
        {
            const input* p = istream.read();
            if( !p ) { break; }
            typename T::output_t r( n );
            for( unsigned int i = 0; i < p->values.size(); ++i ) { r.values[i] = t.convert( p->values[i] ); }
            write( r );
            if( ::csv.flush ) { std::cout.flush(); }
        }
        return 0;
    };
    if( options.exists( "--emplace" ) )
    {
        comma::csv::passed< input > passed( istream, std::cout, ::csv.flush );
        write = [&]( const typename T::output_t& p ) { passed.write( p ); };
        return run_();
    }
    comma::csv::options output_csv = ::csv;
    output_csv.fields = "values";
    comma::csv::output_stream< typename T::output_t > ostream( std::cout, output_csv, input( n ) );
    comma::csv::tied< input, typename T::output_t > tied( istream, ostream );
    write = [&]( const typename T::output_t& p ) { tied.append( p ); };
    return run_();
}

namespace path {

struct basename
{
    typedef input output_t;

    unsigned int head;
    unsigned int tail;
    char delimiter;

    static constexpr char const* name() { return "basename"; }

    basename( const comma::command_line_options& options )
        : head( options.value( "--head", 0 ) )
        , tail( options.value( "--tail", 1 ) )
        , delimiter( options.value( "--path-delimiter,-p", '/' ) )
    {
        options.assert_mutually_exclusive( "--head,--tail" );
    }

    std::string convert( const std::string& t ) const
    {
        const auto& s = comma::split( t, delimiter );
        if( head > 0 )
        {
            if( s.size() >= head ) { return comma::join( s.begin() + head, s.end(), delimiter ); }
            if( strict ) { COMMA_THROW( comma::exception, "expected path depth at least " << head << "; got: '" << comma::join( s, delimiter ) << "'" ); }
            return {};
        }
        if( s.size() >= tail ) { return comma::join( s.end() - tail, s.end(), delimiter ); }
        if( strict ) { COMMA_THROW( comma::exception, "expected path depth at least " << tail << "; got: '" << comma::join( s, delimiter ) << "'" ); }
        return {};
    }
};

struct dirname
{
    typedef input output_t;
    unsigned int head;
    unsigned int tail;
    char delimiter;

    static constexpr char const* name() { return "dirname"; }

    dirname( const comma::command_line_options& options )
        : head( options.value( "--head", 0 ) )
        , tail( options.value( "--tail", 1 ) )
        , delimiter( options.value( "--path-delimiter,-p", '/' ) )
    {
        options.assert_mutually_exclusive( "--head,--tail" );
    }

    std::string convert( const std::string& t ) const
    {
        const auto& s = comma::split( t, delimiter );
        if( head > 0 )
        {
            if( s.size() >= head ) { return comma::join( s.begin(), s.begin() + head, delimiter ); }
            if( strict ) { COMMA_THROW( comma::exception, "expected path depth at least " << head << "; got: '" << comma::join( s, delimiter ) << "'" ); }
            return {};
        }
        if( s.size() >= tail ) { return comma::join( s.begin(), s.end() - tail, delimiter ); }
        if( strict ) { COMMA_THROW( comma::exception, "expected path depth at least " << tail << "; got: '" << comma::join( s, delimiter ) << "'" ); }
        return {};
    }
};

struct canonical
{
    typedef input output_t;
    
    boost::filesystem::path base;

    static constexpr char const* name() { return "canonical"; }
    
    canonical( const comma::command_line_options& options )
        : base( options.exists( "--base" )
        ? boost::filesystem::path( options.value< std::string >( "--base" ) )
        : boost::filesystem::current_path() )
    {
        if( ( options.value( "--path-delimiter,-p", '/' ) ) != '/' ) { COMMA_THROW( comma::exception, "path-canonical: expected path delimiter '/'; got: '" << options.value( "--path-delimiter,-p", '/' ) << "'" ); }
    }

    std::string convert( const std::string& s ) const
    {
        try { return boost::filesystem::canonical( boost::filesystem::path( s ), base ).string(); } catch( ... ) { if( strict ) { throw; } }
        return s;
    }
};

namespace common {

typedef input output_t;

static int run( const comma::command_line_options& options )
{
    if( ::csv.binary() ) { std::cerr << "csv-strings: path-common: binary mode: todo, just ask" << std::endl; exit( 1 ); }
    if( options.exists( "--emplace ") ) { std::cerr << "csv-strings: path-common: --emplace: todo, just ask" << std::endl; exit( 1 ); }
    auto v = comma::split( ::csv.fields, options.value( "--delimiter", ',' ) );
    unsigned int n = 0;
    for( unsigned int i = 0; i < v.size(); ++i )
    {
        if( v[i].empty() ) { continue; }
        v[i] = "values[" + boost::lexical_cast< std::string >( n ) + "]";
        ++n;
    }
    ::csv.fields = n == 0 ? std::string( "values[0]" ) : comma::join( v, ',' );
    if( n == 0 ) { ++n; }
    comma::csv::input_stream< input > istream( std::cin, ::csv, input( n ) );
    std::deque< std::string > inputs;
    output_t output;
    char delimiter = options.value( "--path-delimiter,-p", '/' );
    bool once = options.exists( "--once" );
    bool dirname_on_single_record = options.exists( "--dirname-on-single-record" );
    bool dirname_on_full_match = options.exists( "--dirname-on-full-match" );
    if( dirname_on_full_match || dirname_on_single_record ) { std::cerr << "csv-strings: path-common: --dirname-on-...: todo" << std::endl; return 1; }
    std::vector< char > full_match( output.values.size(), true );
    unsigned int count = 0;
    while( istream.ready() || std::cin.good() )
    {
        auto p = istream.read();
        if( !p ) { break; }
        if( !once ) { inputs.emplace_back( istream.last() ); }
        if( output.values.empty() ) { output.values = p->values; continue; }
        for( std::size_t i = 0; i < p->values.size(); ++i )
        {
            const std::string& common = comma::common_front( output.values[i], p->values[i], delimiter );
            if( dirname_on_single_record || dirname_on_full_match ) { full_match[i] = ( full_match[i] && output.values[i] == common ) || ( !full_match[i] && p->values[i] == common ); }
            output.values[i] = common;
        }
        ++count;
    }
    if( once )
    {
        std::string comma;
        for( auto& value : output.values ) { std::cout << comma << value; comma = ::csv.delimiter; }
        std::cout << std::endl;
    }
    else
    {
        for( const auto& input : inputs )
        {
            std::cout.write( &input[0], input.size() );
            if( ::csv.binary() )
            {
                COMMA_THROW( comma::exception, "todo" );
            }
            else
            {
                for( const auto& value: output.values ) { std::cout << ::csv.delimiter << value; }
                std::cout << std::endl;
            }
        }
    }
    return 0;
}

} // namespace common {

} // namespace path {

struct add
{
    typedef input output_t;

    std::string prefix;
    std::string suffix;

    static constexpr char const* name() { return "add"; }

    explicit add( const comma::command_line_options& options )
        : prefix( options.value( "--prefix", std::string() ) )
        , suffix( options.value( "--suffix", std::string() ) ) {}

    inline std::string convert( const std::string& t ) const { return prefix + t + suffix; }
};
    
} } } // namespace comma { namespace applications { namespace strings {

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        const auto& unnamed = options.unnamed( "--flush,--verbose,-v,--emplace,--strict,--once,--dirname-on-single-record,--dirname-on-full-match", "-.*" );
        if( unnamed.empty() ) { std::cerr << "csv-strings: please specify operation\n"; return 1; }
        std::string operation = unnamed[0];
        strict = options.exists( "--strict" );
        csv = comma::csv::options( options );
        if( operation == "add" ) { return comma::applications::strings::run< comma::applications::strings::add >( options ); }
        if( operation == "path-basename" || operation == "basename" ) { return comma::applications::strings::run< comma::applications::strings::path::basename >( options ); }
        if( operation == "path-dirname" || operation == "dirname" ) { return comma::applications::strings::run< comma::applications::strings::path::dirname >( options ); }
        if( operation == "path-real" || operation == "path-canonical" ) { return comma::applications::strings::run< comma::applications::strings::path::canonical >( options ); }
        if( operation == "path-common" ) { return comma::applications::strings::path::common::run( options ); }
        std::cerr << "csv-strings: expection operation; got: '" << operation << "'\n";
        return 1;
    }
    catch( std::exception& ex ) { std::cerr << "csv-strings: " << ex.what() << '\n'; }
    catch( ... ) { std::cerr << "csv-strings: unknown exception\n"; }
    return 1;
}
