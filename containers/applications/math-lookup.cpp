// Copyright (c) 2023 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#include <fstream>
#include <iostream>
#include "../../application/command_line_options.h"
#include "../../base/exception.h"
#include "../../csv/stream.h"
#include "../../csv/traits.h"
#include "../../name_value/parser.h"
#include "../multidimensional/array.h"

void usage( bool verbose )
{
    std::cerr << "operations on a multidimensional lookup table" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat input.csv | math-lookup <operation> <options>" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations" << std::endl;
    std::cerr << "    interpolate: output interpolated value for the given input" << std::endl;
    std::cerr << "    nearest: todo: output table element index and value nearest to the given input" << std::endl;
    std::cerr << "    query: todo: output table element index and value for the given input" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations" << std::endl;
    std::cerr << "    interpolate" << std::endl;
    std::cerr << "        todo" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    nearest" << std::endl;
    std::cerr << "        todo" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    query" << std::endl;
    std::cerr << "        todo" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    todo" << std::endl;
    std::cerr << std::endl;
    std::cerr << "csv options" << std::endl;
    std::cerr << comma::csv::options::usage( verbose ) << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

namespace comma { namespace applications { namespace lookup { namespace operations {

template < typename T, std::size_t D, std::size_t E >
struct lut
{
    typedef std::array< double, D > point_t;
    typedef std::array< std::size_t, D > index_t;
    typedef std::array< T, E > value_t;
    typedef comma::containers::multidimensional::grid< value_t, D, point_t > grid_t;
    struct input { std::array< double, D > point; };

    static grid_t& load( grid_t& g, const comma::csv::options& csv )
    {
        std::ifstream ifs( csv.filename, std::ios::binary );
        COMMA_ASSERT_BRIEF( ifs.is_open(), "lookup table: failed to open '" << csv.filename << "'" );
        std::size_t size = g.data().size() * sizeof( T ) * E;
        ifs.read( reinterpret_cast< char* >( &g.data()[0] ), size );
        COMMA_ASSERT_BRIEF( ifs.gcount() > 0, "lookup table: failed to read from '" << csv.filename << "'" );
        COMMA_ASSERT_BRIEF( std::size_t( ifs.gcount() ) == size, "lookup table: on file '" << csv.filename << "': expected " << size << " bytes; got: " << ifs.gcount() );
        return g;
    }

    template < typename F >
    int run( const comma::csv::options& lut_csv, F&& f )
    {
        grid_t grid;
        load( grid, lut_csv );
        return 0;
    }
};

} } } } // namespace comma { namespace applications { namespace lookup { namespace operations {

namespace comma { namespace applications { namespace lookup { namespace operations {

static int interpolate( const comma::command_line_options& options, const csv::options& csv, const std::vector< std::string >& unnamed )
{
    COMMA_ASSERT_BRIEF( unnamed.size() > 1, "please specify lookup table file as: math-lookup <operation> <filename>" );
    auto lut_csv = comma::name_value::parser( "filename" ).get< comma::csv::options >( unnamed[1] );
    COMMA_ASSERT_BRIEF( csv.binary(), "lookup table: on file '" << lut_csv.filename << "': only binary files are currently supported" );
    auto input_format = csv.binary() ? csv.format() : comma::csv::format( options.value< std::string >( "--format", "d" ) );
    comma::say() << "interpolate: todo" << std::endl;
    return 1;
}

static int nearest( const comma::command_line_options& options, const csv::options& csv, const std::vector< std::string >& unnamed )
{
    comma::say() << "nearest: todo" << std::endl;
    return 1;
}

static int query( const comma::command_line_options& options, const csv::options& csv, const std::vector< std::string >& unnamed )
{
    comma::say() << "query: todo" << std::endl;
    return 1;
}

} } } } // namespace comma { namespace applications { namespace lookup { namespace operations {

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        comma::csv::options csv( options );
        const auto& unnamed = options.unnamed( "--flush,--verbose,-v", "-.*" );
        if( unnamed.empty() ) { comma::say() << "please specify operation" << std::endl; return 1; }
        std::string operation = unnamed[0];
        if( operation == "interpolate" ) { return comma::applications::lookup::operations::interpolate( options, csv, unnamed ); }
        if( operation == "nearest" ) { return comma::applications::lookup::operations::nearest( options, csv, unnamed ); }
        if( operation == "query" ) { return comma::applications::lookup::operations::query( options, csv, unnamed ); }
        comma::say() << "expected operation; got: '" << operation << "'" << std::endl;
        return 1;
    }
    catch( std::exception& ex ) { comma::say() << "caught exception: " << ex.what() << std::endl; }
    catch( ... ) { comma::say() << "caught unknown exception" << std::endl; }
    return 1;
}
