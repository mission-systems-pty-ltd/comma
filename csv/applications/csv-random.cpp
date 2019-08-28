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

#include <algorithm>
#include <string.h>
#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include "../../application/command_line_options.h"
#include "../../base/exception.h"
#include "../../base/types.h"
#include "../../csv/stream.h"
#include "../../csv/traits.h"
#include "../../string/string.h"
#include "../../visiting/traits.h"

static void usage( bool verbose )
{
    std::cerr << std::endl;
    std::cerr << "random operations on input stream" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --seed=[<int>]; random seed" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations" << std::endl;
    std::cerr << "    shuffle: output input records in pseudo-random order" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        usage: cat records.csv | csv-random shuffle [<options>] > shuffled.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        options" << std::endl;
    std::cerr << "            --fields=[<fields>]; if 'block' field present shuffle each block, otherwise read whole input and then shuffle" << std::endl;
    std::cerr << "            --sliding-window,--window=[<size>]; todo: shuffle on sliding window of <size> records" << std::endl;
    std::cerr << std::endl;
    std::cerr << "csv options:" << std::endl;
    std::cerr << comma::csv::options::usage( "", verbose ) << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

static bool verbose;
static comma::csv::options csv;
static boost::optional< int > seed;

namespace comma { namespace applications { namespace random { namespace shuffle {

struct input
{
    comma::uint32 block;
    input(): block( 0 ) {}
};

} } } } // namespace comma { namespace applications { namespace random { namespace shuffle {

namespace comma { namespace visiting {

template <> struct traits< comma::applications::random::shuffle::input >
{
    template < typename K, typename V > static void visit( const K&, const comma::applications::random::shuffle::input& p, V& v ) { v.apply( "block", p.block ); }
    template < typename K, typename V > static void visit( const K&, comma::applications::random::shuffle::input& p, V& v ) { v.apply( "block", p.block ); }
};

} } // namespace comma { namespace visiting {

namespace comma { namespace applications { namespace random { namespace shuffle {

static int run( const comma::command_line_options& options )
{
    std::default_random_engine generator = seed ? std::default_random_engine( *seed ) : std::default_random_engine();
    std::deque< std::string > records;
    auto output = []( std::deque< std::string >& records )
    { 
        for( const auto& r: records ) { std::cout.write( &r[0], r.size() ); }
        records.clear();
        if( ::csv.flush ) { std::cout.flush(); }
    };
    auto sliding_window = options.optional< unsigned int >( "--sliding-window,--window" );
    if( ::csv.has_field( "block" ) )
    {
        if( sliding_window ) { std::cerr << "csv-random: shuffle: expected either block field or --sliding-window; got both" << std::endl; return 1; }
        comma::csv::input_stream< input > is( std::cin, ::csv );
        comma::uint32 block = 0;
        while( is.ready() || std::cin.good() )
        {
            const input* p = is.read();
            if( !p || p->block != block )
            {
                std::uniform_int_distribution< int > distribution( 0, records.size() - 1 ); // quick and dirty
                std::random_shuffle( records.begin(), records.end(), [&]( int ) -> int { return distribution( generator ); } ); // quick and dirty, watch performance
                output( records );
                if( p ) { block = p->block; }
            }
            if( !p ) { break; }
            if( ::csv.binary() )
            {
                records.push_back( std::string() );
                records.back().resize( ::csv.format().size() );
                std::memcpy( &records.back()[0], is.binary().last(), ::csv.format().size() );
            }
            else
            {
                records.push_back( comma::join( is.ascii().last(), ::csv.delimiter ) + "\n" );
            }
        }
    }
    else // quick and dirty
    {
        if( sliding_window ) { std::cerr << "csv-random: shuffle: --sliding-window: todo" << std::endl; return 1; }
        if( ::csv.binary() )
        {
            std::string s( ::csv.format().size(), 0 );
            while( std::cin.good() )
            {
                std::cin.read( &s[0], s.size() );
                if( std::cin.gcount() == 0 ) { break; }
                if( std::cin.gcount() != int( s.size() ) ) { std::cerr << "csv-random: random: expected " << s.size() << " bytes; got " << std::cin.gcount() << std::endl; return 1; }
                records.push_back( std::string() );
                records.back().resize( ::csv.format().size() );
                std::memcpy( &records.back()[0], &s[0], ::csv.format().size() );
            }
        }
        else
        {
            while( std::cin.good() )
            {
                std::string s;
                std::getline( std::cin, s );
                if( !s.empty() ) { records.push_back( s + "\n" ); }
            }
        }
        std::uniform_int_distribution< int > distribution( 0, records.size() - 1 ); // quick and dirty
        std::random_shuffle( records.begin(), records.end(), [&]( int ) -> int { return distribution( generator ); } ); // quick and dirty, watch performance
        output( records );
    }
    return 0;
}

} } } } // namespace comma { namespace applications { namespace random { namespace shuffle {

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        const auto& unnamed = options.unnamed( "--flush,--verbose,-v", "-.*" );
        if( unnamed.empty() ) { std::cerr << "csv-random: please specify operation" << std::endl; return 1; }
        csv = comma::csv::options( options );
        seed = options.optional< int >( "--seed" );
        verbose = options.exists( "--verbose,-v" );
        std::string operation = unnamed[0];
        if( operation == "shuffle" ) { return comma::applications::random::shuffle::run( options ); }
        std::cerr << "csv-random: expection operation; got: '" << operation << "'" << std::endl;
        return 1;
    }
    catch( std::exception& ex ) { std::cerr << "csv-random: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-random: unknown exception" << std::endl; }
    return 1;
}
