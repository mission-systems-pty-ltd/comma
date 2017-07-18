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

#include <iostream>
#include <vector>
#include <deque>
#include "../../base/types.h"
#include "../../application/command_line_options.h"
#include "../options.h"
#include "../stream.h"

using namespace comma;

// todo
//   - concatenate
//     - --help: explain the operation better
//     - support binary mode (currently throws an error for no good reason)
//     - --help: document binary mode support
//     - put input_t in a namespace for concatenate (when more operations introduced, they may need a different input type)

static void usage( bool verbose=false )
{
    std::cerr << std::endl;
    std::cerr << "Perform reshaping operations on input data" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Usage: cat data.csv | csv-reshape <operation> [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations" << std::endl;
    std::cerr << "    concatenate: input lines to make longer output lines" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --help,-h;  see this usage message" << std::endl;
    std::cerr << "    --verbose,-v: more output to stderr, shows examples with --help,-h" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations options" << std::endl;
    std::cerr << std::endl;
    std::cerr << "   concatenate" << std::endl;
    std::cerr << "      --delimiter,-d=[<char>]; default=','; field separating character" << std::endl;
    std::cerr << "      --size,-n=<num>; number of input record to concatenate into output record" << std::endl;
    std::cerr << "      --sliding-window,-w; use a sliding window to create output record, see examples" << std::endl;
    std::cerr << std::endl;
    if( verbose )
    {
        std::cerr << "examples" << std::endl;   
        std::cerr << "   concatenate" << std::endl;
        std::cerr << "      concatenate 4 input lines into one output line" << std::endl;
        std::cerr << "          seq 1 10 | csv-reshape concatenate -n 5" << std::endl;
        std::cerr << "      concatenate sliding window of 4 input lines into one output line" << std::endl;
        std::cerr << "          seq 1 10 | csv-reshape concatenate -n 5 --sliding-window" << std::endl;
    }
    else
    {
        std::cerr << "examples: run csv-reshape --help --verbose for more..." << std::endl;
    }
    exit( 0 );
}

// The input records are not inspected at all, no need
struct input_t {};

namespace comma { namespace visiting {

template <> struct traits< input_t >
{
    template < typename K, typename V > static void visit( const K&, const input_t& p, V& v ) {  }
    template < typename K, typename V > static void visit( const K&, input_t& p, V& v ) {  }
};

} } // namespace comma { namespace visiting {

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        std::vector< std::string > unnamed = options.unnamed( "--sliding-window,-w,--verbose,-v", "-.*" );
        const comma::csv::options csv( options );
        if( unnamed.empty() ) { std::cerr << comma::verbose.app_name() << ": please specify operations" << std::endl; exit( 1 ); }
        std::string operation = unnamed[0];
        if( operation == "concatenate" )
        {
            const bool use_sliding_window = options.exists("--sliding-window,-w");
            if( !use_sliding_window && csv.binary() ) { std::cerr << comma::verbose.app_name() << ": error - concatenate with binary inputs and no sliding window, nothing to be done"; return 1; } 
            const comma::uint32 size = options.value< comma::uint32 >("--size,-n");
            if( size < 2 ) { std::cerr <<  comma::verbose.app_name() << ": expected --size,-n= value to be greater than 1" << std::endl; return 1; }
            const bool is_binary = csv.binary();
            comma::csv::input_stream< input_t > istream(std::cin, csv);
            std::deque< std::string > deque;
            comma::uint32 count = 0;
            while( istream.ready() || ( std::cin.good() && !std::cin.eof() ) )
            {
                const input_t* p = istream.read();
                if( !p ) { break; }
                deque.push_back( istream.last() );
                ++count;
                if( deque.size() >= size )
                {
                    std::cout.write( &(deque.front()[0]), deque.front().size() );
                    for( auto is=(deque.cbegin()+1); is!=deque.cend(); ++is ) { if(!is_binary){ std::cout << csv.delimiter; } std::cout.write( &(*is)[0], is->size() ); }
                    if(!is_binary){ std::cout << std::endl; } 
                    if( !use_sliding_window ) { deque.clear(); } else { deque.pop_front(); }
                }
            }
            if( use_sliding_window && count < size ) { std::cerr << comma::verbose.app_name() << "--size,-n= is bigger than total number of input records: " << count << std::endl; return 1; }
            if( !use_sliding_window && !deque.empty() ) { std::cerr << comma::verbose.app_name() << ": error, leftover tail input record found: " << deque.size() << " lines." << std::endl; return 1; }
            return 0;
        }
        std::cerr << comma::verbose.app_name() << ": operation not supported or unknown: '" << operation << '\'' << std::endl;
        return 1;
    }
    catch( std::exception& ex ) { std::cerr << "csv-reshape: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-reshape: unknown exception" << std::endl; }
    return 1;
}
