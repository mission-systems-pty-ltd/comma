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

/// @author vsevolod vlaskine

#include <string.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <boost/array.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_map.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <comma/application/command_line_options.h>
#include <comma/application/contact_info.h>
#include <comma/application/signal_flag.h>
#include <comma/base/types.h>
#include <comma/csv/stream.h>
#include <comma/csv/impl/unstructured.h>
#include <comma/io/stream.h>
#include <comma/string/string.h>
#include <comma/visiting/traits.h>

#include "details/inputs.h"

static const char* name() { return "csv-block: "; }

static void usage( bool more )
{
    std::cerr << std::endl;
    std::cerr << "operations" << std::endl;
    std::cerr << std::endl;
    std::cerr << "head" << std::endl;
    std::cerr << "  cat something.csv | csv-update head --number-of-blocks=1" << std::endl;
    std::cerr << "      reads data to stdout by the number of blocks" << std::endl;
    std::cerr << "      appends indexing field for records in each block" << std::endl;
    std::cerr << "append" << std::endl;
    std::cerr << "  cat something.csv | csv-update append --fields=,group, " << std::endl;
    std::cerr << "      appends block field base on specified id keys" << std::endl;
    std::cerr << "index" << std::endl;
    std::cerr << "  cat something.csv | csv-update append --fields=,block --index" << std::endl;
    std::cerr << "      appends block field, and block's indexing fields" << std::endl;
    std::cerr << std::endl;
    std::cerr << std::endl;

    exit(0);
}

static bool verbose;
static comma::csv::options csv;
static comma::signal_flag is_shutdown;
static comma::uint32 block = 0;
static input_t default_input;


int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        verbose = options.exists( "--verbose,-v" );
        csv = comma::csv::options( options );
        csv.full_xpath = true;
        csv.quote.reset();
        
        std::vector< std::string > unnamed = options.unnamed( "--help,-h,--verbose,-v", "-.*" );
        if( unnamed.size() < 1 ) { std::cerr << name() << "expected one operation, got " << comma::join( unnamed, ' ' ) << std::endl; return 1; }
        std::vector< std::string > v = comma::split( csv.fields, ',' );
        bool has_value_fields = false;
        for( std::size_t i = 0; !has_value_fields && i < v.size(); has_value_fields = !v[i].empty() && v[i] != "block" &&  v[i] != "id", ++i );
        std::string first_line;
        comma::csv::format f;
        if( csv.binary() ) { f = csv.format(); }
        else if( options.exists( "--format" ) ) { f = comma::csv::format( options.value< std::string >( "--format" ) ); }
        else
        {
            while( std::cin.good() && first_line.empty() ) { std::getline( std::cin, first_line ); }
            if( first_line.empty() ) { std::cerr << name() << "--format= is missing however the first line is empty" ; return 1; }
            f = comma::csv::impl::unstructured::guess_format( first_line, csv.delimiter );
            if( verbose ) { std::cerr << name() << "guessed format: " << f.string() << std::endl; }
        }
        unsigned int size = f.count();
        for( std::size_t i = 0; i < size; ++i )
        {
            if( i < v.size() )
            {
                if( v[i] == "block" ) { continue; }
                if( v[i] == "id" ) { v[i] = "key/" + default_input.key.append( f.offset( i ).type ); continue; }
            }
            if( !has_value_fields || !v[i].empty() )
            {
                v.resize( size );
                v[i] = "value/" + default_input.value.append( csv.binary() ? f.offset( i ).type : comma::csv::format::fixed_string ); // quick and dirty
            }
        }
        if( default_input.key.empty() ) { std::cerr << name() << "please specify at least one id field" << std::endl; return 1; }
        csv.fields = comma::join( v, ',' );
        if( verbose ) { std::cerr << name() << "csv fields: " << csv.fields << std::endl; }
        comma::csv::input_stream< input_t > istream( std::cin, csv, default_input );
        comma::csv::output_stream< input_t > ostream( std::cout, csv, default_input );
        
        if( !first_line.empty() ) { ostream.write( comma::csv::ascii< input_t >( csv, default_input ).get( first_line ) ); }
        while( !is_shutdown && ( istream.ready() || ( std::cin.good() && !std::cin.eof() ) ) )
        {
            const input_t* p = istream.read();
            if( !p ) { break; }
            
            ostream.write( *p );
        }
        
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << name() << ex.what() << std::endl; }
    catch( ... ) { std::cerr << name() << "unknown exception" << std::endl; }
    return 1;
}
