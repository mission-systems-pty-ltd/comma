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

template < typename T > static void run()
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
    }
    else
    {
        #ifdef WIN32
            _setmode( _fileno( stdin ), _O_BINARY );
        #endif
        std::vector< char > packet( size );
        while( std::cin.good() && !std::cin.eof() )
        {
            std::cin.read( &packet[0], size );
            if( std::cin.gcount() > 0 ) { split.write( &packet[0], size ); }
        }
    }
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
            std::cerr << "\n";
            std::cerr << "read from stdin by packet or by line and split into files\n";
            std::cerr << "files are named by field value or time (if split by time)\n";
            std::cerr << "if splitting by id, input can also be split into streams\n";
            std::cerr << "\n";
            std::cerr << "usage: csv-split [options] [outputs]*\n";
            std::cerr << "\n";
            std::cerr << description;
            std::cerr << "\n";
            std::cerr << "data is split by one of the following fields (listed in descending precedence)\n";
            std::cerr << "    block: split on the block number change\n";
            std::cerr << "    id:    split by id (same as block, except does not have to be contiguous\n";
            std::cerr << "                        with the price of worse performance)\n";
            std::cerr << "    t:     if present, use timestamp from the packet; if absent, use system time\n";
            std::cerr << "\n";
            std::cerr << "examples:\n";
            std::cerr << "    --- split by block field, output to files ---\n";
            std::cerr << "    output records for each block to a separate file\n";
            std::cerr << "    on change of block, open a new file, e.g. 0.csv, 1.csv, etc\n";
            std::cerr << "\n";
            std::cerr << "    with default filenames:\n";
            std::cerr << "    ( echo 0,a; echo 1,b; echo 1,c; echo 2,d ) | csv-split --fields block\n";
            std::cerr << "\n";
            std::cerr << "    with specified filenames:\n";
            std::cerr << "    ( echo 0; echo 1; echo 2 ) \\\n";
            std::cerr << "        | csv-split --fields block --files <( echo a; echo b; echo c )\n";
            std::cerr << "\n";
            std::cerr << "    with filenames mapped to block ids:\n";
            std::cerr << "    ( echo 0; echo 1; echo 2 ) \\\n";
            std::cerr << "        | csv-split --fields block \\\n";
            std::cerr << "              --files <( echo 0,a; echo 1,b; echo 2,c )';fields=id,filename'\n";
            std::cerr << "\n";
            std::cerr << "    --- split by id field, output to files ---\n";
            std::cerr << "    for each id value, output records with this id to a separate file,\n";
            std::cerr << "    e.g. 0.csv, 1.csv, etc\n";
            std::cerr << "\n";
            std::cerr << "    with default filenames:\n";
            std::cerr << "    ( echo 0,a; echo 1,b; echo 1,c; echo 2,d ) | csv-split --fields id\n";
            std::cerr << "\n";
            std::cerr << "    with specified filenames:\n";
            std::cerr << "    ( echo 0; echo 1; echo 2 ) \\\n";
            std::cerr << "        | csv-split --fields id --files <( echo a; echo b; echo c )\n";
            std::cerr << "\n";
            std::cerr << "    with filenames mapped to block ids:\n";
            std::cerr << "    ( echo 0; echo 1; echo 2 ) \\\n";
            std::cerr << "        | csv-split --fields id \\\n";
            std::cerr << "              --files <( echo 0,a; echo 1,b; echo 2,c )';fields=id,filename'\n";
            std::cerr << "\n";
            std::cerr << "    --- split by t field, output to files ---\n";
            std::cerr << "    separate records into different time periods, outputting in separate files\n";
            std::cerr << "    ( echo 20170101T000001,a; echo 20170101T000003,b; echo 20170101T000007,c ) \\\n";
            std::cerr << "        | csv-split --fields=t --period=4\n";
            std::cerr << "\n";
            std::cerr << "    --- split by id field, output to streams ---\n";
            std::cerr << "    if output streams (see example below) are present on the command line and \n";
            std::cerr << "    id field present in --fields output records with the given ids to the\n";
            std::cerr << "    corresponding streams, while outputing the rest into files\n";
            std::cerr << "\n";
            std::cerr << "    records with ids for which output stream is not specified will be discarded,\n";
            std::cerr << "    unless ... stream is specified:\n";
            std::cerr << "\n";
            std::cerr << "    outputs: <keys>;<stream>; send records with given set of ids to this stream\n";
            std::cerr << "        keys:\n";
            std::cerr << "            <id>[,<id>]*: comma-separated list of ids, e.g: '5' or '2,5,7', etc\n";
            std::cerr << "            ... (three dots): send to this stream all the records with ids\n";
            std::cerr << "                for which no other stream is specified (see example below)\n";
            std::cerr << "        stream:\n";
            std::cerr << "            tcp:<port>: e.g. tcp:1234\n";
            std::cerr << "            udp:<port>: e.g. udp:1234 (todo)\n";
            std::cerr << "            local:<name>: linux/unix local server socket\n";
            std::cerr << "                          e.g. local:./tmp/my_socket\n";
            std::cerr << "            <named pipe name>: named pipe, re-opened if client reconnects\n";
            std::cerr << "            <filename>: a regular file\n";
            std::cerr << "\n";
            std::cerr << "        ( echo 0,a; echo 1,b; echo 0,c; echo 2,d ) \\\n";
            std::cerr << "            | csv-split --fields id \"0,1;tcp:5999\" \"...;local:/tmp/named_fifo\"\n";
            std::cerr << "        ( echo 0,a; echo 1,b ) | csv-split --fields id --files \\\n";
            std::cerr << "                  <( echo '1,one.csv'; echo '0,zero.csv' )';fields=id,filename'\n";
            std::cerr << std::endl;
            return 0;
        }
        csv = comma::csv::program_options::get( vm );
        if( vm.count( "period" ) && vm.count( "timestamps" ) ) { std::cerr << "csv-split: --period and --timestamps are mutually exclusive (todo? combine them? just ask)" << std::endl; return 1; }
        if( !default_filename.empty() ) { std::cerr << "csv-split: --default-filename: todo, just ask" << std::endl; }
        if( csv.binary() ) { size = csv.format().size(); }
        bool id_is_string = vm.count( "string" );
        bool id_is_time = vm.count( "time" );
        passthrough = vm.count("passthrough");
        if( id_is_string && id_is_time ) { std::cerr << "csv-split: --string and --time are mutually exclusive" << std::endl; return 1; }
        if( period > 0 ) { duration = boost::posix_time::microseconds( static_cast< unsigned int >( period * 1e6 )); }
        if( extension.empty() ) { suffix = csv.binary() || size > 0 ? ".bin" : ".csv"; }
        else { suffix += "."; suffix += extension; }
        streams = boost::program_options::collect_unrecognized( parsed.options, boost::program_options::include_positional );
        if( !streams.empty() && ( csv.has_field( "block" ) || id_is_time ) ) { std::cerr << "publisher streams are not compatible with splitting by block or timestamp." << std::endl; return 1; }
        if( id_is_string ) { run< std::string >(); }
        else if( id_is_time ) { run< boost::posix_time::ptime >(); }
        else { run< comma::uint32 >(); }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << argv[0] << ": " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << argv[0] << ": unknown exception" << std::endl; }
    return 1;
}
