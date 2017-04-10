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

#include <deque>
#include <iostream>
#include <string>
#include <boost/scoped_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "../../application/command_line_options.h"
#include "../../application/contact_info.h"
#include "../../application/signal_flag.h"
#include "../../base/types.h"
#include "../../csv/stream.h"
#include "../../io/stream.h"
#include "../../csv/traits.h"
#include "../../io/select.h"
#include "../../name_value/parser.h"
#include "../../string/string.h"
#include "../../visiting/traits.h"

static void usage()
{
    std::cerr << std::endl;
    std::cerr << "a quick utility on the popular demand:" << std::endl;
    std::cerr << "join timestamped data from stdin with corresponding" << std::endl;
    std::cerr << "timestamped data from the second input" << std::endl;
    std::cerr << std::endl;
    std::cerr << "timestamps are expected to be fully ordered" << std::endl;
    std::cerr << std::endl;
    std::cerr << "note: on windows only files are supported as bounding data" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat a.csv | csv-time-join <how> [<options>] bounding.csv [-] > joined.csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "<how>" << std::endl;
    std::cerr << "    --by-lower: join with previous filter timestamp (default)" << std::endl;
    std::cerr << "    --by-upper: join by next filter timestamp" << std::endl;
    std::cerr << "    --nearest: join by nearest filter timestamp" << std::endl;
    std::cerr << "    --stream: output immediately with current lowest filter timestamp" << std::endl;
    std::cerr << std::endl;
    std::cerr << "limitation" << std::endl;
    std::cerr << "    data with timestamp before the first and after the last bounding timestamps will be discarded at the moment; this is unwanted behaviour when using --nearest and --bound=n" << std::endl;
    std::cerr << "    workaround" << std::endl;
    std::cerr << "        use csv-time-delay to shift data timestamp and/or possibly add a bounding timestamp record with distant time in the past or future" << std::endl;
    std::cerr << std::endl;
    std::cerr << "<input/output options>" << std::endl;
    std::cerr << "    -: if csv-time-join - b.csv, concatenate output as: <stdin><b.csv>" << std::endl;
    std::cerr << "       if csv-time-join b.csv -, concatenate output as: <b.csv><stdin>" << std::endl;
    std::cerr << "       default: csv-time-join - b.csv" << std::endl;
    std::cerr << "    --binary,-b <format>: binary format" << std::endl;
    std::cerr << "    --delimiter,-d <delimiter>: ascii only; default ','" << std::endl;
    std::cerr << "    --fields,-f <fields>: input fields; default: t" << std::endl;
    std::cerr << "    --bound=<seconds>: if present, output only points inside of bound in second as double" << std::endl;
    std::cerr << "    --do-not-append,--select: if present, do not append any field from the second input" << std::endl;
    std::cerr << "    --no-discard: do not discard input points" << std::endl;
    std::cerr << "                         default: discard input points that cannot be" << std::endl;
    std::cerr << "                         consistently timestamped, especially head or tail" << std::endl;
    std::cerr << "    --timestamp-only,--time-only: join only timestamp from the second input" << std::endl;
    std::cerr << "                                  otherwise join the whole line" << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    first field on stdin is timestamp, the first field of filter is timestamp" << std::endl;
    std::cerr << "        - default:" << std::endl;
    std::cerr << "            cat a.csv | csv-time-join b.csv" << std::endl;
    std::cerr << "        - explicit:" << std::endl;
    std::cerr << "            cat a.csv | csv-time-join --fields=t \"b.csv;fields=t\"" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    3rd field on stdin is timestamp, the 2nd field of filter is timestamp" << std::endl;
    std::cerr << "        cat a.csv | csv-time-join --fields=,,t \"b.csv;fields=,t\"" << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( -1 );
}

struct input_t
{
    boost::optional<boost::posix_time::ptime> t;
    input_t() { }
    input_t( const boost::posix_time::ptime& timestamp ) : t( timestamp ) {}
};

