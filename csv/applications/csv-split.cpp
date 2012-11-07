// This file is part of comma, a generic and flexible library 
// for robotics research.
//
// Copyright (C) 2011 The University of Sydney
//
// comma is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// comma is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License 
// for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with comma. If not, see <http://www.gnu.org/licenses/>.

/// @author vsevolod vlaskine

#ifdef WIN32
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#endif

#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <comma/application/contact_info.h>
#include <comma/csv/impl/program_options.h>
#include <comma/csv/applications/split/split.h>

int main( int argc, char** argv )
{
    
    try
    {
        double period = 0;
        unsigned int size = 0;
        std::string extension;
        boost::program_options::options_description description( "options" );
        description.add_options()
            ( "help,h", "display help message" )
            ( "size,c", boost::program_options::value< unsigned int >( &size ), "packet size, only full packets will be written" )
            ( "period,t", boost::program_options::value< double >( &period ), "period in seconds after which a new file is created" )
            ( "suffix,s", boost::program_options::value< std::string >( &extension ), "filename extension; default will be csv or bin, depending whether it is ascii or binary" );
        description.add( comma::csv::program_options::description() );
        boost::program_options::variables_map vm;
        boost::program_options::store( boost::program_options::parse_command_line( argc, argv, description), vm );
        boost::program_options::parsed_options parsed = boost::program_options::command_line_parser(argc, argv).options( description ).allow_unregistered().run();
        boost::program_options::notify( vm );
        if ( vm.count( "help" ) || vm.count( "long-help" ) )
        {
            std::cerr << std::endl;
            std::cerr << "read from stdin by packet or by line and split the data into files, named by time" << std::endl;
            std::cerr << "usage: csv-split [options]" << std::endl;
            std::cerr << std::endl;
            std::cerr << description << std::endl;
            std::cerr << std::endl;
            std::cerr << "fields" << std::endl;
            std::cerr << "    t: if present, use timestamp from the packet; if absent, use system time" << std::endl;
            std::cerr << "    block: split on the block number change" << std::endl;
            std::cerr << "    id: split by id (same as block, except does not have to be contiguous by the price of worse performance)" << std::endl;
            std::cerr << std::endl;
            std::cerr << comma::contact_info << std::endl;
            std::cerr << std::endl;
            return 1;
        }
        comma::csv::options csv = comma::csv::program_options::get( vm );
        if( csv.binary() ) { size = csv.format().size(); }
        boost::optional< boost::posix_time::time_duration > duration;
        if( period > 0 ) { duration = boost::posix_time::microseconds( period * 1e6 ); }
        std::string suffix;
        if( extension.empty() ) { suffix = csv.binary() || size > 0 ? ".bin" : ".csv"; }
        else { suffix += "."; suffix += extension; }
        comma::csv::applications::split split( duration, suffix, csv );
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
        return 0;
    }
    catch( std::exception& ex )
    {
        std::cerr << argv[0] << ": " << ex.what() << std::endl;
    }
    catch( ... )
    {
        std::cerr << argv[0] << ": unknown exception" << std::endl;
    }
    return 1;
}
