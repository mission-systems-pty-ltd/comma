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
#else
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include <boost/lexical_cast.hpp>
#include "../../../io/file_descriptor.h"
#include "../../../base/exception.h"
#include "split.h"

namespace comma { namespace csv { namespace applications {

template < typename T >
split< T >::split( boost::optional< boost::posix_time::time_duration > period
            , const std::string& suffix
            , const comma::csv::options& csv
            , bool pass )
    : ofstream_( std::bind( &split< T >::ofstream_by_time_, this ) )
    , period_( period )
    , suffix_( suffix )
    , pass_ ( pass )
    , flush_( csv.flush )
    , is_shutdown_( false )
{
    if( ( csv.has_field( "t" ) || csv.fields.empty() ) && !period ) { COMMA_THROW( comma::exception, "please specify --period" ); }
    if( csv.fields.empty() ) { return; }
    if( csv.binary() ) { binary_.reset( new comma::csv::binary< input >( csv ) ); }
    else { ascii_.reset( new comma::csv::ascii< input >( csv ) ); }
    if( csv.has_field( "block" ) ) { ofstream_ = std::bind( &split< T >::ofstream_by_block_, this ); }
    else if( csv.has_field( "id" ) ) { ofstream_ = std::bind( &split< T >::ofstream_by_id_, this ); }
}

//to-do
template < typename T >
split< T >::split( boost::optional< boost::posix_time::time_duration > period
            , const std::string& suffix
            , const comma::csv::options& csv
            , const std::vector< std::string >& streams //to-do
            , bool pass )
    : split( period, suffix, csv, pass )
{
    transaction t( publishers_ );
    if( 0 < streams.size() )
    {
        for( auto const& si : streams )
        {
            auto key_pos = si.find_first_of( ':' );
            if( std::string::npos == key_pos ) { COMMA_THROW( comma::exception, "please specify id in output streams in format <id>:<stream>, got: " << si ); }

            auto const keys = comma::split( si.substr( 0, key_pos ), ',' );
            auto const address = si.substr( key_pos + 1 );
            auto publisher = std::make_shared< comma::io::publisher >( address , csv.binary() ? comma::io::mode::binary : comma::io::mode::ascii , false , csv.flush ); 

            for( auto const& ki : keys )
            {
                if( "..." != ki )
                {
                    t->insert( std::make_pair( boost::lexical_cast< T >( ki ), publisher ) );
                }
                else
                {
                    if( default_publisher_ ) { COMMA_THROW( comma::exception, "dont specify more than one '...' publisher stream" ); }
                    default_publisher_ = publisher;
                    break;
                }
            }
 
            //t->insert( std::make_pair( boost::lexical_cast< T >( si.substr( 0, key_pos ) )
            //            , std::shared_ptr< comma::io::publisher >( new comma::io::publisher( si.substr( key_pos+1 )
            //                    , csv.binary() ? comma::io::mode::binary : comma::io::mode::ascii
            //                    , false
            //                    , csv.flush ) ) ) );
        }
        acceptor_thread_ = std::thread( std::bind( &split< T >::accept_, std::ref( *this )));
    }
}

template < typename T >
split< T >::~split()
{
    is_shutdown_ = true;
    if( acceptor_thread_.joinable() )
    {
        acceptor_thread_.join();
        transaction t( publishers_ );
        for( auto& ii : *t ) { ii.second->close(); } 
    }
}

template < typename T >
void split< T >::accept_()
{
    comma::io::select select;
    {
        transaction t( publishers_ );
        for( auto& ii : *t ) { if( ii.second->acceptor_file_descriptor() != comma::io::invalid_file_descriptor ) { select.read().add( ii.second->acceptor_file_descriptor() ); } } 
        if( default_publisher_ ) { if( default_publisher_->acceptor_file_descriptor() != comma::io::invalid_file_descriptor ) { select.read().add( default_publisher_->acceptor_file_descriptor() ); } }
    }
    while( !is_shutdown_ )
    {
        select.wait( boost::posix_time::millisec( 100 ) ); // arbitrary timeout
        transaction t( publishers_ );
        for( auto& ii : *t ) { if( select.read().ready( ii.second->acceptor_file_descriptor() ) ) { ii.second->accept(); } }
        if( default_publisher_ ) { if( select.read().ready( default_publisher_->acceptor_file_descriptor() ) ) { default_publisher_->accept(); } }
    }
}


template < typename T >
bool split< T >::published_on_stream( const char* data, unsigned int size )
{
    transaction t( publishers_ );
    if( t->empty() ) { return false; }

    auto iter = t->find( current_.id );
    if( t->end() != iter ) { iter->second->write( data, size, false ); return true; }
    if( default_publisher_ ) { default_publisher_->write( data, size, false ); return true; }

    return false;
}

template < typename T >
void split< T >::write( const char* data, unsigned int size )
{
    mode_ = std::ofstream::out | std::ofstream::binary;
    if( binary_ ) { binary_->get( current_, data ); }
    else { current_.timestamp = boost::get_system_time(); }
    if( !published_on_stream( data, size ) )
    {
        ofstream_().write( data, size );
        if( flush_ ) { ofstream_().flush(); }
    }
    if ( pass_ ) { std::cout.write( data, size ); std::cout.flush(); }
}

template < typename T >
void split< T >::write ( std::string line )
{
    mode_ = std::ofstream::out; // quick and dirty
    if( ascii_ ) { ascii_->get( current_, line ); }
    else { current_.timestamp = boost::get_system_time(); }
    line += '\n';
    if( !published_on_stream( &line[0], line.size()) )
    {
        std::ofstream& ofs = ofstream_();
        ofs.write( &line[0], line.size() );
        //ofs.put( '\n' );
        if( flush_ ) { ofs.flush(); }
    }
    if ( pass_ ) { std::cout.write( &line[0], line.size() ); /*std::cout.put('\n');*/ std::cout.flush(); }
}

template < typename T >
std::ofstream& split< T >::ofstream_by_time_()
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

template < typename T >
std::ofstream& split< T >::ofstream_by_block_()
{
    if( !last_ || last_->block != current_.block )
    {
        file_.close();
        std::string name = boost::lexical_cast< std::string >( current_.block ) + suffix_;
        file_.open( name.c_str(), mode_ );
        last_ = current_;
    }
    return file_;
}

template < typename T >
static std::string make_filename_from_id(const T& id, std::string suffix )
{
    return boost::lexical_cast< std::string >( id ) + suffix;
}

static std::string make_filename_from_id(const boost::posix_time::ptime& id, std::string suffix )
{
    return boost::posix_time::to_iso_string( id ) + suffix;
}

template < typename T >
std::ofstream& split< T >::ofstream_by_id_()
{
    typename Files::iterator it = files_.find( current_.id );
    if( it == files_.end() )
    {
        #ifdef WIN32
        static unsigned int max_number_of_open_files = 128;
        #else
        static struct rlimit r;
        static int q = getrlimit( RLIMIT_NOFILE, &r );
        if( q != 0 ) { COMMA_THROW( comma::exception, "getrlimit() failed" ); }
        static unsigned int max_number_of_open_files = static_cast< unsigned int >( r.rlim_cur );
        #endif
        if( files_.size() + 10 > max_number_of_open_files ) { files_.clear(); } // quick and dirty, may be too drastic...
        std::ios_base::openmode mode = mode_;
        if( seen_ids_.find( current_.id ) == seen_ids_.end() ) { seen_ids_.insert( current_.id ); }
        else { mode |= std::ofstream::app; }
        std::string name = make_filename_from_id( current_.id, suffix_);
        std::shared_ptr< std::ofstream > stmp( new std::ofstream( name.c_str(), mode ) );
        it = files_.insert( std::make_pair( current_.id, stmp ) ).first;
    }
    return *it->second;
}

template class split< comma::uint32 >;
template class split< std::string >;
template class split< boost::posix_time::ptime >;

} } } // namespace comma { namespace csv { namespace applications {
