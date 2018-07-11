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
#include "../../application/contact_info.h"
#include "../../csv/impl/program_options.h"
#include "../../csv/traits.h"
#include "split/split.h"

comma::csv::options csv;
std::vector< std::string > streams;
boost::optional< boost::posix_time::time_duration > duration;
std::string suffix;
unsigned int size = 0;
bool passthrough;

template < typename T >
void run()
{
    comma::csv::applications::split< T > split( duration, suffix, csv, streams, passthrough );
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
            ( "size,c", boost::program_options::value< unsigned int >( &size ), "packet size, only full packets will be written" )
            ( "period,t", boost::program_options::value< double >( &period ), "period in seconds after which a new file is created" )
            ( "suffix,s", boost::program_options::value< std::string >( &extension ), "filename extension; default will be csv or bin, depending whether it is ascii or binary" )
            ( "string", "id is string; default: 32-bit integer" )
            ( "time", "id is time; default: 32-bit integer" )
            ( "passthrough,pass", "pass data through to stdout" );
        description.add( comma::csv::program_options::description() );
        boost::program_options::variables_map vm;
        boost::program_options::store( boost::program_options::parse_command_line( argc, argv, description), vm );
        boost::program_options::parsed_options parsed = boost::program_options::command_line_parser(argc, argv).options( description ).allow_unregistered().run();
        boost::program_options::notify( vm );
        if ( vm.count( "help" ) || vm.count( "long-help" ) )
        {
            std::cerr << std::endl;
            std::cerr << "read from stdin by packet or by line and split into files named by field value or time (if split by time)." << std::endl;
            std::cerr << "if splitting by id, input can also be splitted into streams" << std::endl;
            std::cerr << std::endl;
            std::cerr << "usage: csv-split [options] [outputs]*" << std::endl;
            std::cerr << std::endl;
            std::cerr << "use cases" << std::endl;
            std::cerr << "    split by id field, output to files" << std::endl;
            std::cerr << "        if id field present in --fields:" << std::endl;
            std::cerr << "        for each id value, output records with this id to a separate file, e.g. 0.csv, 1.csv, etc" << std::endl;
            std::cerr << "        example: ( echo 0,a; echo 1,b; echo 0,c; echo 2,d ) | csv-split --fields id" << std::endl;
            std::cerr << std::endl;
            std::cerr << "    split by block field, output to files" << std::endl;
            std::cerr << "        if block field present in --fields:" << std::endl;
            std::cerr << "        output records with this block to a separate file, on change of block, open a new file, e.g. 0.csv, 1.csv, etc" << std::endl;
            std::cerr << "        example: ( echo 0,a; echo 1,b; echo 1,c; echo 2,d ) | csv-split --fields block" << std::endl;
            std::cerr << std::endl;
            std::cerr << "    split by t field, output to files" << std::endl;
            std::cerr << "        if t (timestamp) field present in --fields:" << std::endl;
            std::cerr << "        separate records into different time periods, outputting in separate files" << std::endl;
            std::cerr << "        example: ( echo 20170101T000001,a; echo 20170101T000003,b; echo 20170101T000007,c ) | csv-split --fields=t --period=4" << std::endl;
            std::cerr << std::endl;
            std::cerr << "    split by id field, output to streams" << std::endl;
            std::cerr << "        if output streams (see example below) are present on the command line and id field present in --fields:" << std::endl;
            std::cerr << "        output records with the given ids to the corresponding streams, while outputing the rest into files" << std::endl;
            std::cerr << "        records with ids for which output stream is not specified will be discarded, unless ... stream is specified:" << std::endl;
	    std::cerr << std::endl;
	    std::cerr << "        outputs: <keys>;<stream>; to send records with a given set of ids to this stream" << std::endl;
            std::cerr << "            keys:" << std::endl;
            std::cerr << "                <id>[,<id>]*: comma-separated list of ids, e.g: '5' or '2,5,7', etc" << std::endl;
            std::cerr << "                ...: three dots mean: send to this stream all the records with ids for which no other stream is specified (see example below)" << std::endl;
            std::cerr << "            stream:" << std::endl;
	    std::cerr << "                tcp:<port>: e.g. tcp:1234" << std::endl;
	    std::cerr << "                udp:<port>: e.g. udp:1234 (todo)" << std::endl;
	    std::cerr << "                local:<name>: linux/unix local server socket e.g. local:./tmp/my_socket" << std::endl;
	    std::cerr << "                <named pipe name>: named pipe, which will be re-opened, if client reconnects" << std::endl;
	    std::cerr << "                <filename>: a regular file" << std::endl;
            std::cerr << "        example: ( echo 0,a; echo 1,b; echo 0,c; echo 2,d ) | csv-split --fields id \"0,1;tcp:5999\" \"...;local:/tmp/named_fifo\"" << std::endl;
            std::cerr << std::endl;
            std::cerr << description << std::endl;
            std::cerr << std::endl;
            std::cerr << "fields to split by listed in descending precedence" << std::endl;
            std::cerr << "    block: split on the block number change" << std::endl;
            std::cerr << "    id: split by id (same as block, except does not have to be contiguous by the price of worse performance)" << std::endl;
            std::cerr << "    t: if present, use timestamp from the packet; if absent, use system time" << std::endl;
	    std::cerr << std::endl;
            std::cerr << comma::contact_info << std::endl;
            std::cerr << std::endl;
            return 1;
        }
        csv = comma::csv::program_options::get( vm );
        if( csv.binary() ) { size = csv.format().size(); }
        bool id_is_string = vm.count( "string" );
        bool id_is_time = vm.count( "time" );
        passthrough = vm.count("passthrough");
        
        if( id_is_string && id_is_time ) { std::cerr << "csv-split: either --string or --time" << std::endl; }
        if( period > 0 ) { duration = boost::posix_time::microseconds( period * 1e6 ); }
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

