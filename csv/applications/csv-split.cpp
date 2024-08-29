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
// HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
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

#ifdef WIN32
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#endif

#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include "../../csv/impl/program_options.h"
#include "../../csv/traits.h"
#include "split/split.h"

static comma::csv::options csv;
static std::vector< std::string > streams;
static boost::optional< boost::posix_time::time_duration > duration;
static std::string suffix;
static unsigned int size = 0;
static bool passthrough;
static std::string files;
static std::string default_filename;
static std::string timestamps;

template < typename T > static int run()
{
    comma::csv::applications::split< T > split( duration, suffix, csv, streams, passthrough, files, default_filename, timestamps );
    if( size == 0 )
    {
        std::string line;
        while( std::cin.good() && !std::cin.eof() )
        {
            std::getline( std::cin, line );
            if( line.empty() ) { break; }
            split.write( line );
        }
        return 0;
    }
    #ifdef WIN32
        _setmode( _fileno( stdin ), _O_BINARY );
    #endif
    std::vector< char > packet( size );
    while( std::cin.good() && !std::cin.eof() )
    {
        std::cin.read( &packet[0], size );
        if( std::cin.gcount() > 0 ) { split.write( &packet[0], size ); }
    }
    return 0;
}

int main( int argc, char** argv )
{
    try
    {
        double period = 0;
        std::string extension;
        boost::program_options::options_description description( "options" );
        description.add_options()
            ( "help,h", "display help message" )
            ( "default-file", boost::program_options::value< std::string >( &default_filename ), "todo: if --files present, unmatched ids will be put in the file with a given name; otherwise, unmatched values will be ignored" )
            ( "files", boost::program_options::value< std::string >( &files ), "if 'block' or 'id' field present, list of output files (see examples below)" )
            ( "passthrough,pass", "pass data through to stdout" )
            ( "period,t", boost::program_options::value< double >( &period ), "period in seconds after which a new file is created" )
            ( "size,c", boost::program_options::value< unsigned int >( &size ), "packet size, only full packets will be written" )
            ( "string", "id is string; default: 32-bit integer" )
            ( "suffix,s", boost::program_options::value< std::string >( &extension ), "filename extension; default will be csv or bin, depending whether it is ascii or binary" )
            ( "time", "id is time; default: 32-bit integer" )
            ( "timestamps", boost::program_options::value< std::string >( &timestamps ), "<filename>[;<csv options>]: split by timestamps (assuming both input and timestamps are in ascending order)" );
        description.add( comma::csv::program_options::description() );
        boost::program_options::variables_map vm;
        boost::program_options::store( boost::program_options::parse_command_line( argc, argv, description), vm );
        boost::program_options::parsed_options parsed = boost::program_options::command_line_parser( argc, argv ).options( description ).allow_unregistered().run();
        boost::program_options::notify( vm );
        if ( vm.count( "help" ) || vm.count( "long-help" ) )
        {
            std::cerr << R"(
read from stdin by packet or by line and split into files
files are named by field value or time (if split by time)
if splitting by id, input can also be split into streams

usage: csv-split [options] [outputs]*
)";
            std::cerr << description;
            std::cerr << R"(
data is split by one of the following fields (listed in descending precedence)
    block: split on the block number change
    id:    split by id (same as block, except does not have to be contiguous
                        with the price of worse performance)
    t:     if present, use timestamp from the packet; if absent, use system time

examples:
    --- split by block field, output to files ---
    output records for each block to a separate file
    on change of block, open a new file, e.g. 0.csv, 1.csv, etc

    with default filenames:
    ( echo 0,a; echo 1,b; echo 1,c; echo 2,d ) | csv-split --fields block

    with specified filenames:
    ( echo 0; echo 1; echo 2 ) \
        | csv-split --fields block --files <( echo a; echo b; echo c )

    with filenames mapped to block ids:
    ( echo 0; echo 1; echo 2 ) \
        | csv-split --fields block \
              --files <( echo 0,a; echo 1,b; echo 2,c )';fields=id,filename'

    --- split by id field, output to files ---
    for each id value, output records with this id to a separate file,
    e.g. 0.csv, 1.csv, etc

    with default filenames:
    ( echo 0,a; echo 1,b; echo 1,c; echo 2,d ) | csv-split --fields id

    with specified filenames:
    ( echo 0; echo 1; echo 2 ) \
        | csv-split --fields id --files <( echo a; echo b; echo c )

    with filenames mapped to block ids:
    ( echo 0; echo 1; echo 2 ) \
        | csv-split --fields id \
              --files <( echo 0,a; echo 1,b; echo 2,c )';fields=id,filename'

    --- split by t field, output to files ---
    separate records into different time periods, outputting in separate files
    ( echo 20170101T000001,a; echo 20170101T000003,b; echo 20170101T000007,c ) \
        | csv-split --fields=t --period=4

    --- split by id field, output to streams ---
    if output streams (see example below) are present on the command line and 
    id field present in --fields output records with the given ids to the
    corresponding streams, while outputing the rest into files

    records with ids for which output stream is not specified will be discarded,
    unless ... stream is specified:

    outputs: <keys>;<stream>; send records with given set of ids to this stream
        keys:
            <id>[,<id>]*: comma-separated list of ids, e.g: '5' or '2,5,7', etc
            ... (three dots): send to this stream all the records with ids
                for which no other stream is specified (see example below)
        stream:
            tcp:<port>: e.g. tcp:1234
            udp:<port>: e.g. udp:1234 (todo)
            local:<name>: linux/unix local server socket
                          e.g. local:./tmp/my_socket
            <named pipe name>: named pipe, re-opened if client reconnects
            <filename>: a regular file

        ( echo 0,a; echo 1,b; echo 0,c; echo 2,d ) \
            | csv-split --fields id "0,1;tcp:5999" "...;local:/tmp/named_fifo"
        ( echo 0,a; echo 1,b ) | csv-split --fields id --files \
                  <( echo '1,one.csv'; echo '0,zero.csv' )';fields=id,filename'

)";
            return 0;
        }
        csv = comma::csv::program_options::get( vm );
        COMMA_ASSERT_BRIEF( !vm.count( "period" ) || !vm.count( "timestamps" ), "csv-split: --period and --timestamps are mutually exclusive (todo? combine them? just ask)" );
        COMMA_ASSERT_BRIEF( default_filename.empty(), "csv-split: --default-filename: todo, just ask" )
        if( csv.binary() ) { size = csv.format().size(); }
        bool id_is_string = vm.count( "string" );
        bool id_is_time = vm.count( "time" );
        passthrough = vm.count("passthrough");
        COMMA_ASSERT_BRIEF( !id_is_string || !id_is_time, "csv-split: --string and --time are mutually exclusive" );
        if( period > 0 ) { duration = boost::posix_time::microseconds( static_cast< unsigned int >( period * 1e6 )); }
        if( extension.empty() ) { suffix = csv.binary() || size > 0 ? ".bin" : ".csv"; }
        else { suffix += "."; suffix += extension; }
        streams = boost::program_options::collect_unrecognized( parsed.options, boost::program_options::include_positional );
        COMMA_ASSERT_BRIEF( !( !streams.empty() && ( csv.has_field( "block" ) || id_is_time ) ), "publisher streams are not compatible with splitting by block or timestamp." );
        if( id_is_string ) { return run< std::string >(); }
        if( id_is_time ) { return run< boost::posix_time::ptime >(); }
        return run< comma::uint32 >();
    }
    catch( std::exception& ex ) { comma::say() << ex.what() << std::endl; }
    catch( ... ) { comma::say() << "unknown exception" << std::endl; }
    return 1;
}
