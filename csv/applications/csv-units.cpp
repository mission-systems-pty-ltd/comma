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
#include <boost/unordered/unordered_map.hpp>

#include <boost/units/systems/si.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/si/mass.hpp>

#include <boost/units/systems/si/temperature.hpp>
#include <boost/units/systems/temperature/celsius.hpp>
#include <boost/units/systems/temperature/fahrenheit.hpp>

#include <boost/units/base_units/us/foot.hpp>
#include <boost/units/base_units/us/pound.hpp>
#include <boost/units/base_units/metric/nautical_mile.hpp>
#include <boost/units/base_units/us/mile.hpp>
#include <boost/units/base_units/metric/knot.hpp>
#include <boost/units/base_units/angle/radian.hpp>
#include <boost/units/base_units/angle/degree.hpp>

#include <comma/application/command_line_options.h>
#include <comma/application/contact_info.h>
#include <comma/base/exception.h>
#include <comma/csv/stream.h>
#include <comma/visiting/traits.h>

void usage(char const * const txt = "")
{
    static char const * const msg_general =
        "\n"
        "\nperform unit conversion in a file or stream by specifying the conversion units and the csv fields to be converted"
        "\n"
        "\nusage: cat a.csv | csv-units <options>"
        "\n"
        "\noptions"
        "\n"
        "\n    --from <unit>   : unit converting from"
        "\n    --to   <unit>   : unit converting to"
        "\n    --scale <factor> : scale value by given factor instead of unit conversion"
        "\n                       a convenience option, probably somewhat misplaced"
        "\n"
        "\nsupported units"
        "\n    meters / feet / statute-miles / nautical-miles "
        "\n    kilograms / pounds "
        "\n    meters-per-second / knots "
        "\n    kelvin / celsius / fahrenheit "
        "\n    radians / degrees "
        "\n"
        "\n    uppercase and abbreviations"
        "\n    todo: document abbreviations";
    static char const * const msg_examples =
        "\n"
        "\nexamples"
        "\n    echo 1.2345 | csv-units --from meters --to feet "
        "\n    echo 1.2345,2.3456 | csv-units --from kilograms --to pounds --fields=a,b";

    if( 0 != txt[0] ) std::cerr << "error: " << txt << std::endl;
    std::cerr << msg_general << std::endl; // endl to make this function easier to debug by flushing
    std::cerr << "\ncsv options\n" << comma::csv::options::usage() << std::endl;
    std::cerr << msg_examples << std::endl;
    std::cerr << comma::contact_info << '\n' << std::endl;
    exit( 1 );
}

namespace units {
    // sorted
    enum et { CELSIUS,
              DEGREES,
              FAHRENHEIHT,
              FEET,
              KELVIN,
              KILOGRAMS,
              KNOTS,
              METRES,
              METRES_PER_SECOND,
              NAUTICAL_MILES,
              POUNDS,
              RADIANS,
              STATUTE_MILES,
              COUNT,
              INVALID
            };
            
    char const * name( const et val )
    {
        static char const * const NAMES[COUNT + 2]
            = { "celsius",
                "degrees",
                "fahrenheiht",
                "feet",
                "kelvin",
                "kilograms",
                "knots",
                "metres",
                "metres_per_second",
                "nautical_miles",
                "pounds",
                "radians",
                "statute_miles",
                "ERROR",
                "INVALID"
            };
        return NAMES[val];
    }

    et value( std::string const & str )
    {
        typedef boost::unordered_map<std::string, et> map_t;
        static map_t MAP;
        if ( MAP.empty() )
        {
            MAP["pounds"] = POUNDS;
            MAP["lbs"] = POUNDS;
            MAP["kilograms"] = KILOGRAMS;
            MAP["kg"] = KILOGRAMS;
            MAP["feet"] = FEET;
            MAP["ft"] = FEET;
            MAP["nautical-miles"] = NAUTICAL_MILES;
            MAP["nm"] = NAUTICAL_MILES;
            MAP["miles"] = STATUTE_MILES;
            MAP["statute-miles"] = STATUTE_MILES;
            MAP["meters"] = METRES;
            MAP["metres"] = METRES;
            MAP["meters-per-second"] = METRES_PER_SECOND;
            MAP["knots"] = KNOTS;
            MAP["radians"] = RADIANS;
            MAP["rad"] = RADIANS;
            MAP["degrees"] = DEGREES;
            MAP["deg"] = DEGREES;
            MAP["kelvin"] = KELVIN;
            MAP["celsius"] = CELSIUS;
            MAP["fahrenheit"] = FAHRENHEIHT;
        }
        
        map_t::const_iterator const citr = MAP.find( str );
        if ( MAP.cend() == citr ) return INVALID;
        return citr->second;
    }
    
