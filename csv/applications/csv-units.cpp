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

/// Outputs the usage syntax, options and examples
/// @warning exits the program with an error code of 1
static void usage(char const * const txt = "")
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

// Specify some aliases to represent the measurement units supported.
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

/// Converts the given value between the two template measurement units 
template < typename From, typename To >
double cast( const double input )
{
    typedef boost::units::quantity< From > from_quantity_t;
    typedef boost::units::quantity< To > to_quantity_t;
    return static_cast< to_quantity_t >( from_quantity_t( input * From() ) ).value();
}

double null_cast( const double input )
{
    return input;
}

// Explicit instantiations specified here so that they can be 
// used below in a lookup table.
template double cast< imperial_us_mass_t, mass_t >( const double );
template double cast< mass_t, imperial_us_mass_t >( const double );
template double cast< imperial_us_length_t, nautical_mile_t >( const double );
template double cast< imperial_us_length_t, statute_mile_t >( const double );
template double cast< imperial_us_length_t, length_t >( const double );
template double cast< length_t, nautical_mile_t >( const double );
template double cast< length_t, statute_mile_t >( const double );
template double cast< length_t, imperial_us_length_t >( const double );
template double cast< nautical_mile_t, imperial_us_length_t >( const double );
template double cast< nautical_mile_t, statute_mile_t >( const double );
template double cast< nautical_mile_t, length_t >( const double );
template double cast< statute_mile_t, imperial_us_length_t >( const double );
template double cast< statute_mile_t, nautical_mile_t >( const double );
template double cast< statute_mile_t, length_t >( const double );
template double cast< velocity_t, knot_t >( const double );
template double cast< knot_t, velocity_t >( const double );
template double cast< radian_t, degree_t >( const double );
template double cast< degree_t, radian_t >( const double );
template double cast< kelvin_t, fahrenheit_t >( const double );
template double cast< kelvin_t, celsius_t >( const double );
template double cast< celsius_t, fahrenheit_t >( const double );
template double cast< celsius_t, kelvin_t >( const double );
template double cast< fahrenheit_t, kelvin_t >( const double );
template double cast< fahrenheit_t, celsius_t >( const double );

// A name space to wrap the manipulation of named conversions.
namespace units {
    // A set of number to identify the supported measurement units.
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

    /// Retrieve a human readable canonical name for the given number. Supports
    /// the extra two internal numbers (count and invalid) for diagnostics.
    char const * name( const et val )
    {
        if ( val < 0 || val > INVALID )
            COMMA_THROW( comma::exception, "can not get name for invalid units " << val );

        static char const * const NAMES[COUNT + 2]
            = { "celsius",
                "degrees",
                "fahrenheit",
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
                "COUNT",
                "INVALID"
            };
        return NAMES[val];
    }

    /// returns a name even if the value is invalid
    std::string debug_name( const et val )
    {
        if ( val < 0 || val > INVALID )
            return "ERROR:" + boost::lexical_cast<std::string>(val);
        return name(val);
    }

    /// Given a canonical name or an alias of a measurement unit 
    /// retrieve the canonical enumeration.
    et value( std::string const & str )
    {
        typedef boost::unordered_map<std::string, et> map_t;
        static map_t MAP;
        if ( MAP.empty() )
        {
            MAP["celsius"] = CELSIUS;
            MAP["deg"] = DEGREES;
            MAP["degrees"] = DEGREES;
            MAP["fahrenheit"] = FAHRENHEIHT;
            MAP["feet"] = FEET;
            MAP["ft"] = FEET;
            MAP["kelvin"] = KELVIN;
            MAP["kg"] = KILOGRAMS;
            MAP["kilograms"] = KILOGRAMS;
            MAP["knots"] = KNOTS;
            MAP["lbs"] = POUNDS;
            MAP["meters"] = METRES;
            MAP["meters-per-second"] = METRES_PER_SECOND;
            MAP["metres"] = METRES;
            MAP["miles"] = STATUTE_MILES;
            MAP["mi"] = STATUTE_MILES;
            MAP["m"] = METRES;
            MAP["nautical-miles"] = NAUTICAL_MILES;
            MAP["nm"] = NAUTICAL_MILES;
            MAP["pounds"] = POUNDS;
            MAP["radians"] = RADIANS;
            MAP["rad"] = RADIANS;
            MAP["statute-miles"] = STATUTE_MILES;
        }
        
        map_t::const_iterator const citr = MAP.find( str );
        if ( MAP.cend() == citr ) return INVALID;
        return citr->second;
    }
    
