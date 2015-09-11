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

#include <iostream>
#include <cstdio>   // for popen()
#include <cstdlib>
#include <string>
#include <cstring>  // for strerror()
#include <cerrno>   // for errno
#include <iostream>
#include <boost/array.hpp>
#include <boost/optional.hpp>
#include "../../application/command_line_options.h"
#include "../../io/select.h"
#include "../../io/stream.h"

static const char *app_name = "io-tee";

static void show_usage()
{
    std::cerr << "Usage: " << app_name << " <output file> [options ... --] <command ...>\n";
}

static void show_help( bool verbose = false )
{
    show_usage();
    std::cerr
        << std::endl
        << "Similar to \"tee >( command ... > file )\" in bash, but ensures that the command is complete before continuing." << std::endl
        << std::endl
        << "Options:" << std::endl
        << "    --dry-run,--dry: print command that will be piped and exit, debug option" << std::endl
        << "    --unbuffered,-u: unbuffered input and output" << std::endl
        << "    --verbose,-v: more output" << std::endl
        << std::endl
        << "Note that only single commands are supported; to run multiple commands (or a pipeline), put them inside a bash function:" << std::endl
        << "*** IMPORTANT *** use \"export -f function_name\" to make the function visible to " << app_name << "." << std::endl
        << "Remember that " << app_name << " will not have access to the unexported variables, so pass any required values as function arguments." << std::endl
        << std::endl
        << "If any options are used (such as --unbuffered), \"--\" must precede the command." << std::endl
        << std::endl
        << "A note about using \"grep\": be aware grep returns 1 if the pattern is not found, which will make " << app_name << " think the command failed." << std::endl
        << "To avoid this, call grep inside a function like this: grep (pattern) || true." << std::endl
        << std::endl
        << "Example 1:" << std::endl
        << "    ( echo one two ; echo three ) | io-tee outfile grep \"one two\" | grep \"three\" > outfile2" << std::endl
        << std::endl
        << "    - which is equivalent to:" << std::endl
        << std::endl
        << "    ( echo one two ; echo three ) | grep \"one two\" > outfile" << std::endl
        << "    ( echo one two ; echo three ) | grep \"three\" > outfile2" << std::endl
        << std::endl
        << "Example 2:" << std::endl
        << "    function do_something() { local pattern=$1; grep \"$pattern\"; }" << std::endl
        << "    export -f do_something" << std::endl
        << "    ( echo one two ; echo three ) | io-tee outfile do_something \"one two\" | grep \"three\" > outfile2" << std::endl
        << std::endl
        << "    - exactly the same as Example 1, but using a function (note the \"export -f\")." << std::endl
        << std::endl;
}

static bool file_is_writable( const std::string &filename )
{
    FILE *file = fopen( &filename[0], "w" );
    if ( file == NULL ) { return false; }
    fclose( file );
    return true;
}

static std::string escape_quotes( const char *s )
{
    std::string result;
    for ( ; *s; ++s )
    {
        if ( *s == '"' ) { result += "\""; }          //if ( *s == '"' ) { result += "\\\""; }          // double quote becomes \"
        else if ( *s == '\'' ) { result += "'\\''"; }   // single quote becomes '\'' (since the whole command is single quoted)
        else { result += *s; }
    }
    return result;
}

int main( int ac, char **av )
{
    FILE *pipe = NULL;
    try
    {
        if( ac < 3 )
        {
            comma::command_line_options options( ac, av );
            if( options.exists( "--help,-h" ) ) { show_help(); return 0; }
            else { show_usage(); return 1; }
        }
        unsigned int command_offset = 2;
        for( int i = 1; i < ac; ++i ) { if( av[i] == std::string( "--" ) ) { command_offset = i + 1; break; } }
        if( command_offset == ( unsigned int )( ac ) ) { show_usage(); return 1; }
        comma::command_line_options options( command_offset > 2 ? command_offset - 1 : 2, av, show_help );
        const std::vector< std::string >& unnamed = options.unnamed( "--unbuffered,-u,--verbose,-v,--dry-run,--dry", "-.*" );
        if( unnamed.empty() ) { std::cerr << "io-tee: please specify output file name" << std::endl; return 1; }
        if( unnamed.size() > 1 ) { std::cerr << "io-tee: expected one output filename, got: " << comma::join( unnamed, ' ' ) << std::endl; return 1; }
        std::string outfile = unnamed[0];
        // bash -c only takes a single argument, so put the whole command in single quotes, then double quote each individual argument
        std::string command = "bash -c '" + escape_quotes( av[command_offset] );
        for( int i = command_offset + 1; i < ac; ++i ) { command += " \""; command += escape_quotes( av[i] ); command += "\""; }
        command += " > ";
        command += outfile;
        command += "'";
        bool unbuffered = options.exists( "--unbuffered,-u" );
        bool verbose = options.exists( "--verbose,-v" );
        if( !file_is_writable( outfile ) ) { std::cerr << app_name << ": cannot write to " << outfile << std::endl; exit( 1 ); }
        if( options.exists( "--dry-run,--dry" ) ) { std::cout << command << std::endl; return 0; }
        if( verbose ) { std::cerr << app_name << ": will run command: " << command << std::endl; }
        pipe = ::popen( &command[0], "w" );
        if( pipe == NULL ) { std::cerr << app_name << ": failed to open pipe; command: " << command << std::endl; return 1; }
        boost::array< char, 0xffff > buffer;
        comma::io::select stdin_select;
        if( unbuffered ) { stdin_select.read().add( 0 ); }
        comma::io::istream is( "-", comma::io::mode::binary );
        while( std::cin.good() )
        {
            std::size_t bytes_to_read = buffer.size();
            if( unbuffered )
            {
                if( stdin_select.wait( boost::posix_time::seconds( 1 ) ) == 0 ) { continue; }
                std::size_t available = is.available();
                bytes_to_read = std::min( available, buffer.size() );
            }
            std::cin.read( &buffer[0], bytes_to_read );
            if( std::cin.gcount() <= 0 ) { break; }
            std::cout.write( &buffer[0], std::size_t( std::cin.gcount() ) );
            //int r = ::fwrite( pipe, &buffer[0], std::size_t( std::cin.gcount() ) );
            int r = ::fwrite( &buffer[0], sizeof( char ), ::size_t( std::cin.gcount() ), pipe );
            if( r < 0 ) // TODO: check what ::write actually returns
            { 
                std::cerr << app_name << ": error on pipe: error " << r << std::endl; // TODO: output text for the error
                ::pclose( pipe );
                return 1;
            }
            if( unbuffered )
            { 
                std::cout.flush();
                ::fflush( pipe );
            }
        }
        int result = ::pclose( pipe );
        if ( result == -1 ) { std::cerr << app_name << ": pipe error: " << std::strerror( errno ) << "; command was: " << command << std::endl; exit( 1 ); }
        else if ( result != 0 ) { std::cerr << app_name << ": command failed: " << command << std::endl; return 1; }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << app_name << ": " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << app_name << ": unknown exception" << std::endl; }
    if( pipe ) { ::pclose( pipe ); }
    return 1;
}