namespace comma { namespace visiting {

template <> struct traits< input_t >
{
    template < typename K, typename V > static void visit( const K&, const input_t& p, V& v )
    { 
        v.apply( "t", p.t );
    }
    
    template < typename K, typename V > static void visit( const K&, input_t& p, V& v )
    {
        v.apply( "t", p.t );
    }
};
    
} } // namespace comma { namespace visiting {

typedef std::pair< boost::posix_time::ptime, std::string > timestring_t;

bool select_only;
bool timestamp_only;

enum join_t
{
    streaming,
    prev,
    next,
    nearest
};

join_t mode = join_t::prev;

boost::optional< double > bound;

static comma::csv::options stdin_csv;
static comma::csv::options filter_csv;

void output(const timestring_t & input, const timestring_t& filter)
{   
    if (bound)
    {
        boost::posix_time::time_duration d = input.first - filter.first;
        double seconds = abs(d.total_microseconds() / 1000000.0);
        if ( seconds > *bound ) { return; }
    }
    if (stdin_csv.binary())
    {
        std::cout.write(&input.second[0], stdin_csv.format().size());
        if (select_only) { return; }
        
        if (timestamp_only)
        {
            static const unsigned int time_size = comma::csv::format::traits< boost::posix_time::ptime, comma::csv::format::time >::size;
            static char timestamp[ time_size ];
            comma::csv::format::traits< boost::posix_time::ptime, comma::csv::format::time >::to_bin( filter.first, timestamp );
            std::cout.write( (char*)&timestamp, time_size );
        }
        else
        {
            std::cout.write(&filter.second[0], filter_csv.format().size());
        }
    }
    else
    {
        std::cout << input.second;
        if ( select_only ) { return; } 
        
        if (timestamp_only)
        {
            std::cout << stdin_csv.delimiter << boost::posix_time::to_iso_string(filter.first);
        }
        else 
        {
            std::cout << stdin_csv.delimiter << filter.second << std::endl;
        }
    }
    std::cout.flush();
}

