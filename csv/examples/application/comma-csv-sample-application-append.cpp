// Copyright (c) 2023 Vsevolod Vlaskine
// All rights reserved.

#include <exception>
#include <iostream>
#include <comma/application/command_line_options.h>
#include <comma/csv/stream.h>
#include <comma/string/string.h>
#include <comma/visiting/traits.h>

static void usage( bool verbose )
{
    std::cerr << std::endl;
    std::cerr << "example: read csv/binary fixed-width data on stdin, calculate some result" << std::endl;
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
    std::cerr << "        ( echo 1,2,3; echo 4,5,6 ) | ./comma-csv-sample-application-append" << std::endl;
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
    struct nested { double d{0}; };
    unsigned int a{0};
    double b{0};
    nested c;
};

struct output
{
    unsigned int count{0};
    double result{0};
};

} } } } // namespace comma { namespace csv { namespace examples { namespace application {

namespace comma { namespace visiting {

template <> struct traits< comma::csv::examples::application::input::nested >
{
    template < typename Key, class Visitor > static void visit( const Key&, const comma::csv::examples::application::input::nested& p, Visitor& v )
    {
        v.apply( "d", p.d );
    }

    template < typename Key, class Visitor > static void visit( const Key&, comma::csv::examples::application::input::nested& p, Visitor& v )
    {
        v.apply( "d", p.d );
    }
};

template <> struct traits< comma::csv::examples::application::input >
{
    template < typename Key, class Visitor > static void visit( const Key&, const comma::csv::examples::application::input& p, Visitor& v )
    {
        v.apply( "a", p.a );
        v.apply( "b", p.b );
        v.apply( "c", p.c );
    }

    template < typename Key, class Visitor > static void visit( const Key&, comma::csv::examples::application::input& p, Visitor& v )
    {
        v.apply( "a", p.a );
        v.apply( "b", p.b );
        v.apply( "c", p.c );
    }
};

template <> struct traits< comma::csv::examples::application::output >
{
    template < typename Key, class Visitor > static void visit( const Key&, const comma::csv::examples::application::output& p, Visitor& v )
    {
        v.apply( "count", p.count );
        v.apply( "result", p.result );
    }
};

} } // namespace comma { namespace visiting {

static comma::csv::examples::application::output& populate_output( const comma::csv::examples::application::input& input
                                                                 , comma::csv::examples::application::output& output )
{
    ++output.count;
    output.result = input.a + input.b * input.c.d;
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