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
#include <boost/lexical_cast.hpp>
#include <comma/application/contact_info.h>
#include <comma/application/command_line_options.h>
#include <comma/string/split.h>

static void usage( bool verbose = false )
{
    std::cerr << std::endl;
    std::cerr << "take csv from stdin, output path-value pairs to stdout" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: data.csv | name-value-from-csv <fields> [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    --delimiter,-d <delimiter>: default: ," << std::endl;
    std::cerr << "    --end-of-line,--eol <delimiter>: end of line output delimiter; default: end of line" << std::endl;
    std::cerr << "    --fields,-f <fields>: comma-separated field names, same as unnamed parameter for backward compatibility" << std::endl;
    std::cerr << "    --strict: don't accept lines with more fields than expected" << std::endl;
    std::cerr << "    --no-brackets: use with --line-number option above, it does not output line numbers in square brackets." << std::endl;
    std::cerr << "    --output-line-number,--line-number,-n: output line numbers (see examples)" << std::endl;
    std::cerr << "    --prefix,-p <prefix>: append this prefix to all paths" << std::endl;
    std::cerr << "    --indices=<fields>: comma-separated list of fields to use as indices (see examples)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "todo: support escaped strings (e.g. currently string values cannot have <delimiter> in them)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    echo 1,2,3 | name-value-from-csv a,b,c/d -d ," << std::endl;
    std::cerr << "    a=1,b=2,c/d=3" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    echo -e \"1,2,3\\n4,5,6\" | name-value-from-csv a,b,c/d -d , --line-number" << std::endl;
    std::cerr << "    [0]/a=1,[0]/b=2,[0]/c/d=3" << std::endl;
    std::cerr << "    [1]/a=4,[1]/b=5,[1]/c/d=6" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    echo -e \"1,2,3\\n4,5,6\" | name-value-from-csv --prefix letters/ a,b,c/d  --line-number --no-brackets" << std::endl;
    std::cerr << "    letters/0/a=1" << std::endl;
    std::cerr << "    letters/0/b=2" << std::endl;
    std::cerr << "    letters/0/c/d=3" << std::endl;
    std::cerr << "    letters/1/a=4" << std::endl;
    std::cerr << "    letters/1/b=5" << std::endl;
    std::cerr << "    letters/1/c/d=6" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    echo -e \"1,2,3\\n4,5,6\" | name-value-from-csv --indices=a,c a,b,c" << std::endl;
    std::cerr << "    a[1]/c[3]/b=2" << std::endl;
    std::cerr << "    a[4]/c[6]/b=5" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    echo -e \"1,2,3\\n4,5,6\" | name-value-from-csv --indices=a,c --line-number --no-brackets a,b,c" << std::endl;
    std::cerr << "    0/1/3/b=2" << std::endl;
    std::cerr << "    1/4/6/b=5" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    echo -e '1,\"aa\",3\\n4,\"bb\",6' | name-value-from-csv --indices=a,foo/bar/baz a,foo/bar/baz,c" << std::endl;
    std::cerr << "    a[1]/foo/bar/baz[\"aa\"]/c=3" << std::endl;
    std::cerr << "    a[4]/foo/bar/baz[\"bb\"]/c=6" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

int main( int ac, char** av )
{
    std::string line;
    try
    {
        comma::command_line_options options( ac, av, usage );
        char delimiter = options.value< char >( "--delimiter,-d", ',' );
        char end_of_line = options.value< char >( "--end-of-line,--eol", '\n' );
        std::string fields = options.value< std::string >( "--fields,-f", "" );
        bool strict = options.exists( "--strict" );
        bool no_brackets = options.exists( "--no-brackets" );
        bool output_line_numbers = options.exists( "--output-line-number,--line-number,-n" );
        std::string prefix = options.value< std::string >( "--prefix,-p", "" );
        if( !prefix.empty() && ( no_brackets || !output_line_numbers ) ) { prefix += '/'; }
        std::string left_bracket, right_bracket;
        if( !no_brackets ) { left_bracket = "["; right_bracket = "]"; }
        
        if( fields.empty() )
        { 
            const std::vector< std::string >& unnamed = options.unnamed( "--strict,--no-brackets,--output-line-number,--line-number,-n,--indices", "-.*" );
            if( unnamed.empty() || unnamed[0].empty() ) { std::cerr << "name-value-from-csv: please specify fields" << std::endl; return 1; }
            fields = unnamed[0];
        }
        const std::vector< std::string >& paths = comma::split( fields, ',' );
        std::vector< unsigned int > indices;
        if ( options.exists( "--indices" ) ) {
            const std::vector< std::string > & index_names = comma::split( options.value< std::string >( "--indices", "" ), delimiter );
            for ( unsigned int i = 0; i < index_names.size(); ++i ) {
                if ( index_names[i].empty() ) continue;
                std::vector< std::string >::const_iterator position = std::find( paths.begin(), paths.end(), index_names[i] );
                if ( position == paths.end() ) { std::cerr << "name-value-from-csv: index '" << index_names[i] << "' not in fields list" << std::endl; return 1; }
                indices.push_back( position - paths.begin() );
            }
        }
        for( unsigned int i = 0; std::cin.good() && !std::cin.eof(); ++i )
        {
            std::getline( std::cin, line );
            if( line.empty() || line[0] == '\r' ) { continue; } // quick and dirty: windows...
            //const std::vector< std::string >& values = comma::split_escaped( line, delimiter );
            const std::vector< std::string >& values = comma::split( line, delimiter );
            std::string index = output_line_numbers ? left_bracket + boost::lexical_cast< std::string >( i ) + right_bracket + "/" : "";
            if ( !indices.empty() ) {
                for ( unsigned int j = 0; j < indices.size(); ++j ) {
                    if ( indices[j] >= values.size() ) { std::cerr << "name-value-from-csv: line " << i << ": no value for index '" << paths[ indices[j] ] << "'" << std::endl; return 1; }
                    index += ( no_brackets ? values[indices[j]] : paths[ indices[j] ] + "[" + values[indices[j]] + "]" ) + "/";
                }
            }
            for( unsigned int k = 0; k < values.size(); ++k )
            {
                bool overshot = k >= paths.size();
                if( overshot && strict ) { std::cerr << "name-value-from-csv: line " << i << ": expected not more than " << paths.size() << " value[s], got " << values.size() << std::endl; return 1; }
                if( overshot || paths[k].empty() ) { continue; }
                if ( std::find( indices.begin(), indices.end(), k ) != indices.end() ) continue;
                std::cout << prefix << index << paths[k] << "=\"" << comma::strip( comma::strip( values[k], '"' ), ' ' ) << "\"" << end_of_line;
                std::cout.flush();
            }
        }
        return 0;
    }
    catch( std::exception& ex )
    { 
        std::cerr << "name-value-from-csv: " << ex.what() << std::endl;
    }
    catch( ... )
    { 
        std::cerr << "name-value-from-csv: unknown exception" << std::endl;
    }
    return 1;
}