void process_input_queue(std::deque<timestring_t>& inputs, const boost::optional<timestring_t>& prev, boost::optional<timestring_t>& next)
{
    bool output_prev = true;
    while (!inputs.empty() && (next && inputs[0].first < next->first) )
    {
        switch (mode)
        {
            case join_t::streaming:
            case join_t::prev:
                output(inputs[0], *prev);
                break;
            case join_t::next:
                output(inputs[0], *next);
                break;
            case join_t::nearest:
                if ( prev && next && (inputs[0].first - prev->first) < ( next->first - inputs[0].first) )
                { 
                    output_prev = false;
                }
                if (prev && output_prev) {output(inputs[0], *prev); break; }
                else { output(inputs[0], *next); break; }
        }
        inputs.pop_front();
    }
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av );
        comma::signal_flag is_shutdown( comma::signal_flag::hard );

        if( options.exists( "--help" ) || options.exists( "-h" ) || ac == 1 ) { usage(); }
        options.assert_mutually_exclusive( "--by-lower,--by-upper,--nearest" );
        
        if ( options.exists( "--stream" ) ) { mode = join_t::streaming; }
        else if ( options.exists( "--by-upper" ) ) { mode = join_t::next; }
        else if ( options.exists( "--nearest" ) ) { mode = join_t::nearest; }
        
        
        timestamp_only = options.exists( "--timestamp-only,--time-only" );
        select_only = options.exists( "--do-not-append,--select" );
        if( select_only && timestamp_only ) { std::cerr << "csv-time-join: --timetamp-only specified with --select, ignoring --timestamp-only" << std::endl; }
        
        boost::optional< timestring_t > prev;
        boost::optional< timestring_t > next;
        
        bool no_discard = options.exists("--no-discard");
        
        if( options.exists( "--bound" ) ) { bound = options.value< double >( "--bound" ); }
        
        stdin_csv = comma::csv::options( options, "t" );
        comma::csv::input_stream< input_t > stdin( std::cin, stdin_csv );
        
        std::vector< std::string > unnamed = options.unnamed( "-*" );
        
        if( unnamed.empty() ) { std::cerr << "csv-time-join: please specify the second source" << std::endl; return 1; }
        if( unnamed.size() > 1 ) { std::cerr << "csv-time-join: expected one file or stream to join, got " << comma::join( unnamed, ' ' ) << std::endl; return 1; }
        
        comma::name_value::parser parser( "filename" );
        filter_csv = parser.get< comma::csv::options >( unnamed[0] );
        if( filter_csv.fields.empty() ) { filter_csv.fields = "t"; }
        comma::io::istream filter_stream( comma::split( unnamed[0], ';' )[0], filter_csv.binary() ? comma::io::mode::binary : comma::io::mode::ascii );
        comma::csv::input_stream< input_t > filter( *filter_stream, filter_csv );
        std::deque<timestring_t> input_queue;
        comma::io::select select;
        select.read().add(comma::io::stdin_fd);
        select.read().add(filter_stream.fd());
        
        bool end_of_input = false;
        bool end_of_filter = false;
        boost::optional<boost::posix_time::ptime> input_time;
        boost::optional<boost::posix_time::ptime> filter_time;

        while (!is_shutdown && !end_of_input && !end_of_filter) {
            
            if ( !filter.ready() && !stdin.ready() )
            {
                select.wait(boost::posix_time::milliseconds(1));
            }
            
            if( !is_shutdown && !end_of_input && ( stdin.ready() || ( select.check() && select.read().ready( comma::io::stdin_fd ) ) ) )
            {
                // read and process input until greater than upper
                // if in "lower (streaming)" mode: output immediately
                // otherwise queue
                do
                {
                    const input_t* p = stdin.read();
                    if( !p ) 
                    { 
                        comma::verbose << "end of input stream" << std::endl;
                        end_of_input = true; 
                        break;
                    }
                    input_time = !p->t ? *p->t : boost::posix_time::microsec_clock::universal_time();
                    timestring_t input = std::make_pair(*input_time, stdin.last());
                    if ( mode == join_t::streaming )
                    {
                        // output immediately if in streaming mode
                        if (prev) { output(input, *prev); }
                        else if (no_discard) { input_queue.push_back(input); }
                        else { comma::verbose << "no lower timestamp available - discarded input" << std::endl; }
                        break;
                        
                    } else {
                        // add to input queue
                        input_queue.push_back(input);
                    }
                } while (filter_time && *input_time < *filter_time);
            }
            
            if( !is_shutdown && !end_of_filter && ( filter.ready() || ( select.check() && select.read().ready(filter_stream.fd()) ) ) )
            {
                // read filter until input queue is empty
                do
                {
                    const input_t* f = filter.read();
                    if (!f) {
                        comma::verbose << "end of filter stream" << std::endl; 
                        end_of_filter = true;
                        break;
                    }
                    // add to queue, remove
                    filter_time = !f->t ? *f->t : boost::posix_time::microsec_clock::universal_time();
                    timestring_t bounds = (std::make_pair(*filter_time, filter.last()));
                    if (!prev) { prev = bounds; }
                    else if (next) { prev = next; }
                    next = bounds;
                    
                    process_input_queue(input_queue, prev, next);
                } while (input_time && *filter_time < *input_time );
                if (end_of_input) { break; }
            }
        }
        if (is_shutdown) { std::cerr << "got a signal" << std::endl; return 0; }
        if (!input_queue.empty())
        {
            if (no_discard || mode == join_t::nearest) { process_input_queue(input_queue, prev, next); }
            else if (mode == join_t::next) { comma::verbose << "no upper timestamp - discarding inputs" << std::endl; }
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-time-join: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-time-join: unknown exception" << std::endl; }
    return 1;
}
