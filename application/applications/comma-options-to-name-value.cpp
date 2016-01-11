// This file is part of comma, a generic and flexible library
// Copyright (c) 2011 The University of Sydney
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Sydney nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
// HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/// @author vsevolod vlaskine

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
    std::cerr << "lines starting with spaces, tabs, or comment signs '#' are ignored" << std::endl;
    std::cerr << std::endl;
    std::cerr << "attention: error checking does not work very well; e.g. --blah=<abc.txt> in description" << std::endl;
    std::cerr << "           will be silently omitted and --blah will be considered a valueless option" << std::endl;
    std::cerr << "           todo: make the parser less permissive" << std::endl;
    std::cerr << std::endl;
    std::cerr << "todo: currently, multiple option values are output out of order; order them" << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    a simple usage (try it):" << std::endl;
    std::cerr << "        echo -e \"--verbose,-v\\n--filename,-f=<value>\" | comma-options-to-name-value -v -f file.txt hello world " << std::endl;
    std::cerr << std::endl;
    std::cerr << "    setting variables in bash script (try it):" << std::endl;
    std::cerr << std::endl;
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
    std::cerr << "        comma_path_value_to_var < <( description | comma-options-to-name-value $@ | grep -v '^\"' )" << std::endl;
    std::cerr << "        " << std::endl;
    std::cerr << "        echo $filename" << std::endl;
    std::cerr << "        " << std::endl;
    std::cerr << "        comma_path_value_to_var --prefix=some_prefix < <( description | comma-options-to-name-value $@ | grep -v '^\"' )" << std::endl;
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
        // todo: output option values in the correct order
        //       - first read all descriptions from stdin
        //       - command_line_options: add vector of option-value pairs; expose as a method
        //       - here: put output into the map based on option number; sort before outputting
        while( std::cin.good() )
        {
            std::string line;
            std::getline( std::cin, line );
            line = comma::strip( line, '\r' ); // windows... sigh...
            if( line.empty() || line[0] == ' '  || line[0] == '\t'  || line[0] == '#' ) { continue; } // quick and dirty
            const comma::command_line_options::description& d = comma::command_line_options::description::from_string( line );
            d.assert_valid( options );
            std::string& s = d.has_value ? valued : valueless;
            for( unsigned int i = 0; i < d.names.size(); s += ( s.empty() ? "" : "," ) + d.names[i], ++i );
            std::vector< std::string > names;
            for( unsigned int i = 0; i < d.names.size(); ++i ) { if( options.exists( d.names[i] ) ) { names.push_back( d.names[i] ); } }
            if( names.empty() ) { continue; }
            const std::string& stripped = comma::strip( comma::strip( d.names[0], '-' ), '-' );
            if( d.has_value )
            {
                for( unsigned int k = 0; k < names.size(); ++k )
                {
                    const std::vector< std::string >& values = options.values< std::string >( names[k] );
                    for( unsigned int i = 0; i < values.size(); ++i ) { std::cout << stripped << "=\"" << comma::command_line_options::escaped( values[i] ) << "\"" << std::endl; }
                }
            }
            else
            {
                std::cout << stripped << "=\"1\"" << std::endl;
            }
        }
        const std::vector< std::string >& unnamed = options.unnamed( valueless, valued );
        for( unsigned int i = 0; i < unnamed.size(); ++i ) { std::cout << '"' << comma::command_line_options::escaped( unnamed[i] ) << '"' << std::endl; }
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
