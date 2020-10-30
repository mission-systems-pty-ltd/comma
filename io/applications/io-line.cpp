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

#include <unistd.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <string>
#include <vector>
 
#include "../../application/command_line_options.h"
#include "../../base/exception.h"

void usage( bool const verbose = false )
{
    static char const * const message =
        "\n"
        "\nA toolkit for quick access to ascii line-based input stream"
        "\n"
        "\nUsage:"
        "\n    cat lines.txt | io-line <operation> [<options>]"
        "\n"
        "\nOperations"
        "\n"
        "\n    length: output each line with a length field at the front"
        "\n        options:"
        "\n            --include-eol; add eol to length count"
        "\n"
        "\n    get: read just 1 line with a length field at the front"
        "\n"
        "\n        options"
        "\n            --to-new-line=[<chars>]; convert the given characters to new lines before output"
        "\n            --keep-length; keep the length while outputing the line"
        "\n"
        "\n    head: Output a number of lines in a pipe safe manner."
        "\n"
        "\n        options"
        "\n            --lines,-n <count>; print the first count lines instead of the first 10"
        "\n            --keep-length; keep the length while outputing the line"
        "\n"
        "\nOptions"
        "\n    --delimiter,-d [<delimiter>] : character after the line length; default: ','"
        "\n    --end-of-line,--eol [<character>]; the input and output use the given character; default: '\\n'"
        "\n    --include-eol; include end of line into the length, e.g. length of line 'xxx' would be 4"
        "\n"
        "\nExamples"
        "\n    ( echo xxx ; echo yy ; echo zzzz ) | io-line length | { echo 'line 1:' ; io-line get ; echo 'line 2:' ; io-line get ; }"
        "\n    ( echo xxx ; echo yy ; echo zzzz ) | io-line length | while true ; do echo 'read line:' ; io-line get || break ; done"
        "\n"
        "\n";
    std::exit( 0 );
}

bool read_from_stdin( char * const buffy, const ssize_t length )
{
    ssize_t total = 0, sz = 0;
    while( total < length )
    {
        sz = read( 0, buffy + total, length - total );
        if( 0 == sz ) { return false; }
        if( -1 != sz ) { total = total + sz; }
        else if( EINTR != errno ) { std::cerr << "io-line: error: could not get rest of line, had " << total << ", trying for " << length << std::endl; return false; }
    }
    return true;
}

static bool include_eol = false;
static bool keep_length = true;
static char end_of_line;
static char delimiter;
static std::string to_new_line;
static unsigned line_count;

static int length()
{
    std::string buffy; buffy.reserve( 1024 * 1024 );
    while( std::cin )
    {
        std::getline( std::cin, buffy, end_of_line );
        if( std::cin.eof() ) { return 0; }
        if( ! std::cin.good() ) { return 1; }
        std::cout
            << ( buffy.size() + ( include_eol ? 1 : 0 ) )
            << delimiter
            << buffy
            << end_of_line << std::flush;
    }
    return 0;
}

static int get()
{
    ::setvbuf( stdin, (char *)NULL, _IONBF, 0 );
    
    unsigned length;
    char ch;
    
    if( ! std::cin ) { std::cerr << "io-line: notice: stdin not good" << std::endl; return 1; }
    std::cin >> length;
    if( std::cin.eof() ) { return 1; }
    if( ! std::cin ) { std::cerr << "io-line: invalid length" << std::endl; return 1; }
    std::cin.get(ch);
    if( ! std::cin ) { std::cerr << "io-line: could not get delimiter" << std::endl; return 1; }
    if( ch != delimiter ) { std::cerr << "io-line: expected delimiter \"" << delimiter << "\"; got: \"" << ch << "\"" << std::endl; return 1; }
    if( keep_length ) { std::cout << length << delimiter ; }
    
    static std::vector< char > buffy;
    buffy.resize( length + 1 );
    buffy[length] = 0;
    if( length > 0 ) { if( ! read_from_stdin(&buffy[0], length ) ) { return 1; } }
    std::cin.get( ch );
    if( ! std::cin ) { std::cerr << "io-line: error: could not get end of line" << std::endl; return 1; }
    if( ch != end_of_line ) { std::cerr << "io-line: error: expected end of line; got: \"" << ch << "\"" << std::endl; return 1; }
    for( unsigned i = 0; i < to_new_line.size(); ++i ) { std::replace( &buffy[0], &buffy[0] + length, to_new_line[i], end_of_line ); }
    std::cout << &buffy[0];
    std::cout << end_of_line << std::flush;
    return 0;
}

static int head()
{
    for( unsigned i = 0; i < line_count; ++i )
    {
        int code = get();
        if( code != 0 ) { return code; }
    }
    return 0;
}

int main( int argc, char ** argv )
{
    try
    {
        if( argc == 1 ) { usage(); }
        comma::command_line_options options( argc, argv, usage );
        to_new_line = options.value< std::string >( "--to-new-line", "" );
        delimiter = options.value( "--delimiter,-d", ',' );
        end_of_line = options.value( "--end-of-line,--eol", '\n' );
        line_count = options.value< unsigned >( "--lines,-n", 10 );
        keep_length = options.exists( "--keep-length" );
        include_eol = options.exists( "--include-eol" );
        
        if( 0 == std::strcmp( argv[1], "length" ) ) { return length(); }
        else if( 0 == std::strcmp( argv[1], "get" ) ) { return get(); }
        else if( 0 == std::strcmp( argv[1], "head" ) ) { return head(); }
        std::cerr << "io-line: expected operation, got: \"" << argv[1] << "\"" << std::endl;
    }
    catch( std::exception& ex ) { std::cerr << "io-line: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "io-line: unknown exception" << std::endl; }
    return 1;
}
