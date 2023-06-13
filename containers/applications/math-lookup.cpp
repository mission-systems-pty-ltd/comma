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
    std::cerr << "up to 4-dimension lookup tables with up to 4-dimension" << std::endl;
    std::cerr << "values are currently supported; if you need more, just ask" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat input.csv | math-lookup <operation> [<lut-filename>[;<lut-csv-options>]] <options>" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations" << std::endl;
    std::cerr << "    index: todo: output index for a given input" << std::endl;
    std::cerr << "    interpolate: todo: output interpolated value for the given input" << std::endl;
    std::cerr << "    nearest: todo: output table element index and value nearest to the given input" << std::endl;
    std::cerr << "    query: todo: output table element index and value for the given input" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --origin,-o=<point>; e.g: --origin=0,1,2,3" << std::endl;
    std::cerr << "    --resolution,-r=<point>; e.g: --resolution=0.5,3,2,3" << std::endl;
    std::cerr << "    --shape=<point>; e.g: --shape=3,2,5,3, same as in numpy" << std::endl;
    std::cerr << "                     i.e. shape[0] is the slowest-changing" << std::endl;
    std::cerr << "                     i.e. expected lookup table memory layout" << std::endl;
    std::cerr << "                     is rows first" << std::endl;
    std::cerr << std::endl;
    std::cerr << "input/output options" << std::endl;
    std::cerr << "    --input-fields; todo: print input fields for an operation to stdout and exit" << std::endl;
    std::cerr << "    --output-fields; todo: print output fields for an operation to stdout and exit" << std::endl;
    std::cerr << "    --output-format; todo: print output format for an operation to stdout and exit" << std::endl;
    std::cerr << std::endl;
    std::cerr << "csv options" << std::endl;
    std::cerr << comma::csv::options::usage( verbose ) << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

// todo
// - 1-dimensional: fix
// - array operators: fix
// - nearest: fix
// ! --help
// - regression test: basics

template< typename T, std::size_t D, typename S >
std::array< T, D >& operator*=( std::array< T, D >& lhs, const S& rhs ) { COMMA_THROW( comma::exception, "todo" ); }

template< typename T, std::size_t D, typename S >
std::array< T, D > operator*( const std::array< T, D >& lhs, const S& rhs ) { COMMA_THROW( comma::exception, "todo" ); }

template< typename T, std::size_t D >
std::array< T, D >& operator+=( std::array< T, D >& lhs, const std::array< T, D >& rhs ) { COMMA_THROW( comma::exception, "todo" ); }

template< typename T, std::size_t D >
std::array< T, D > operator+( const std::array< T, D >& lhs, const std::array< T, D >& rhs ) { COMMA_THROW( comma::exception, "todo" ); }

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

    static std::pair< index_t, value_t > interpolate( const grid_t& g, const point_t& p )
    {
        std::pair< index_t, value_t > r;
        r.first = g.index_of( p );
        r.second = g.interpolated( p );
        return r;
    }

    static std::pair< index_t, value_t > nearest( const grid_t& g, const point_t& p )
    {
        std::pair< index_t, value_t > r;
        r.first = g.nearest_to( p );
        r.second = g[r.first];
        return r;
    }

    static std::pair< index_t, value_t > query( const grid_t& g, const point_t& p )
    {
        std::pair< index_t, value_t > r;
        r.first = g.index_of( p );
        r.second = g[r.first];
        return r;
    }

    static int run( const std::string& operation
                  , const comma::csv::options& csv
                  , const comma::csv::options& lut_csv
                  , const std::vector< double >& origin
                  , const std::vector< double >& resolution
                  , const std::vector< std::size_t >& shape )
    {
        std::pair< index_t, value_t > ( *f )( const grid_t&, const point_t& );
        if( operation == "interpolate" ) { f = lut< T, D, E >::interpolate; }
        //else if( operation == "nearest" ) { f = lut< T, D, E >::nearest; }
        else if( operation == "query" ) { f = lut< T, D, E >::query; }
        else { COMMA_THROW_BRIEF( comma::exception, "expected operation; got: '" << operation << "'" ); }
        point_t o, r;
        index_t s;
        std::memcpy( &o[0], &origin[0], D * sizeof( double ) ); // quick and dirty
        std::memcpy( &r[0], &resolution[0], D * sizeof( double ) ); // quick and dirty
        std::memcpy( &s[0], &shape[0], D * sizeof( std::size_t ) ); // quick and dirty
        grid_t grid( o, r, s );
        load( grid, lut_csv );
        input_t zero;
        std::memset( &zero.point[0], 0, zero.point.size() * sizeof( T ) );
        comma::csv::input_stream< input_t > istream( std::cin, csv, zero );
        comma::csv::output_stream< std::pair< index_t, value_t > > ostream( std::cout, csv.binary() );
        auto tied = comma::csv::make_tied( istream, ostream );
        while( istream.ready() || std::cin.good() )
        {
            const auto& p = istream.read();
            if( !p ) { break; }
            tied.append( f( grid, p->point ) );
            if( csv.flush ) { std::cout.flush(); }
        }
        return 0;
    }
};

