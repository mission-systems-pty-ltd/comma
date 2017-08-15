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
#include "../format.h"

using namespace comma;

static void usage( bool verbose=false )
{
    std::cerr << "Perform reshaping operations on input data" << std::endl;
    std::cerr << std::endl;
    std::cerr << "operations" << std::endl;
    std::cerr << "    concatenate: group input records for concatenation into output records." << std::endl;
    std::cerr << "                 the user can choose non-overlapping or overlapping grouping (sliding window) mode." << std::endl;
    std::cerr << "    loop:        group input records for concatenation into output records." << std::endl;
    std::cerr << "                 the last record loops and is joined to the first record." << std::endl;
    std::cerr << "                 this mode always uses the sliding window for overlapping groups" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Usage: cat data.csv | csv-shape <operation> [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --binary,-b=[<format>]: in binary mode: format string of the input csv data types" << std::endl;
    std::cerr << "    --delimiter,-d=[<char>]; default=','; in ascii mode, field separating character." << std::endl;
    std::cerr << "    --help,-h;  see this usage message" << std::endl;
    std::cerr << "    --verbose,-v: more output to stderr, shows examples with --help,-h" << std::endl;
    std::cerr << std::endl;
    if( verbose ) { std::cerr << comma::csv::format::usage() << std::endl; }
    std::cerr << "operations options" << std::endl;
    std::cerr << std::endl;
    std::cerr << "   concatenate" << std::endl;
    std::cerr << "      --bidirectional; output records in both directions (e.g. a,b; b,a)" << std::endl;
    std::cerr << "      --reverse; output records in reverse order (e.g. b,a)" << std::endl;
    std::cerr << "      --size,-n=<num>; number of input records in each grouping, range: 2 and above" << std::endl;
    std::cerr << "      --sliding-window,-w; use a sliding window to group input records, see examples" << std::endl;
    std::cerr << "   loop" << std::endl;
    std::cerr << "      --bidirectional; output records in both directions (e.g. a,b; b,a)" << std::endl;
    std::cerr << "      --reverse; output records in reverse order (e.g. b,a)" << std::endl;
    std::cerr << "      --size,-n=<num>; number of input records in each grouping, range: 2 and above" << std::endl;
    std::cerr << std::endl;
    if( verbose )
    {
        std::cerr << "examples" << std::endl;   
        std::cerr << "   concatenate" << std::endl;
        std::cerr << "      non overlaping groups:" << std::endl;
        std::cerr << "          concatenate each group of 5 input records into one output record." << std::endl;
        std::cerr << "          input records 1 to 5 create the first output record, input records 6-10 create the second output record, and so forth." << std::endl;
        std::cerr << "              seq 1 15 | csv-shape concatenate -n 5" << std::endl;
        std::cerr << "      overlapping groups:" << std::endl;
        std::cerr << "          move a sliding window of size 5 along the input records, every time the sliding window moves, make an output record from window" << std::endl;
        std::cerr << "          input records 1 to 5 create the first output record, input records 2 to 6 create the second record, input records 3 to 7 create the third record, and so forth" << std::endl;
        std::cerr << "              seq 1 10 | csv-shape concatenate -n 5 --sliding-window" << std::endl;
    }
    else
    {
        std::cerr << "examples: run csv-shape --help --verbose for more..." << std::endl;
    }
    exit( 0 );
}

// There is nothing to do in this case - binary data
static void simple_binary_pass_through(const comma::csv::format& f, bool flush=false)
{
    std::vector< char > buffer( f.size(), '\0' );
    while( std::cin.good() && !std::cin.eof() )
    {
        if( std::cin.read( &buffer[0], buffer.size() ) ) {
            std::cout.write( &buffer[0], buffer.size() );
            if( flush ) { std::cout.flush(); }
        }
    }
}

bool is_binary;

class concatenate_impl_
{
public:

    concatenate_impl_() 
        : use_sliding_window_(false)
        , bidirectional_(false)
        , reverse_(false)
        , looping_(false)
        , size_(0)
        , count_(0)
        , block_(0)
        {}

    int run( const comma::command_line_options& options, const comma::csv::options& csv)
    {
        std::vector< std::string > unnamed = options.unnamed( "--sliding-window,-w,--verbose,-v", "-.*" );
        std::string operation = unnamed[0];
        looping_ = ( operation == "loop");
        use_sliding_window_ = ( looping_ || options.exists("--sliding-window,-w") );
        reverse_ = options.exists("--reverse");
        bidirectional_ = options.exists("--bidirectional");
        if( !use_sliding_window_ && is_binary ) { simple_binary_pass_through(csv.format(), csv.flush); return 0; };
        size_ = options.value< comma::uint32 >("--size,-n");
        if( size_ < 2 ) { std::cerr <<  comma::verbose.app_name() << ": expected --size,-n= value to be greater than 1" << std::endl; return 1; }
        comma::csv::input_stream< input_t > istream(std::cin, csv);
        std::deque< std::string > deque;
        std::deque< std::string > first;
        bool has_block_ = csv.has_field( "block" );
        while( istream.ready() || ( std::cin.good() && !std::cin.eof() ) )
        {
            const input_t* p = istream.read();
            if( !p ) { break; }
            if ( !deque.empty() && has_block_ && (block_ != p->block) )
            {
                if (!verify(deque)) { return 1; }
                count_ = 0;
                if ( looping_ ) { output_loop(deque, first, csv); }
                deque.clear();  
            }
            block_ = p->block;
            deque.push_back( istream.last() );
            if ( looping_ && first.size() + 1  < size_ )  { first.push_back(istream.last()); }
            ++count_;
            if( deque.size() >= size_ )
            {
                output_records(deque, csv);
                if( !use_sliding_window_ ) { deque.clear(); } else { deque.pop_front(); }
            }
        }
        if ( !verify(deque)) { return 1; }
        if ( looping_ ) { output_loop(deque, first, csv); }
        return 0;
    }

    struct input_t {
        comma::uint32 block = 0;
    };

private:

    bool use_sliding_window_;
    bool bidirectional_;
    bool reverse_;
    bool looping_;
    comma::uint32 size_;
    comma::uint32 count_;
    comma::uint32 block_;

    bool verify( const std::deque<std::string>& deque )
    { 
        if( use_sliding_window_ && count_ < size_ ) 
        { 
            std::cerr << comma::verbose.app_name() << ": --size,-n= " << size_ << " is bigger than total number of input records: " << count_ << std::endl; 
            return false; 
        }
        if ( !use_sliding_window_ && !deque.empty() ) 
        { 
            std::cerr << comma::verbose.app_name() << ": error, leftover tail input record found: " << deque.size() << " lines." << std::endl; 
            return false; 
        }
        return true;
    }

    void output_records(const std::deque<std::string>& deque, const comma::csv::options& csv)
    {
        if (!reverse_)
        {
            std::cout.write( &(deque.front()[0]), deque.front().size() );
            for( auto is=(deque.cbegin()+1); is!=deque.cend(); ++is ) { if(!is_binary){ std::cout << csv.delimiter; } std::cout.write( &(*is)[0], is->size() ); }
            if(!is_binary){ std::cout << std::endl; } 
        }
        if (bidirectional_ || reverse_ ) 
        {
            std::cout.write( &(deque.back()[0]), deque.back().size() );
            for( auto is=(deque.crbegin()+1); is!=deque.crend(); ++is ) { if(!is_binary){ std::cout << csv.delimiter; } std::cout.write( &(*is)[0], is->size() ); }
            if(!is_binary){ std::cout << std::endl; } 
        }
    }

    void output_loop( std::deque<std::string>& deque, std::deque<std::string>& first, const comma::csv::options& csv)
    {
        while ( first.size() > 0 )
        {
            deque.push_back(first.front());
            output_records(deque, csv);
            deque.pop_front();
            first.pop_front();
        }
    }
};

namespace comma { namespace visiting {

template <> struct traits< concatenate_impl_::input_t >
{
    template < typename K, typename V > static void visit( const K&, const concatenate_impl_::input_t& p, V& v ) { v.apply("block", p.block); }
    template < typename K, typename V > static void visit( const K&, concatenate_impl_::input_t& p, V& v ) { v.apply("block", p.block); }
};

} } // namespace comma { namespace visiting {

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        std::vector< std::string > unnamed = options.unnamed( "--sliding-window,-w,--verbose,-v", "-.*" );
        comma::csv::options csv( options );
        if (csv.fields.empty()) { csv.fields="a"; }
        is_binary = csv.binary();
        if( unnamed.empty() ) { std::cerr << comma::verbose.app_name() << ": please specify operations" << std::endl; exit( 1 ); }
        std::string operation = unnamed[0];
        if( operation == "concatenate" || operation == "loop" )
        {
            return concatenate_impl_().run(options, csv);
        }
        std::cerr << comma::verbose.app_name() << ": operation not supported or unknown: '" << operation << '\'' << std::endl;
        return 1;
    }
    catch( std::exception& ex ) { std::cerr << "csv-shape: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-shape: unknown exception" << std::endl; }
    return 1;
}
