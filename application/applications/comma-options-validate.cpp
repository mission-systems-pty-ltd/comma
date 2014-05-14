#include <iostream>
#include <comma/application/command_line_options.h>

void usage()
{
    std::cerr << std::endl;
    std::cerr << "validate command line options against option description" << std::endl;
    std::cerr << std::endl;
    std::cerr << "returns 0, if options are valid, 1 otherwise" << std::endl;
    std::cerr << std::endl;
    std::cerr << "a convenience for option parsing, e.g. in bash scripts" << std::endl;
    std::cerr << std::endl;
    std::cerr << "cat description.txt | comma-options-to-name-value <options>" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::command_line_options::description::usage() << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples (try them)" << std::endl;
    std::cerr << "    valid options:" << std::endl;
    std::cerr << "        echo '--verbose,-v' | comma-options-validate -v hello world" << std::endl;
    std::cerr << "    invalid options:" << std::endl;
    std::cerr << "        echo '--verbose,-v' | comma-options-validate -v hello world --blah" << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av );
        if( options.exists( "--help,-h" ) ) { usage(); }
        std::vector< comma::command_line_options::description > descriptions;
        while( std::cin.good() )
        {
            std::string line;
            std::getline( std::cin, line );
            line = comma::strip( line, '\r' ); // windows... sigh...
            if( line.empty() ) { continue; }
            descriptions.push_back( comma::command_line_options::description::from_string( line ) );
        }
        options.assert_valid( descriptions, true );
        return 0;
    }
    catch( std::exception& ex )
    {
        std::cerr << "comma-options-validate: " << ex.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << "comma-options-validate: unknown exception" << std::endl;
    }
    return 1;
}
