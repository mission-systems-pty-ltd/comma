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
#else
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread_time.hpp>
#include <comma/base/exception.h>
#include "./split.h"

namespace comma { namespace csv { namespace applications {

split::split( boost::posix_time::time_duration period
            , const std::string& suffix
            , const comma::csv::options& csv )
    : ofstream_( boost::bind( &split::ofstream_by_time_, this ) )
    , period_( period )
    , suffix_( suffix )
{
    if( csv.binary() ) { binary_.reset( new comma::csv::binary< input >( csv ) ); }
    else { ascii_.reset( new comma::csv::ascii< input >( csv ) ); }
    std::cerr << " fields " << csv.fields << " binary " << csv.format().string() << std::endl;
}

void split::write( const char* data, unsigned int size )
{
    mode_ = std::ofstream::out | std::ofstream::binary;
    if( binary_ ) { binary_->get( current_, data ); }
    else { current_.timestamp = boost::get_system_time(); }
    ofstream_().write( data, size );
}

void split::write ( const std::string& line )
{
    mode_ = std::ofstream::out; // quick and dirty
    if( ascii_ ) { ascii_->get( current_, line ); }
    else { current_.timestamp = boost::get_system_time(); }
    std::ofstream& ofs = ofstream_();
    ofs.write( &line[0], line.size() );
    ofs.put( '\n' );
}

std::ofstream& split::ofstream_by_time_()
{
    if( !last_ || current_.timestamp > ( last_->timestamp + *period_ ) )
    {
        file_.close();
        std::string time = boost::posix_time::to_iso_string( current_.timestamp );
        if( time.find_first_of( '.' ) == std::string::npos ) { time += ".000000"; }
        file_.open( ( time + suffix_ ).c_str(), mode_ );
        last_ = current_;
    }
    return file_;
}

} } } // namespace comma { namespace csv { namespace applications {
