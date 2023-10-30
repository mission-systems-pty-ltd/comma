// Copyright (c) 2023 Vsevolod Vlaskine
// All rights reserved.

#include <exception>
#include <iostream>
#include <comma/application/command_line_options.h>
#include <comma/csv/stream.h>
#include <comma/visiting/traits.h>

static void usage( bool verbose )
{
    std::cerr << std::endl;
    std::cerr << "todo" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --input-fields" << std::endl;
    std::cerr << "    --output-fields" << std::endl;
    std::cerr << "    --output-format" << std::endl;
    std::cerr << std::endl;
    std::cerr << "csv options" << std::endl;
    std::cerr << comma::csv::options::usage( verbose ) << std::endl;
    std::cerr << std::endl;
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        
        return 0;
    }
    catch( const std::exception& e ) { std::cerr << av[0] << ": " << e.what() << std::endl; }
    catch( ... ) { std::cerr << av[0] << ": unknown exception" << std::endl; }
    return 1;
}