    /// A type to allow a lookup table for converting units
    typedef double (* cast_function)( const double );
    
    /// Retrieve a function that will convert between the two given
    /// measurement units.
    /// @returns NULL if the conversion is not supported.
    cast_function cast_lookup( const et from, const et to )
    {
        if ( from < 0 || from >= COUNT )
            COMMA_THROW( comma::exception, "can not cast lookup for invalid unit (from) " << from );
        if ( to < 0 || to >= COUNT )
            COMMA_THROW( comma::exception, "can not cast lookup for invalid unit (to) " << to );
        
        static cast_function MAP[COUNT][COUNT] = { NULL, };
        static bool initialised = false;
        if (! initialised )
        {
#define MAP_NOP(x) MAP[x][x] = null_cast;
            MAP_NOP(CELSIUS);
            MAP_NOP(DEGREES);
            MAP_NOP(FAHRENHEIHT);
            MAP_NOP(FEET);
            MAP_NOP(KELVIN);
            MAP_NOP(KILOGRAMS);
            MAP_NOP(KNOTS);
            MAP_NOP(METRES);
            MAP_NOP(METRES_PER_SECOND);
            MAP_NOP(NAUTICAL_MILES);
            MAP_NOP(POUNDS);
            MAP_NOP(RADIANS);
            MAP_NOP(STATUTE_MILES);
#undef MAP_NOP            
            MAP[POUNDS][KILOGRAMS] = cast< imperial_us_mass_t,mass_t >;
            MAP[KILOGRAMS][POUNDS] = cast< mass_t,imperial_us_mass_t >;
            MAP[METRES_PER_SECOND][KNOTS] = cast< velocity_t, knot_t >;
            MAP[KNOTS][METRES_PER_SECOND] = cast< knot_t, velocity_t >;
            MAP[RADIANS][DEGREES] = cast< radian_t, degree_t >;
            MAP[DEGREES][RADIANS] = cast< degree_t, radian_t >;
            MAP[KELVIN][CELSIUS] = cast< kelvin_t, celsius_t >;
            MAP[KELVIN][FAHRENHEIHT] = cast< kelvin_t, fahrenheit_t >;
            MAP[CELSIUS][KELVIN] = cast< celsius_t, kelvin_t >;
            MAP[CELSIUS][FAHRENHEIHT] = cast< celsius_t, fahrenheit_t >;
            MAP[FAHRENHEIHT][KELVIN] = cast< fahrenheit_t, kelvin_t >;
            MAP[FAHRENHEIHT][CELSIUS] = cast< fahrenheit_t, celsius_t >;
            MAP[FEET][NAUTICAL_MILES] = cast< imperial_us_length_t, nautical_mile_t >;
            MAP[FEET][STATUTE_MILES] = cast< imperial_us_length_t, statute_mile_t >;
            MAP[FEET][METRES] = cast< imperial_us_length_t, length_t >;
            MAP[NAUTICAL_MILES][FEET] = cast< nautical_mile_t, imperial_us_length_t >;
            MAP[NAUTICAL_MILES][STATUTE_MILES] = cast< nautical_mile_t, statute_mile_t >;
            MAP[NAUTICAL_MILES][METRES] = cast< nautical_mile_t, length_t >;
            MAP[STATUTE_MILES][FEET] = cast< statute_mile_t, imperial_us_length_t >;
            MAP[STATUTE_MILES][NAUTICAL_MILES] = cast< statute_mile_t, nautical_mile_t >;
            MAP[STATUTE_MILES][METRES] = cast< statute_mile_t, length_t >;
            MAP[METRES][FEET] = cast< length_t, imperial_us_length_t >;
            MAP[METRES][NAUTICAL_MILES] = cast< length_t, nautical_mile_t >;
            MAP[METRES][STATUTE_MILES] = cast< length_t, statute_mile_t >;
            initialised = true;
        }
        return MAP[from][to];
    }
    
    /// Test if the conversion between two measurement units is supported.
    bool can_convert( const et from, const et to )
    {
        return NULL != cast_lookup(from, to);
    }
}