    bool can_convert( const et from, const et to )
    {
        typedef boost::unordered_multimap<et, et> map_t;
        static map_t MAP;
        if ( MAP.empty() )
        {
            MAP.insert(std::make_pair(POUNDS, KILOGRAMS));
            MAP.insert(std::make_pair(KILOGRAMS, POUNDS));
            MAP.insert(std::make_pair(METRES_PER_SECOND, KNOTS));
            MAP.insert(std::make_pair(KNOTS, METRES_PER_SECOND));
            MAP.insert(std::make_pair(RADIANS, DEGREES));
            MAP.insert(std::make_pair(DEGREES, RADIANS));
            MAP.insert(std::make_pair(KELVIN, CELSIUS));
            MAP.insert(std::make_pair(KELVIN, FAHRENHEIHT));
            MAP.insert(std::make_pair(CELSIUS, KELVIN));
            MAP.insert(std::make_pair(CELSIUS, FAHRENHEIHT));
            MAP.insert(std::make_pair(FAHRENHEIHT, KELVIN));
            MAP.insert(std::make_pair(FAHRENHEIHT, CELSIUS));
            MAP.insert(std::make_pair(FEET, NAUTICAL_MILES));
            MAP.insert(std::make_pair(FEET, STATUTE_MILES));
            MAP.insert(std::make_pair(FEET, METRES));
            MAP.insert(std::make_pair(NAUTICAL_MILES, FEET));
            MAP.insert(std::make_pair(NAUTICAL_MILES, STATUTE_MILES));
            MAP.insert(std::make_pair(NAUTICAL_MILES, METRES));
            MAP.insert(std::make_pair(STATUTE_MILES, FEET));
            MAP.insert(std::make_pair(STATUTE_MILES, NAUTICAL_MILES));
            MAP.insert(std::make_pair(STATUTE_MILES, METRES));
            MAP.insert(std::make_pair(METRES, FEET));
            MAP.insert(std::make_pair(METRES, NAUTICAL_MILES));
            MAP.insert(std::make_pair(METRES, STATUTE_MILES));
        }
        
        std::pair<map_t::const_iterator, map_t::const_iterator> const crange = MAP.equal_range( from );
        if ( MAP.cend() == crange.first ) return false;
        for ( map_t::const_iterator citr = crange.first; citr != crange.second; ++citr )
            if ( to == citr->second ) return true;
        return false;
    }
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
typedef boost::units::us::mile_base_unit::unit_type statute_mile_t;
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

static int scale( double factor )
{
    comma::csv::input_stream< input_t > istream( std::cin, csv, input );
    comma::csv::output_stream< input_t > ostream( std::cout, csv, input );
    while( istream.ready() || ( std::cin.good() && !std::cin.eof() ) )
    {
        const input_t* p = istream.read();
        if( !p ) { break; }
        input_t output = *p;
        for( unsigned int i = 0; i < output.values.size(); output.values[i] = output.values[i] * factor, ++i );
        ostream.write( output, istream );
    }
    return 0;
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

static int run( const std::string& from, const std::string& to )
{    
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
        if( to == "nautical-miles" || to == "nm" ) { return run< imperial_us_length_t, nautical_mile_t >( from, to, "nautical-miles" ); }
        else if ( to == "statute-miles" || to == "miles" ) { return run< imperial_us_length_t, statute_mile_t >( from, to, "statute-miles" ); }
        return run< imperial_us_length_t, length_t >( from, to, "meters" );
    }
    if( from == "meters" )
    {
        if( to == "nautical-miles" || to == "nm" ) { return run< length_t, nautical_mile_t >( from, to, "nautical-miles" ); }
        else if ( to == "statute-miles" || to == "miles" ) { return run< length_t, statute_mile_t >( from, to, "statute-miles" ); }
        return run< length_t, imperial_us_length_t >( from, to, "feet" );
    }
    if( from == "nautical-miles" )
    {
        if( to == "feet" ) { return run< nautical_mile_t, imperial_us_length_t >( from, to, "feet" ); }
        else if ( to == "statute-miles" || to == "miles" ) { return run< nautical_mile_t, statute_mile_t >( from, to, "statute-miles" ); }
        return run< nautical_mile_t, length_t >( from, to, "meters" );
    }
    if ( from == "statute-miles" )
    {
        if( to == "feet" ) { return run< statute_mile_t, imperial_us_length_t >( from, to, "feet" ); }
        else if ( to == "nautical-miles" || to == "nm" ) { return run< statute_mile_t, nautical_mile_t >( from, to, "nautical-miles" ); }
        return run< statute_mile_t, length_t >( from, to, "meters" );
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
    COMMA_THROW( comma::exception, "unsupported conversion format " << from );
}

static std::string to_lower( const std::string& s )
{
    std::string t = s;
    for( unsigned int i = 0; i < s.size(); ++i ) { if( s[i] >= 'A' && s[i] <= 'Z' ) { t[i] = s[i] - 'A' + 'a'; } }
    return t;
}

static units::et normalized_name( const std::string& s )
{
    units::et result = units::value( to_lower( s ) );
    if ( result < 0 || result >= units::COUNT )
        COMMA_THROW( comma::exception, "unsupported or unexpected unit: \"" << s << "\"" );
    return result;
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
        boost::optional< double > scale_factor = options.optional< double >( "--scale" );
        if( scale_factor ) { return scale( *scale_factor ); }
        units::et from = normalized_name( options.value< std::string >( "--from" ) );
        units::et to = normalized_name( options.value< std::string >( "--to" ) );
        return run( units::name(from), units::name(to) );
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

