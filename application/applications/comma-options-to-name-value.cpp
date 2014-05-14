#include <iostream>
#include <comma/application/command_line_options.h>

void usage()
{
    std::cerr << std::endl;
    std::cerr << "take option description on stdin, output options to stdout as name=value pairs" << std::endl;
    std::cerr << std::endl;
    std::cerr << "a convenience for option parsing, e.g. in bash scripts" << std::endl;
    std::cerr << std::endl;
    std::cerr << "cat description.txt | comma-options-to-name-value <options>" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::command_line_options::description::usage() << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    a simple usage (try it)" << std::endl;
    std::cerr << "        echo -e \"--verbose,-v\\n--filename,-f=<value>\" | comma-options-to-name-value -v -f file.txt hello world " << std::endl;
    std::cerr << std::endl;
    std::cerr << "    setting variables in bash script (try it)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "        . $( which comma-application-util )" << std::endl;
    std::cerr << "        " << std::endl;
    std::cerr << "        #!/bin/bash" << std::endl;
    std::cerr << "        " << std::endl;
    std::cerr << "        . $( which comma-application-util )" << std::endl;
    std::cerr << "        " << std::endl;
    std::cerr << "        function description()" << std::endl;
    std::cerr << "        {" << std::endl;
    std::cerr << "            cat <<EOF" << std::endl;
    std::cerr << "        --verbose,-v; more info" << std::endl;
    std::cerr << "        --filename,-f=<value>; some filename" << std::endl;
    std::cerr << "        EOF" << std::endl;
    std::cerr << "        }" << std::endl;
    std::cerr << "        " << std::endl;
    std::cerr << "        comma_path_value_to_var < <( description | comma-options-to-name-value $@ | grep '=' )" << std::endl;
    std::cerr << "        " << std::endl;
    std::cerr << "        echo $filename" << std::endl;
    std::cerr << "        " << std::endl;
    std::cerr << "        comma_path_value_to_var --prefix=some_prefix < <( description | comma-options-to-name-value $@ | grep '=' )" << std::endl;
    std::cerr << "        " << std::endl;
    std::cerr << "        echo $some_prefix_filename" << std::endl;
    std::cerr << std::endl;
    exit( 1 );
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av );
        if( options.exists( "--help,-h" ) ) { usage(); }
        std::string valued; // quick and dirty
        std::string valueless;
        while( std::cin.good() )
        {
            std::string line;
            std::getline( std::cin, line );
            line = comma::strip( line, '\r' ); // windows... sigh...
            if( line.empty() ) { continue; }
            const comma::command_line_options::description& d = comma::command_line_options::description::from_string( line );
            d.assert_valid( options );
            std::string& s = d.has_value ? valued : valueless;
            for( unsigned int i = 0; i < d.names.size(); s += ( s.empty() ? "" : "," ) + d.names[i], ++i );
            std::string name;
            for( unsigned int i = 0; i < d.names.size(); ++i ) { if( options.exists( d.names[i] ) ) { name = d.names[i]; break; } }
            if( name.empty() ) { continue; }
            const std::string& stripped = comma::strip( comma::strip( d.names[0], '-' ), '-' );
            if( d.has_value )
            {
                const std::vector< std::string >& values = options.values< std::string >( name );
                for( unsigned int i = 0; i < values.size(); ++i ) { std::cout << stripped << "=" << values[i] << std::endl; }
            }
            else
            {
                std::cerr << stripped << "=true" << std::endl;
            }
        }
        const std::vector< std::string >& unnamed = options.unnamed( valueless, valued );
        for( unsigned int i = 0; i < unnamed.size(); ++i ) { std::cout << unnamed[i] << std::endl; }
        return 0;
    }
    catch( std::exception& ex )
    {
        std::cerr << "comma-options-to-name-value: " << ex.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << "comma-options-to-name-value: unknown exception" << std::endl;
    }
    return 1;
}
