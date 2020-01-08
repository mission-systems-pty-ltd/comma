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

#include <unordered_map>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include "../../../base/exception.h"
#include "../../../csv/stream.h"
#include "../../../csv/traits.h"
#include "../../../io/file_descriptor.h"
#include "../../../name_value/parser.h"
#include "../../../visiting/traits.h"
#include "split.h"

namespace comma { namespace csv { namespace applications {

template < typename T > struct filename_record
{
    T id;
    std::string filename;
    filename_record( const T& id = 0, const std::string& filename = "" ): id( id ), filename( filename ) {}
};

} } } // namespace comma { namespace csv { namespace applications {

namespace comma { namespace visiting {

template < typename T > struct traits< comma::csv::applications::filename_record< T > >
{
    template< typename K, typename V > static void visit( const K& k, comma::csv::applications::filename_record< T >& t, V& v )
    {
        v.apply( "id", t.id );
        v.apply( "filename", t.filename );
    }

    template< typename K, typename V > static void visit( const K& k, const comma::csv::applications::filename_record< T >& t, V& v )
    {
        v.apply( "id", t.id );
        v.apply( "filename", t.filename );
    }
};

} } // namespace comma { namespace visiting {

namespace comma { namespace csv { namespace applications {

std::pair< std::unordered_map< comma::uint32, std::string >, bool > static filenames( const std::string& filename )
{
    std::pair< std::unordered_map< comma::uint32, std::string >, bool > r;
    r.second = false;
    if( filename.empty() ) { return r; }
    auto csv = comma::name_value::parser( "filename" ).get< comma::csv::options >( filename );
    if( csv.fields.empty() ) { csv.fields = "filename"; }
    std::ifstream ifs( csv.filename );
    if( !ifs.is_open() ) { COMMA_THROW( comma::exception, "could not open --files='" << csv.filename << "'" ); }
    comma::csv::input_stream< filename_record< comma::uint32 > > is( ifs, csv ); // quick and dirty; todo: support templated map
    comma::uint32 id = 0;
    r.second = csv.has_field( "id" );
    while( is.ready() || ifs.good() )
    {
        auto p = is.read();
        if( p == nullptr ) { break; }
        r.first[ r.second ? p->id : id++ ] = p->filename; // quick and dirty
    }
    if( r.first.empty() ) { COMMA_THROW( comma::exception, "got no filenames from '" << csv.filename << "'" ); }
    return r;
}
    
template < typename T >
split< T >::split( const boost::optional< boost::posix_time::time_duration >& period
                 , const std::string& suffix
                 , const comma::csv::options& csv
                 , bool pass
                 , const std::string& filenames
                 , const std::string& default_filename )
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
    boost::tie( filenames_, filenames_have_id_ ) = applications::filenames( filenames );
    if( csv.has_field( "block" ) )
    {
        ofstream_ = std::bind( &split< T >::ofstream_by_block_, this );
    }
    else
    {
        if( csv.has_field( "id" ) )
        { 
            ofstream_ = std::bind( &split< T >::ofstream_by_id_, this );
        }
        else
        {    
            if( !filenames_.empty() ) { COMMA_THROW( comma::exception, "--files given, but no block field specified in --fields" ); }
        }
    }
}

template < typename T >
split< T >::split( const boost::optional< boost::posix_time::time_duration >& period
                 , const std::string& suffix
                 , const comma::csv::options& csv
                 , const std::vector< std::string >& streams //to-do
                 , bool pass
                 , const std::string& filenames
                 , const std::string& default_filename )
    : split( period, suffix, csv, pass, filenames, default_filename )
{
    if( streams.empty() ) { return; }
    auto const io_mode = csv.binary() ? comma::io::mode::binary : comma::io::mode::ascii;
    for( auto const& si : streams )
    {
        auto const stream_values = comma::split( si, ';' );
        if( 2 > stream_values.size() || stream_values[ 0 ].empty() || stream_values[ 1 ].empty() ) { COMMA_THROW( comma::exception, "please specify <id> and output <stream> in format <id>;<stream>, got: " << si ); }
        transaction t( publishers_ );
        std::unique_ptr< comma::io::publisher > publisher( new comma::io::publisher( stream_values[1], io_mode, false, csv.flush ) );
        if( "..." == stream_values[0] )
        {
            if( default_publisher_ ) { COMMA_THROW( comma::exception, "multiple output streams have the id: ..." ); }
            default_publisher_ = std::move( publisher );
        }
        else
        {
            auto publisher_pos = t->insert( std::move( publisher ) );
            auto const keys = comma::split( stream_values[0], ',' );
            for( auto const& ki : keys )
            {
                auto const kii = boost::lexical_cast< T >( ki );
                if( seen_ids_.end() !=  seen_ids_.find( kii ) ) { COMMA_THROW( comma::exception, "multiple output streams have the id: " << ki ); }
                seen_ids_.insert( kii );
                mapped_publishers_.insert( std::make_pair( kii, publisher_pos.first->get() ) );
            }
        }
    }
    acceptor_thread_ = std::thread( std::bind( &split< T >::accept_, std::ref( *this )));
}

template < typename T >
split< T >::~split()
{
    is_shutdown_ = true;
    if( acceptor_thread_.joinable() )
    {
        acceptor_thread_.join();
        transaction t( publishers_ );
        for( auto& ii : *t ) { ii->close(); } 
    }
}

template < typename T >
void split< T >::accept_()
{
    comma::io::select select;
    {
        transaction t( publishers_ );
        for( auto& ii : *t ) { if( ii->acceptor_file_descriptor() != comma::io::invalid_file_descriptor ) { select.read().add( ii->acceptor_file_descriptor() ); } } 
        if( default_publisher_ ) { if( default_publisher_->acceptor_file_descriptor() != comma::io::invalid_file_descriptor ) { select.read().add( default_publisher_->acceptor_file_descriptor() ); } }
    }
    while( !is_shutdown_ )
    {
        select.wait( boost::posix_time::millisec( 100 ) ); // arbitrary timeout
        transaction t( publishers_ );
        for( auto& ii : *t ) { if( select.read().ready( ii->acceptor_file_descriptor() ) ) { ii->accept(); } }
        if( default_publisher_ ) { if( select.read().ready( default_publisher_->acceptor_file_descriptor() ) ) { default_publisher_->accept(); } }
    }
}


template < typename T >
bool split< T >::published_on_stream( const char* data, unsigned int size )
{
    transaction t( publishers_ );
    if( t->empty() && !default_publisher_ ) { return false; }

    auto iter = mapped_publishers_.find( current_.id );
    if( mapped_publishers_.end() != iter ) { iter->second->write( data, size, false ); return true; }
    if( default_publisher_ ) { default_publisher_->write( data, size, false ); return true; }
    return true;
}

template < typename T >
void split< T >::write( const char* data, unsigned int size )
{
    mode_ = std::ofstream::out | std::ofstream::binary;
    if( binary_ ) { binary_->get( current_, data ); }
    else { current_.timestamp = boost::get_system_time(); }
    if( !published_on_stream( data, size ) ) // todo? or bind write function on initialisation and call it here?
    {
        auto ofs = ofstream_();
        if( ofs )
        {
            ofs->write( data, size );
            if( flush_ ) { ofs->flush(); }
        }
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
    if( !published_on_stream( &line[0], line.size()) ) // todo? or bind write function on initialisation and call it here?
    {
        auto ofs = ofstream_();
        if( ofs )
        {
            ofs->write( &line[0], line.size() );
            //ofs.put( '\n' );
            if( flush_ ) { ofs->flush(); }
        }
    }
    if ( pass_ ) { std::cout.write( &line[0], line.size() ); /*std::cout.put('\n');*/ std::cout.flush(); }
}

template < typename T >
std::ofstream* split< T >::ofstream_by_time_()
{
    if( !last_ || current_.timestamp > ( last_->timestamp + *period_ ) )
    {
        file_.close();
        std::string time = boost::posix_time::to_iso_string( current_.timestamp );
        if( time.find_first_of( '.' ) == std::string::npos ) { time += ".000000"; }
        file_.open( ( time + suffix_ ).c_str(), mode_ );
        last_ = current_;
    }
    return &file_;
}

template < typename T >
std::ofstream* split< T >::ofstream_by_block_()
{
    static comma::uint32 id = 0;
    if( !last_ || last_->block != current_.block )
    {
        file_.close();
        std::string filename;
        if( !filenames_.empty() )
        {
            auto it = filenames_.find( filenames_have_id_ ? current_.block : id );
            if( it == filenames_.end() ) { return nullptr; }
            filename = it->second;
            const auto& dirname = boost::filesystem::path( filename ).parent_path();
            if( !( dirname.empty() || boost::filesystem::is_directory( dirname ) || boost::filesystem::create_directories( dirname ) ) )
            {
                COMMA_THROW( comma::exception, "failed to create directory '" << dirname << "' for file: '" << filename << "'" );
            }
        }
        if( filename.empty() ) { filename = boost::lexical_cast< std::string >( current_.block ) + suffix_; }
        file_.open( &filename[0], mode_ );
        if( !file_.is_open() ) { COMMA_THROW( comma::exception, "failed to open '" << filename << "'" ); }
        last_ = current_;
        ++id;
    }
    return &file_;
}

template < typename T > static std::string to_string( const T& v ) { return boost::lexical_cast< std::string >( v ); }

template <> std::string to_string< boost::posix_time::ptime >( const boost::posix_time::ptime& v ) { return boost::posix_time::to_iso_string( v ); }

template < typename T, typename M > static std::string find_( const M& m, const T& id ) { COMMA_THROW( comma::exception, "id-to-filename map not implemented for this type" ); }

template <> std::string find_< comma::uint32, std::unordered_map< comma::uint32, std::string > >( const std::unordered_map< comma::uint32, std::string >& m, const comma::uint32& id )
{
    auto it = m.find( id );
    return it == m.end() ? std::string() : it->second;
}

template < typename T >
std::string split< T >::filename_from_id_( const T& id ) { return filenames_.empty() ? to_string( id ) + suffix_ : find_( filenames_, id ); }

template < typename T >
std::ofstream* split< T >::ofstream_by_id_()
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
        std::string name = filename_from_id_( current_.id );
        if( name.empty() ) { return nullptr; }
        std::shared_ptr< std::ofstream > stmp( new std::ofstream( &name[0], mode ) );
        it = files_.insert( std::make_pair( current_.id, stmp ) ).first;
    }
    return it->second.get();
}

template class split< comma::uint32 >;
template class split< std::string >;
template class split< boost::posix_time::ptime >;

} } } // namespace comma { namespace csv { namespace applications {
