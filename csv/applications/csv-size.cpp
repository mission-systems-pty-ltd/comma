// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#include <iostream>
#include "../../application/command_line_options.h"
#include "../../csv/format.h"

using namespace comma;

static void usage( bool verbose = false )
{
    std::cerr << std::endl;
    std::cerr << "a convenience utility: output to stdout size of given binary format" << std::endl;
    std::cerr << std::endl;
    std::cerr << "DEPRECATED: use csv-format size" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Usage: csv-size <format> [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --count,-c: output number of fields, rather than size" << std::endl;
    std::cerr << std::endl;
    std::cerr << "e.g.: csv-size t,d,d,d,ui will output 40" << std::endl;
    std::cerr << "      csv-size t will output 8" << std::endl;
    std::cerr << "      csv-size 2d will output 16" << std::endl;
    std::cerr << "      csv-size 2d --count will output 2" << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

int main( int ac, char** av )
{
    try
    {
        command_line_options options( ac, av, usage );
        if( ac < 2 ) { usage(); }
        comma::csv::format format( options.unnamed( "--count,-c", "" )[0] );
        std::cout << ( options.exists( "--count,-c" ) ? format.count() : format.size() ) << std::endl;
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-size: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-size: unknown exception" << std::endl; }
    return 1;
}