template < typename T, std::size_t D > static int run_with_dim( const std::string& operation
                                                              , const comma::csv::options& csv
                                                              , const comma::csv::options& lut_csv
                                                              , const std::vector< double >& origin
                                                              , const std::vector< double >& resolution
                                                              , const std::vector< std::size_t >& shape )
{
    switch( lut_csv.format().count() )
    {
        case 1: return lut< T, D, 1 >::run( operation, csv, lut_csv, origin, resolution, shape );
        case 2: return lut< T, D, 2 >::run( operation, csv, lut_csv, origin, resolution, shape );
        case 3: return lut< T, D, 3 >::run( operation, csv, lut_csv, origin, resolution, shape );
        case 4: return lut< T, D, 4 >::run( operation, csv, lut_csv, origin, resolution, shape );
        default: COMMA_THROW( comma::exception, "up to 4-dimensional lookup table values currently supported; got: " << lut_csv.format().count() << " dimensions in " << lut_csv.format().string() );
    }
    return 1;
}

template < typename T > static int run_as( const std::string& operation
                                         , const comma::csv::options& csv
                                         , const comma::csv::options& lut_csv
                                         , const std::vector< double >& origin
                                         , const std::vector< double >& resolution
                                         , const std::vector< std::size_t >& shape )
{
    switch( origin.size() )
    {
        // todo! case 1: return run_with_dim< T, 1 >( operation, csv, lut_csv, origin, resolution, shape );
        case 2: return run_with_dim< T, 2 >( operation, csv, lut_csv, origin, resolution, shape );
        case 3: return run_with_dim< T, 3 >( operation, csv, lut_csv, origin, resolution, shape );
        case 4: return run_with_dim< T, 4 >( operation, csv, lut_csv, origin, resolution, shape );
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

static int run( const comma::command_line_options& options, const csv::options& csv, const std::vector< std::string >& unnamed )
{
    COMMA_ASSERT_BRIEF( unnamed.size() > 1, "please specify lookup table file as: math-lookup <operation> <filename>" );
    auto lut_csv = comma::name_value::parser( "filename" ).get< comma::csv::options >( unnamed[1] );
    COMMA_ASSERT_BRIEF( csv.binary(), "lookup table: on file '" << lut_csv.filename << "': only binary files are currently supported, e.g: 'lut.bin;binary=3f'" );
    const auto& origin = comma::split_as< double >( options.value< std::string >( "--origin,-o" ), ',' );
    const auto& resolution = comma::split_as< double >( options.value< std::string >( "--resolution,-r" ), ',' );
    const auto& shape = comma::split_as< std::size_t >( options.value< std::string >( "--shape" ), ',' );
    COMMA_ASSERT_BRIEF( origin.size() == resolution.size(), "expected --origin and --resolution of the same dimensions; got: " << origin.size() << " and " << resolution.size() );
    COMMA_ASSERT_BRIEF( origin.size() == shape.size(), "expected --origin and --shape of the same dimensions; got: " << origin.size() << " and " << shape.size() );
    switch( lut_csv.format().elements()[0].type ) // todo! quick and dirty
    {
        case comma::csv::format::float_t: return comma::applications::lookup::operations::run_as< float >( unnamed[0], csv, lut_csv, origin, resolution, shape );
        case comma::csv::format::double_t: return comma::applications::lookup::operations::run_as< double >( unnamed[0], csv, lut_csv, origin, resolution, shape );
        default: COMMA_THROW( comma::exception, "only float and double as lookup table values are supported; got: '" << unnamed[1] << "'" );
    }
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
        return comma::applications::lookup::operations::run( options, csv, unnamed );
    }
    catch( std::exception& ex ) { comma::say() << "caught exception: " << ex.what() << std::endl; }
    catch( ... ) { comma::say() << "caught unknown exception" << std::endl; }
    return 1;
}
