// Copyright (c) 2023 Vsevolod Vlaskine
// All rights reserved.

#include <cmath>
#include <exception>
#include <iostream>
#include <comma/application/command_line_options.h>
#include <comma/csv/stream.h>
#include <comma/string/string.h>
#include <comma/visiting/traits.h>

static void usage( bool verbose )
{
    std::cerr << std::endl;
    std::cerr << "example: read csv/binary fixed-width data on stdin (angle and factor)" << std::endl;
    std::cerr << "         count input records, calculate sin and cos of angle, and sign of factor" << std::endl;
    std::cerr << "         append to input, output to stdout" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat values.csv | ./comma-csv-sample-application-append [<options>] > result.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --input-fields; print input fields to stdout and exit" << std::endl;
    std::cerr << "    --output-fields; print output fields to stdout and exit" << std::endl;
    std::cerr << "    --output-format; print output format to stdout and exit" << std::endl;
    std::cerr << "    --no-append; do not output stdin records to stdout, output only result" << std::endl;
    std::cerr << std::endl;
    std::cerr << "csv options" << std::endl;
    std::cerr << comma::csv::options::usage( verbose ) << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    ascii" << std::endl;
    std::cerr << "        ( echo 45,20; echo 30,-10 ) | ./comma-csv-sample-application-append" << std::endl;
    std::cerr << "    binary" << std::endl;
    std::cerr << "        ( echo 1,2,3; echo 4,5,6 ) \\" << std::endl;
    std::cerr << "            | csv-to-bin 3ui \\" << std::endl;
    std::cerr << "            | ./comma-csv-sample-application-append --binary 3ui" << std::endl;
    std::cerr << "            | csv-from-bin 3ui,ui,d" << std::endl;
    std::cerr << std::endl;
}

namespace comma { namespace csv { namespace examples { namespace application {

struct input
{
    double angle{0};
    double factor{0};
};

struct output
{
    struct trigonometric_t
    {
        double sin;
        double cos;
    };
    unsigned int count{0};
    trigonometric_t trigonometric;
    double factor_sign{0};
};

} } } } // namespace comma { namespace csv { namespace examples { namespace application {

namespace comma { namespace visiting {

template <> struct traits< comma::csv::examples::application::input >
{
    template < typename Key, class Visitor > static void visit( const Key&, const comma::csv::examples::application::input& p, Visitor& v )
    {
        v.apply( "angle", p.angle );
        v.apply( "factor", p.factor );
    }

    template < typename Key, class Visitor > static void visit( const Key&, comma::csv::examples::application::input& p, Visitor& v )
    {
        v.apply( "angle", p.angle );
        v.apply( "factor", p.factor );
    }
};

template <> struct traits< comma::csv::examples::application::output::trigonometric_t >
{
    template < typename Key, class Visitor > static void visit( const Key&, const comma::csv::examples::application::output::trigonometric_t& p, Visitor& v )
    {
        v.apply( "sin", p.sin );
        v.apply( "cos", p.cos );
    }
};

template <> struct traits< comma::csv::examples::application::output >
{
    template < typename Key, class Visitor > static void visit( const Key&, const comma::csv::examples::application::output& p, Visitor& v )
    {
        v.apply( "count", p.count );
        v.apply( "trigonometric", p.trigonometric );
        v.apply( "factor_sign", p.factor_sign );
    }
};

} } // namespace comma { namespace visiting {

static comma::csv::examples::application::output& populate_output( const comma::csv::examples::application::input& input
                                                                 , comma::csv::examples::application::output& output )
{
    ++output.count;
    output.trigonometric.sin = std::sin( input.angle * M_PI / 180 );
    output.trigonometric.cos = std::cos( input.angle * M_PI / 180 );
    output.factor_sign = input.factor > 0 ? 1 : input.factor < 0 ? -1 : 0;
    return output;
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        comma::csv::options csv( options );
        typedef comma::csv::examples::application::input input_t;
        typedef comma::csv::examples::application::output output_t;
        if( options.exists( "--input-fields" ) ) { std::cout << comma::join( comma::csv::names< input_t >(), ',' ) << std::endl; return 0; }
        if( options.exists( "--output-fields" ) ) { std::cout << comma::join( comma::csv::names< output_t >(), ',' ) << std::endl; return 0; }
        if( options.exists( "--output-format" ) ) { std::cout << comma::csv::format::value< output_t >() << std::endl; return 0; }
        comma::csv::input_stream< comma::csv::examples::application::input > is( std::cin, csv );
        comma::csv::output_stream< comma::csv::examples::application::output > os( std::cout, csv.binary() );
        auto tied = comma::csv::make_tied( is, os );
        bool append = !options.exists( "--no-append" );
        comma::csv::examples::application::output output;
        while( is.ready() || std::cin.good() )
        {
            auto p = is.read();
            if( !p ) { break; }
            if( append ) { tied.append( populate_output( *p, output ) ); }
            else { os.write( populate_output( *p, output ) ); }
        }
        return 0;
    }
    catch( const std::exception& e ) { std::cerr << av[0] << ": " << e.what() << std::endl; }
    catch( ... ) { std::cerr << av[0] << ": unknown exception" << std::endl; }
    return 1;
}