/// Support reading the data from a file.
/// A type to handle a pair of fields where one is named units and the other
/// value, such that units contains the measurement standard represented by
/// the value. For example 1,feet or on another row 0.3048,metres
struct item_t
{
    double value;
    std::string units;
    item_t() : value(0.0) {}
};

struct input_t
{
    std::vector< item_t > values;    
};

namespace comma { namespace visiting {

template <> struct traits< item_t >
{
    template < typename K, typename V > static void visit( const K&, const item_t& p, V& v )
    {
        v.apply( "value", p.value );
        v.apply( "units", p.units );
    }

    template < typename K, typename V > static void visit( const K&, item_t& p, V& v )
    {
        v.apply( "value", p.value );
        v.apply( "units", p.units );
    }
};

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
static boost::unordered_map< std::string, unsigned > input_fields;

static std::string init_input_field( const std::string& v )
{
    const std::string stripped( comma::strip( v, ' ' ) );
    if ( stripped.empty() ) return std::string();
    
    const size_t pos = stripped.rfind( '/' );
    std::string head, tail;
    if ( std::string::npos == pos ) // just a
    {
        head = stripped;
        tail = "value";
    }
    else
    {
        head = stripped.substr(0, pos);
        tail = stripped.substr(pos + 1, std::string::npos);
        if ( "units" != tail && "value" != tail )
        {
            head = stripped;
            tail = "value";
        }
    }
    
    unsigned idx = input_fields.size();
    if ( input_fields.cend() == input_fields.find( head ) )
        input_fields[head] = idx;
    else
        idx = input_fields.at(head);
    
    return "values[" + boost::lexical_cast< std::string >( idx ) + "]/" + tail;
}

static void init_input()
{
    std::string fields;
    std::string comma;
    const std::vector< std::string >& v = comma::split( csv.fields, ',' );
    for( unsigned int i = 0; i < v.size(); ++i )
    {
        fields += comma;
        comma = ",";
        fields += init_input_field( v[i] );
    }
    csv.fields = fields;
    csv.full_xpath = true;
    input.values.resize( input_fields.size() ); //input.values.resize( size );
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
        for( unsigned int i = 0; i < output.values.size(); output.values[i].value = output.values[i].value * factor, ++i );
        ostream.write( output, istream );
    }
    return 0;
}

static int run( const units::et from, const units::et to )
{
    comma::csv::input_stream< input_t > istream( std::cin, csv, input );
    comma::csv::output_stream< input_t > ostream( std::cout, csv, input );

    units::cast_function const cast_fnp = units::cast_lookup( from, to );
    if (NULL == cast_fnp)
        COMMA_THROW( comma::exception, "unsupported default conversion from " << debug_name(from) << " to " << debug_name(to) );
    
    unsigned line = 0;
    while( istream.ready() || ( std::cin.good() && !std::cin.eof() ) )
    {
        const input_t* p = istream.read();
        if( !p ) { break; }
        input_t output = *p;
        for( unsigned int i = 0; i < output.values.size(); ++i )
        {
            units::cast_function fld_cast_fnp = cast_fnp;
            if ( ! output.values[i].units.empty() )
            {
                units::et fld_from = units::value( output.values[i].units );
                if ( units::INVALID == fld_from )
                    COMMA_THROW( comma::exception, "on line " << line << " unsupported units " << output.values[i].units );
                fld_cast_fnp = units::cast_lookup( fld_from, to );
                if (NULL == fld_cast_fnp)
                    COMMA_THROW( comma::exception, "on line " << line << " unsupported conversion from " << debug_name(fld_from) << " to " << debug_name(to) );
            }
            output.values[i].value = fld_cast_fnp( output.values[i].value );
            output.values[i].units = units::name( to );
        }
        ostream.write( output, istream );
        ++line;
    }
    return 0;
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
        units::et const to = normalized_name( options.value< std::string >( "--to" ) );
        units::et const from
            = ! options.exists( "--from" ) ? to : normalized_name( options.value< std::string >( "--from" ) );
        if( ! units::can_convert(from, to) )
        {
            std::cerr << "csv-units: don't know how to convert " << units::name(from) << " to " << units::name(to) << std::endl;
            return 1; 
        }
        return run( from, to );
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

