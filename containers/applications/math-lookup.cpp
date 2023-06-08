// Copyright (c) 2023 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#include <iostream>
#include "../../application/command_line_options.h"
#include "../../csv/stream.h"

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

static int interpolate( const comma::command_line_options& options, const csv::options& csv, const std::vector< std::string >& unnamed )
{
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
