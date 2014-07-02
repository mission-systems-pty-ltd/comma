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
#include <comma/application/command_line_options.h>
#include <comma/application/contact_info.h>
#include <comma/base/exception.h>
#include <comma/csv/stream.h>
#include <comma/visiting/traits.h>

#include <boost/units/systems/si.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/si/mass.hpp>

#include <boost/units/systems/si/temperature.hpp>
#include <boost/units/systems/temperature/celsius.hpp>
#include <boost/units/systems/temperature/fahrenheit.hpp>

#include <boost/units/base_units/us/foot.hpp>
#include <boost/units/base_units/us/pound.hpp>
#include <boost/units/base_units/metric/nautical_mile.hpp>
#include <boost/units/base_units/metric/knot.hpp>
#include <boost/units/base_units/angle/radian.hpp>
#include <boost/units/base_units/angle/degree.hpp>

void usage()
{
    std::cerr << std::endl;
    std::cerr << "perform unit conversion in a file or stream by specifying the conversion units and the csv fields to be converted" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat a.csv | csv-units <options>" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    --from <unit> : unit converting from" << std::endl;
    std::cerr << "    --to   <unit> : unit converting to" << std::endl;
    std::cerr << std::endl;
    std::cerr << "unit (supported)" << std::endl;
    std::cerr << "    meters / feet / nautical-miles " << std::endl;
    std::cerr << "    kilograms / pounds " << std::endl;
    std::cerr << "    meters-per-second / knots " << std::endl;
    std::cerr << "    kelvin / celsius / fahrenheit " << std::endl;
    std::cerr << "    radians / degrees " << std::endl;
    std::cerr << std::endl;
    std::cerr << "csv options" << std::endl;
    std::cerr << comma::csv::options::usage() << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    echo 1.2345 | csv-uints --from meters --to feet " << std::endl;
    std::cerr << "    echo 1.2345,2.3456 | csv-uints --from kilograms --to pounds --fields=a,b" << std::endl;
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

typedef boost::units::si::length::unit_type length_t;
typedef boost::units::us::foot_base_unit::unit_type imperial_us_length_t;
typedef boost::units::metric::nautical_mile_base_unit::unit_type nautical_mile_t;
typedef boost::units::si::velocity::unit_type velocity_t;
typedef boost::units::metric::knot_base_unit::unit_type knot_t;
typedef boost::units::si::kilogram_base_unit::unit_type mass_t;
typedef boost::units::us::pound_base_unit::unit_type imperial_us_mass_t;
typedef boost::units::angle::radian_base_unit::unit_type radian_t;
typedef boost::units::angle::degree_base_unit::unit_type degree_t;
typedef boost::units::absolute< boost::units::si::temperature > kelvin_t;
typedef boost::units::absolute< boost::units::celsius::temperature > celsius_t;
typedef boost::units::absolute< boost::units::fahrenheit::temperature > fahrenheit_t;
 
static bool verbose;
static comma::csv::options csv;
static input_t input;

static void init_input()
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
    csv.fields = fields;
    csv.full_xpath = true;
    input.values.resize( size );
}

template < typename From, typename To >
static int run( const std::string& from, const std::string& to, const std::string& target )
{
    if( to != target ) { std::cerr << "csv-units: don't know how to convert " << from << " to " << to << std::endl; return 1; }
    comma::csv::input_stream< input_t > istream( std::cin, csv, input );
    comma::csv::output_stream< input_t > ostream( std::cout, csv, input );

    typedef boost::units::quantity< From > from_quantity_t;
    typedef boost::units::quantity< To > to_quantity_t;
    while( istream.ready() || ( std::cin.good() && !std::cin.eof() ) )
    {
        const input_t* p = istream.read();
        if( !p ) { break; }
        input_t output = *p;
        for( unsigned int i = 0; i < output.values.size(); ++i )
        {
           output.values[i] = static_cast< to_quantity_t >( from_quantity_t( output.values[i] * From() ) ).value();
        }
        ostream.write( output, istream );
    }
    return 0;
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av );
        if( options.exists( "--help,-h" ) ) { usage(); }
        verbose = options.exists( "--verbose,-v" );
        csv = comma::csv::options( options );
        if( csv.fields.empty() ) { csv.fields="a"; }
        init_input();
        std::string from = options.value< std::string >( "--from" );
        std::string to = options.value< std::string >( "--to" );

        if( from == "pounds" )
        {
            return run< imperial_us_mass_t, mass_t >( from, to, "kilograms" );
        }
        if( from == "kilograms" )
        {
            return run< mass_t, imperial_us_mass_t >( from, to, "pounds" );
        }
        if( from == "feet" )
        {
            if( to == "nautical-miles" ) { return run< imperial_us_length_t, nautical_mile_t >( from, to, "nautical-miles" ); }
            return run< imperial_us_length_t, length_t >( from, to, "meters" );
        }
        if( from == "meters" )
        {
            if( to == "nautical-miles" ) { return run< length_t, nautical_mile_t >( from, to, "nautical-miles" ); }
            return run< length_t, imperial_us_length_t >( from, to, "feet" );
        }
        if( from == "meters-per-second" )
        {
            return run< velocity_t, knot_t >( from, to, "knots" );
        }
        if( from == "knots" )
        {
            return run< knot_t, velocity_t >( from, to, "meters-per-second" );
        }
        if( from == "radians" )
        {
            return run< radian_t, degree_t >( from, to, "degrees" );
        }
        if( from == "degrees" )
        {
            return run< degree_t, radian_t >( from, to, "radians" );
        }
        if( from == "kelvin" )
        {
            if( to == "fahrenheit" ) { return run< kelvin_t, fahrenheit_t >( from, to, "fahrenheit" ); }
            return run< kelvin_t, celsius_t >( from, to, "celsius" );
        }
        if( from == "celsius" )
        {
            if( to == "fahrenheit" ) { return run< celsius_t, fahrenheit_t >( from, to, "fahrenheit" ); }
            return run< celsius_t, kelvin_t >( from, to, "kelvin" );
        }
        if( from == "fahrenheit" )
        {
            if( to == "kelvin" ) { return run< fahrenheit_t, kelvin_t >( from, to, "kelvin" ); }
            return run< fahrenheit_t, celsius_t >( from, to, "celsius" );
        }
    }
    catch( std::exception& ex )
    {
        std::cerr << "csv-units: caught: " << ex.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << "csv-units: unknown exception" << std::endl;
    }
    return 1;
}
