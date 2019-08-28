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

static void usage( bool more )
{
    std::cerr << std::endl;
    std::cerr << "Sort a csv file using one or several keys" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Usage: cat something.csv | csv-random [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "    --help,-h: help; --help --verbose: more help" << std::endl;
    std::cerr << "    --discard-out-of-order,--discard-unsorted: instead of sorting, discard records out of order" << std::endl;
    std::cerr << "    --first: first line matching given keys; first line in the block, if block field present; no sorting will be done; if sorting required, use unique instead" << std::endl;
    std::cerr << "           fields" << std::endl;
    std::cerr << "               id: if present, multiple id fields accepted; output first record for each set of ids in a given block; e.g. --fields=id,a,,id" << std::endl;
    std::cerr << "               block: if present; output minimum for each contiguous block" << std::endl;
    std::cerr << "    --min: output only record(s) with minimum value for a given field." << std::endl;
    std::cerr << "           fields" << std::endl;
    std::cerr << "               id: if present, multiple id fields accepted; output minimum for each set of ids in a given block; e.g. --fields=id,a,,id" << std::endl;
    std::cerr << "               block: if present; output minimum for each contiguous block" << std::endl;
    std::cerr << "    --max: output record(s) with maximum value, same semantics as --min" << std::endl;
    std::cerr << "           --min and --max may be used together." << std::endl;
    std::cerr << "    --numeric-keys-are-floats,--floats; in ascii, if --format not present, assume that numeric fields are floating point numbers" << std::endl;
    std::cerr << "    --order=<fields>: order in which to sort fields; default is input field order" << std::endl;
    std::cerr << "    --random: output input records in pseudo-random order" << std::endl;
    std::cerr << "    --random-seed,--seed=[<int>]; random seed for --random" << std::endl;
    std::cerr << "    --reverse,--descending,-r: sort in reverse order" << std::endl;
    std::cerr << "    --sliding-window,--window=<size>: sort last <size> entries" << std::endl;
    std::cerr << "    --string,-s: keys are strings; a quick and dirty option to support strings" << std::endl;
    std::cerr << "                 default: double" << std::endl;
    std::cerr << "    --unique,-u: sort input, output only the first line matching given keys; if no sorting required, use --first for better performance" << std::endl;
    std::cerr << "    --verbose,-v: more output to stderr" << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    sort by first field:" << std::endl;
    std::cerr << "        echo -e \"2\\n1\\n3\" | csv-random --fields=a" << std::endl;
    std::cerr << "    sort by second field:" << std::endl;
    std::cerr << "        echo -e \"2,3\\n1,1\\n3,2\" | csv-random --fields=,b" << std::endl;
    std::cerr << "    sort by second field then first field:" << std::endl;
    std::cerr << "        echo -e \"2,3\\n3,1\\n1,1\\n2,2\\n1,3\" | csv-random --fields=a,b --order=b,a" << std::endl;
    std::cerr << "    minimum (using maximum would be the same):" << std::endl;
    std::cerr << "        basic use" << std::endl;
    std::cerr << "            ( echo 1,a,2; echo 2,a,2; echo 3,a,3; ) | csv-random --min --fields=,,a" << std::endl;
    std::cerr << "        using single id" << std::endl;
    std::cerr << "            ( echo 1,a,2; echo 2,a,2; echo 3,b,3; ) | csv-random --min --fields=a,id" << std::endl;
    std::cerr << "        using multiple id fields" << std::endl;
    std::cerr << "            ( echo 1,a,1; echo 1,b,1; echo 3,b,5; echo 3,b,5; ) | csv-random --min --fields=id,a,id" << std::endl;
    std::cerr << "        using block" << std::endl;
    std::cerr << "            ( echo 0,a,2; echo 0,a,2; echo 0,b,3; echo 0,b,1; echo 1,c,3; echo 1,c,2; ) | csv-random --min --fields=block,,a" << std::endl;
    std::cerr << "        using block and id" << std::endl;
    std::cerr << "            ( echo 0,a,2; echo 0,a,2; echo 0,b,3; echo 0,b,1; echo 1,c,3; echo 1,c,2; ) | csv-random --min --fields=block,id,a" << std::endl;
    std::cerr << "    minimum and maximum:" << std::endl;
    std::cerr << "        basic use" << std::endl;
    std::cerr << "            ( echo 1,a,2; echo 2,a,2; echo 3,b,3; echo 5,b,7; echo 3,b,9 ) | csv-random --max --min --fields=,,a" << std::endl;
    std::cerr << "        using id" << std::endl;
    std::cerr << "            ( echo 1,a,2; echo 2,a,2; echo 3,b,3; echo 5,b,7; echo 3,b,9 ) | csv-random --max --min --fields=,id,a" << std::endl;
    std::cerr << std::endl;
    if( more )
    {
        std::cerr << std::endl;
        std::cerr << "csv options:" << std::endl;
        std::cerr << comma::csv::options::usage() << std::endl;
    }
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
    if( ::csv.has_field( "block" ) )
    {
        comma::csv::input_stream< input > is( std::cin, ::csv );
        comma::uint32 block = 0;
        while( is.ready() || std::cin.good() )
        {
            const input* p = is.read();
            if( !p || p->block != block )
            {
                std::uniform_int_distribution< int > distribution( 0, records.size() - 1 ); // quick and dirty
                std::random_shuffle( records.begin(), records.end(), [&]( int ) -> int { return distribution( generator ); } ); // quick and dirty, watch performance
                for( const auto& r: records ) { std::cout.write( &r[0], r.size() ); }
                if( ::csv.flush ) { std::cout.flush(); }
                records.clear();
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
    else
    {
        // todo: quick and dirty, code duplication
        // todo: implement --sliding-window
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
        for( const auto& r: records ) { std::cout.write( &r[0], r.size() ); }
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
