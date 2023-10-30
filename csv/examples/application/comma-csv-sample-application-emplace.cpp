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
    std::cerr << "         update input values emplace, output to stdout" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat values.csv | ./comma-csv-sample-application-emplace [<options>] > result.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --input-fields; print input fields to stdout and exit" << std::endl;
    std::cerr << std::endl;
    std::cerr << "csv options" << std::endl;
    std::cerr << comma::csv::options::usage( verbose ) << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    ascii" << std::endl;
    std::cerr << "        ( echo 1,2; echo 3,4 ) | ./comma-csv-sample-application-emplace" << std::endl;
    std::cerr << "    binary" << std::endl;
    std::cerr << "        ( echo 1,2; echo 3,4 ) \\" << std::endl;
    std::cerr << "            | csv-to-bin 2ui \\" << std::endl;
    std::cerr << "            | ./comma-csv-sample-application-emplace --binary 2ui \\" << std::endl;
    std::cerr << "            | csv-from-bin 2ui" << std::endl;
    std::cerr << std::endl;
}

namespace comma { namespace csv { namespace examples { namespace application {

struct input
{
    struct nested { double c{0}; };
    double a{0};
    nested b;
};

} } } } // namespace comma { namespace csv { namespace examples { namespace application {

namespace comma { namespace visiting {

template <> struct traits< comma::csv::examples::application::input::nested >
{
    template < typename Key, class Visitor > static void visit( const Key&, const comma::csv::examples::application::input::nested& p, Visitor& v )
    {
        v.apply( "c", p.c );
    }

    template < typename Key, class Visitor > static void visit( const Key&, comma::csv::examples::application::input::nested& p, Visitor& v )
    {
        v.apply( "c", p.c );
    }
};

template <> struct traits< comma::csv::examples::application::input >
{
    template < typename Key, class Visitor > static void visit( const Key&, const comma::csv::examples::application::input& p, Visitor& v )
    {
        v.apply( "a", p.a );
        v.apply( "b", p.b );
    }

    template < typename Key, class Visitor > static void visit( const Key&, comma::csv::examples::application::input& p, Visitor& v )
    {
        v.apply( "a", p.a );
        v.apply( "b", p.b );
    }
};

} } // namespace comma { namespace visiting {

static comma::csv::examples::application::input updated( const comma::csv::examples::application::input& input )
{
    auto r = input;
    r.a = input.a + input.b.c;
    return r;
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        comma::csv::options csv( options );
        typedef comma::csv::examples::application::input input_t;
        if( options.exists( "--input-fields" ) ) { std::cout << comma::join( comma::csv::names< input_t >(), ',' ) << std::endl; return 0; }
        comma::csv::input_stream< comma::csv::examples::application::input > is( std::cin, csv );
        auto passed = comma::csv::make_passed( is, std::cout, csv.flush );
        while( is.ready() || std::cin.good() )
        {
            auto p = is.read();
            if( !p ) { break; }
            passed.write( updated( *p ) );
        }
        return 0;
    }
    catch( const std::exception& e ) { std::cerr << av[0] << ": " << e.what() << std::endl; }
    catch( ... ) { std::cerr << av[0] << ": unknown exception" << std::endl; }
    return 1;
}