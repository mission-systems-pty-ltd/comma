// Copyright (c) 2023 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#include <fstream>
#include <iostream>
#include <memory>
#include "../../application/command_line_options.h"
#include "../../base/exception.h"
#include "../../csv/stream.h"
#include "../../csv/traits.h"
#include "../../name_value/parser.h"
#include "../../visiting/traits.h"
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

template< typename T, std::size_t D >
struct _array { std::array< T, D > point; };

template < typename T, std::size_t D, std::size_t E >
struct lut
{
    typedef std::array< double, D > point_t;
    typedef std::array< std::size_t, D > index_t;
    typedef std::array< T, E > value_t;
    typedef comma::containers::multidimensional::grid< value_t, D, point_t > grid_t;
    typedef _array< double, D > input_t;
    typedef _array< T, D > output_t;

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

    template < typename F > static int run( const comma::csv::options& csv
                                          , const comma::csv::options& lut_csv
                                          , const std::vector< double >& origin
                                          , const std::vector< double >& resolution
                                          , const std::vector< std::size_t >& shape                                          
                                          , F&& f )
    {
        point_t o, r;
        index_t s;
        std::memcpy( &o[0], &origin[0], D * sizeof( double ) );
        std::memcpy( &r[0], &resolution[0], D * sizeof( double ) );
        std::memcpy( &s[0], &shape[0], D * sizeof( std::size_t ) );
        grid_t grid( o, r, s );
        load( grid, lut_csv );
        input_t zero;
        std::memset( &zero.point[0], 0, zero.point.size() * sizeof( T ) );
        comma::csv::input_stream< input_t > istream( std::cin, csv, zero );
        // todo! ostream and tied? or just do it by hand for now?
        while( istream.ready() || std::cin.good() )
        {
            const auto& p = istream.read();
            if( !p ) { break; }

            // todo: process
            // todo: output using tied; just a stub for debugging for now
            if( csv.binary() ) { std::cout.write( istream.binary().last(), istream.binary().size() ); }
            else { std::cout << comma::join( istream.ascii().last(), csv.delimiter ) << std::endl; }

            if( csv.flush ) { std::cout.flush(); }
        }
        return 0;
    }
};

template < typename T, std::size_t D, typename F > static int run_with_dim( const comma::csv::options& csv
                                                                          , const comma::csv::options& lut_csv
                                                                          , const std::vector< double >& origin
                                                                          , const std::vector< double >& resolution
                                                                          , const std::vector< std::size_t >& shape
                                                                          , F&& f )
{
    switch( lut_csv.format().count() )
    {
        case 1: return lut< T, D, 1 >::run( csv, lut_csv, origin, resolution, shape, f );
        case 2: return lut< T, D, 2 >::run( csv, lut_csv, origin, resolution, shape, f );
        case 3: return lut< T, D, 3 >::run( csv, lut_csv, origin, resolution, shape, f );
        case 4: return lut< T, D, 4 >::run( csv, lut_csv, origin, resolution, shape, f );
        default: COMMA_THROW( comma::exception, "up to 4-dimensional lookup table values currently supported; got: " << lut_csv.format().count() << " dimensions in " << lut_csv.format().string() );
    }
    return 1;
}

template < typename T, typename F > static int run_as( const comma::csv::options& csv
                                                     , const comma::csv::options& lut_csv
                                                     , const std::vector< double >& origin
                                                     , const std::vector< double >& resolution
                                                     , const std::vector< std::size_t >& shape
                                                     , F&& f )
{
    switch( origin.size() )
    {
        case 1: return run_with_dim< T, 1 >( csv, lut_csv, origin, resolution, shape, f );
        case 2: return run_with_dim< T, 2 >( csv, lut_csv, origin, resolution, shape, f );
        case 3: return run_with_dim< T, 3 >( csv, lut_csv, origin, resolution, shape, f );
        case 4: return run_with_dim< T, 4 >( csv, lut_csv, origin, resolution, shape, f );
        default: COMMA_THROW( comma::exception, "up to 4-dimensional lookup tables currently supported; got: " << origin.size() << " dimensions" );
    }
    return 1;
}

} } } } // namespace comma { namespace applications { namespace lookup { namespace operations {

namespace comma { namespace visiting {

template < typename T, std::size_t D > struct traits< comma::applications::lookup::operations::_array< T, D > >
{
    template < typename Key, class Visitor > static void visit( const Key&, comma::applications::lookup::operations::_array< T, D >& p, Visitor& v ) { v.apply( "point", p.point ); }
    template < typename Key, class Visitor > static void visit( const Key&, const comma::applications::lookup::operations::_array< T, D >& p, Visitor& v ) { v.apply( "point", p.point ); }
};

} } // namespace comma { namespace visiting {

namespace comma { namespace applications { namespace lookup { namespace operations {

static int interpolate( const comma::command_line_options& options, const csv::options& csv, const std::vector< std::string >& unnamed )
{
    COMMA_ASSERT_BRIEF( unnamed.size() > 1, "please specify lookup table file as: math-lookup <operation> <filename>" );
    auto lut_csv = comma::name_value::parser( "filename" ).get< comma::csv::options >( unnamed[1] );
    COMMA_ASSERT_BRIEF( csv.binary(), "lookup table: on file '" << lut_csv.filename << "': only binary files are currently supported" );
    const auto& origin = comma::split_as< double >( options.value< std::string >( "--origin,-o" ), ',' );
    const auto& resolution = comma::split_as< double >( options.value< std::string >( "--resolution,-r" ), ',' );
    const auto& shape = comma::split_as< std::size_t >( options.value< std::string >( "--shape" ), ',' );
    COMMA_ASSERT_BRIEF( origin.size() == resolution.size(), "expected --origin and --resolution of the same dimensions; got: " << origin.size() << " and " << resolution.size() );
    COMMA_ASSERT_BRIEF( origin.size() == shape.size(), "expected --origin and --shape of the same dimensions; got: " << origin.size() << " and " << shape.size() );
    switch( lut_csv.format().elements()[0].type ) // todo! quick and dirty
    {
        case comma::csv::format::float_t: return comma::applications::lookup::operations::run_as< float >( csv, lut_csv, origin, resolution, shape, nullptr );
        case comma::csv::format::double_t: return comma::applications::lookup::operations::run_as< double >( csv, lut_csv, origin, resolution, shape, nullptr );
        default: COMMA_THROW( comma::exception, "only float and double as lookup table values are supported; got: '" << unnamed[1] << "'" );
    }
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
