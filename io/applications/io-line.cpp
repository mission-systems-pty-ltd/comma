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

/// @author mathew hounsell

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

#include <unistd.h>
 
#include <comma/application/contact_info.h>
#include <comma/application/command_line_options.h>
#include <comma/base/exception.h>

void usage( bool const verbose = false )
{
    static char const * const message =
        "\n"
        "\nAccess an I/O stream in a fast line based manner."
        "\n"
        "\nUsage:"
        "\n    source | io-line <action> <options> | destination"
        "\n"
        "\nActions:"
        "\n    length"
        "\n        - Output each line with a length field at the front"
        "\n    get"
        "\n        - Read just 1 line with a length field at the front in a pipe safe manner."
        "\n        - Other tools like head -1 read too much from the pipe to be "
        "\n"
        "\nOptions:"
        "\n    --delimiter,-d <delimiter> : default: ','"
        "\n"
        "\nSpecial Options:"
        "\n    --to-new-line=[<chars>]; convert the given characters to new lines before output"
        "\n         Works with get"
        "\n"
        "\nExamples:"
        "\n    cat text | io-line length | { io-line get ; echo @ ; io-line get ; }"
        "\n    "
        "\n"
        "\n";
    std::cerr << message << comma::contact_info << '\n' << std::endl;
    std::exit( 0 );
}

bool read_from_stdin( char * const buffy, const ssize_t length )
{
    ssize_t total = 0, sz = 0;
    while( total < length )
    {
        sz = read( 0, buffy + total, length - total );
        if( -1 != sz ) { total = total + sz; }
        else if( EINTR != errno ) { return false; }
    }
    return true;
}

static std::string delimiter;
static std::string to_new_line;

int length( void )
{
    std::string buffy; buffy.reserve( 1024 * 1024 * 1024 );
    
    while( std::cin )
    {
        std::getline( std::cin, buffy );
        if( ! std::cin ) return 0;
        std::cout
            // << std::setfill('0') << std::setw(8) 
            <<  buffy.size() << delimiter[0] << buffy
            << std::endl;
    }
    return 0;
}

int get( void )
{
    ::setvbuf( stdin, (char *)NULL, _IONBF, 0 );
    
    unsigned length;
    char ch;
    
    if( ! std::cin ) { std::cerr << "io-line: notice: input stream was not good at start" << std::endl; return 0; }
    std::cin >> length;
    if( ! std::cin ) { std::cerr << "io-line: notice: could not get a length" << std::endl; return 0; }
    std::cin.get(ch);
    if( ! std::cin ) { std::cerr << "io-line: error: could not get delimiter" << std::endl; return 1; }
    if( ch != delimiter[0] ) { std::cerr << "io-line: error: delimiter was incorrect " << ch << std::endl; return 1; }
    
    char * const buffy = new char[length + 1];
    
    if( length > 0 )
    {
        buffy[length] = 0;
        if( ! read_from_stdin(buffy, length ) ) { std::cerr << "io-line: error: could not get rest of line - tried for " << length << std::endl; return 1; }
    }
    
    std::cin.get(ch);
    if( ! std::cin ) { std::cerr << "io-line: error: could not get end of line" << std::endl; return 1; }
    if( ch != '\n' ) { std::cerr << "io-line: error: end of line was incorrect " << ch << std::endl; return 1; }
    
    for( unsigned i = 0; i < to_new_line.size(); ++i ) { std::replace( buffy, buffy + length, to_new_line[i], '\n' ); }
    
    if( length > 0 ) { std::cout << buffy; }
    std::cout << std::endl;
    return 0;
}

int main( int argc, char ** argv )
{
    try
    {
        if( argc == 1 ) { usage() ; return 1; }

        comma::command_line_options options( argc, argv, usage );
        to_new_line = options.value< std::string >( "--to-new-line", "" );
        delimiter = options.value< std::string >( "--delimiter,-d", "," );
        
        if( delimiter.size() != 1 ) { std::cerr << "io-line: error: delimiter must be a single character '" << delimiter << '\'' << std::endl; return 1; }
        
        if( 0 == std::strcmp( argv[1], "length" ) ) { return length(); }
        else if( 0 == std::strcmp( argv[1], "get" ) )  { return get(); }
        
        usage();
    }
    catch( std::exception& ex ) { std::cerr << "io-line: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "io-line: unknown exception" << std::endl; }
    return 1;
}
