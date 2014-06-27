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
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by the The University of Sydney.
// 4. Neither the name of the The University of Sydney nor the
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
/// @author kai huang

#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
#include <boost/scoped_ptr.hpp>
#include <comma/application/command_line_options.h>
#include <comma/application/contact_info.h>
#include <comma/base/exception.h>
#include <comma/csv/stream.h>
#include <comma/math/compare.h>
#include <comma/name_value/parser.h>
#include <comma/string/string.h>
#include <comma/visiting/traits.h>

void usage()
{
    std::cerr << std::endl;
    std::cerr << "find in a file or stream by constraints on a given key" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat a.csv | csv-select <options> [<key properties>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    todo" << std::endl;
    std::cerr << std::endl;
    std::cerr << "csv options" << std::endl;
    std::cerr << comma::csv::options::usage() << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    todo" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

struct input_t { std::vector< double > values; };

namespace comma { namespace visiting {

template <> struct traits< input_t >
{
    template < typename K, typename V > static void visit( const K&, const input_t& p, V& v )
    {
        v.apply( "values", p.values );
    }

    template < typename K, typename V > static void visit( const K&, input_t& p, V& v )
    {
        v.apply( "values", p.values );
    }
};

} } // namespace comma { namespace visiting {

static bool verbose;
static comma::csv::options csv;
static input_t input;

static void init_input( const comma::csv::format& format, const comma::command_line_options& options )
{
    std::string fields;
    std::string comma;
    const std::vector< std::string >& v = comma::split( csv.fields, ',' );
    unsigned int size = 0;
    for( unsigned int i = 0; i < v.size(); ++i )
    {
        fields += comma;
        comma = ",";

        if( !comma::strip( v[i], ' ' ).empty() ) { fields += "values[" + boost::lexical_cast< std::string >( size++ ) + "]"; }
    }
    csv.fields = comma::join( fields, ',' );
    csv.full_xpath = true;
    input.values.resize( size );
}

struct what { enum values { metres, kg, feet }; }; // todo: etc

template < what::values From, what::values To > // todo: use boost::units rather than enum
int run()
{
    comma::csv::input_stream< input_t > istream( std::cin, csv, input );
    comma::csv::output_stream< input_t > ostream( std::cout, csv, input );
    while( istream.ready() || ( std::cin.good() && !std::cin.eof() ) )
    {
        const input_t* p = istream.read();
        if( !p ) { break; }
        input_t output = *p;

        // todo: do conversion; use static_cast + boost::units (see aero traits for examples)

        ostream.write( output, istream );
    }
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av );
        if( options.exists( "--help,-h" ) ) { usage(); }
        verbose = options.exists( "--verbose,-v" );
        bool is_or = options.exists( "--or" );
        csv = comma::csv::options( options );
        init_input( csv.format(), options );

        what::values from; // todo: --from
        what::values to; // todo: --to
        switch( from )
        {
            case what::metres:
                if( to == what::feet ) { return run< what::metres, what::feet >(); }
            // degrees/radians
            // kg/pounds
            // nautical miles
            // knots
            // feet
            // todo: etc
        }


    }
    catch( std::exception& ex )
    {
        std::cerr << "csv-select: caught: " << ex.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << "csv-select: unknown exception" << std::endl;
    }
    return 1;
}
