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

/// @author dewey nguyen

#ifdef WIN32
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#endif

#include <iostream>
#include <vector>
#include <deque>
#include "../../base/types.h"
#include "../../application/command_line_options.h"
#include "../../application/contact_info.h"
#include "../options.h"
#include "../stream.h"

using namespace comma;

static void usage( bool verbose=false )
{
    std::cerr << std::endl;
    std::cerr << "Perform reshaping operations on input data" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Usage: cat data.csv | csv-reshape <operation> [options]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "   operations" << std::endl;
    std::cerr << "      concatenate: input lines to make longer output lines, e.g. 3 csv input lines to one output line" << std::endl;
    std::cerr << "          cat file.csv | csv-reshape concatenate -n 4 --discard" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "   general" << std::endl;
    std::cerr << "      --help,-h;  see this usage message" << std::endl;
    std::cerr << "      --verbose,-v: more output to stderr, shows examples with --help,-h" << std::endl;
    std::cerr << std::endl;
    std::cerr << "   concatenate" << std::endl;
    std::cerr << "      --delimiter,-d=[<char>]; default=','; field separating character" << std::endl;
    std::cerr << "      --size,-n=<num>; number of input record to concatenate into output record" << std::endl;
    std::cerr << "      --sliding-window,-w; use a sliding window to create output record, see examples" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    if(verbose)
    {
        std::cerr << "examples:" << std::endl;
        std::cerr << "   concatenate" << std::endl;
        std::cerr << "      concatenate 4 input lines into one output line" << std::endl;
        std::cerr << "          seq 1 10 | csv-reshape concatenate -n 4" << std::endl;
        std::cerr << "      concatenate sliding window of 4 input lines into one output line" << std::endl;
        std::cerr << "          seq 1 10 | csv-reshape concatenate -n 4 --sliding-window" << std::endl;
    }
    exit( -1 );
}

struct input_t {};

namespace comma { namespace visiting {

template <> struct traits< input_t >
{
    template < typename K, typename V > static void visit( const K&, const input_t& p, V& v ) {  }
    template < typename K, typename V > static void visit( const K&, input_t& p, V& v ) {  }
};

} } // namespace comma { namespace visiting {

typedef std::deque< std::string > lines_type;

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        std::vector< std::string > unnamed = options.unnamed( "--sliding-window,-w,--discard,--help,-h,--verbose,-v", "--binary,-b,--delimiter,-d,--format,--fields,-f,--output-fields,--size,-n" );
        const comma::csv::options csv( options );
        #ifdef WIN32
        if( csv.binary() ) { _setmode( _fileno( stdin ), _O_BINARY ); _setmode( _fileno( stdout ), _O_BINARY ); }
        #endif
        if( unnamed.empty() ) { std::cerr << comma::verbose.app_name() << ": please specify operations" << std::endl; exit( 1 ); }
        std::vector< std::string > v = comma::split( unnamed[0], ',' );
        
        if( unnamed.front() == "concatenate" )
        {
            const bool use_sliding_window = options.exists("--sliding-window,-w");
            if( !use_sliding_window && csv.binary() ) { COMMA_THROW(comma::exception, " error - concatenate without sliding window expects only input csv data"); } 
            const comma::uint32 size = options.value< comma::uint32 >("--size,-n");
            if( size < 2 ) { std::cerr <<  comma::verbose.app_name() << ": expected --size,-n= value to be greater than 1" << std::endl; return 1; }
            const bool discard = options.exists("--discard");
            
            const bool is_binary = csv.binary();
            
            comma::csv::input_stream< input_t > istream(std::cin, options);
            
            lines_type lines;
            comma::uint32 count = 0;
            while( istream.ready() || ( std::cin.good() && !std::cin.eof() ) )
            {
                const input_t* p = istream.read();
                if( !p ) { break; }
                lines.push_back( istream.last() );
                ++count;
                
                if( lines.size() < size ) {}
                else if ( lines.size() > size ) { COMMA_THROW(comma::exception, "too many input lines buffered"); }
                else
                {
                    std::cout.write( &(lines.front()[0]), lines.front().size() );
                    for( lines_type::const_iterator is=(lines.cbegin()+1); is!=lines.cend(); ++is ) 
                    { 
                        if(!is_binary){ std::cout << csv.delimiter; } 
                        std::cout.write( &(*is)[0], is->size() ); 
                    }
                    if(!is_binary){ std::cout << std::endl; } 
                    
                    if( !use_sliding_window ) { lines.clear(); } else { lines.pop_front(); }
                }
            }
            
            if( use_sliding_window && count < size ) { std::cerr << comma::verbose.app_name() << "--size,-n= is bigger than total number of input records: " << count << std::endl; return 1; }
            if( !use_sliding_window && !lines.empty() ) { std::cerr << comma::verbose.app_name() << ": error, leftover tail input record found: " << lines.size() << " lines." << std::endl; return 1; }
        }
        else { std::cerr << comma::verbose.app_name() << ": operation not supported or unknown: '" << unnamed.front() << '\'' << std::endl; }

        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-reshape: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-reshape: unknown exception" << std::endl; }
    usage();
}
