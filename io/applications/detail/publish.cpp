// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2020 Vsevolod Vlaskine
// All rights reserved.

#include "publish.h"

namespace comma { namespace io { namespace detail {

publish::publish( const std::vector< std::string >& endpoints
                , unsigned int packet_size
                , bool discard
                , bool flush
                , bool output_number_of_clients
                , bool update_no_clients
                , unsigned int cache_size )
    : discard_( discard )
    , flush_( flush )
    , buffer_( packet_size, '\0' )
    , packet_size_( packet_size )
    , output_number_of_clients_( output_number_of_clients )
    , cache_size_( cache_size )
    , update_no_clients_( update_no_clients )
    , got_first_client_ever_( false )
    , sizes_( endpoints.size(), 0 )
    , num_clients_( 0 )
    , is_shutdown_( false )
{
    bool has_primary_stream = false;
    for( unsigned int i = 0; i < endpoints.size(); ++i )
    {
        comma::name_value::map m( endpoints[i], "address", ';', '=' );
        bool secondary = !m.exists( "primary" ) && m.exists( "secondary" );
        endpoints_.push_back( endpoint( m.value< std::string >( "address" ), secondary ) ); // todo? quick and dirty; better usage semantics?
        if( !secondary ) { has_primary_stream = true; }
    }
    COMMA_ASSERT_BRIEF( has_primary_stream, "please specify at least one primary stream" );
    struct sigaction new_action, old_action;
    new_action.sa_handler = SIG_IGN;
    sigemptyset( &new_action.sa_mask );
    sigaction( SIGPIPE, NULL, &old_action );
    sigaction( SIGPIPE, &new_action, NULL );
    transaction_t t( publishers_ );
    t->resize( endpoints.size() );
    for( std::size_t i = 0; i < endpoints.size(); ++i )
    {
        if( !endpoints_[i].secondary ) { ( *t )[i].reset( new comma::io::publisher( endpoints_[i].address, is_binary_() ? comma::io::mode::binary : comma::io::mode::ascii, !discard, flush ) ); }
    }
    acceptor_thread_.reset( new boost::thread( boost::bind( &publish::accept_, boost::ref( *this ))));
}
        
publish::~publish()
{
    is_shutdown_ = true;
    acceptor_thread_->join();
    transaction_t t( publishers_ );
    for( std::size_t i = 0; i < t->size(); ++i ) { if( ( *t )[i] ) { ( *t )[i]->close(); } }
}
        
void publish::disconnect_all()
{
    transaction_t t( publishers_ );
    for( auto& p: *t ) { if( p ) { p->disconnect_all(); } }
    handle_sizes_( t ); // quick and dirty
}

bool publish::write( const std::string& s )
{
    transaction_t t( publishers_ );
    if( cache_size_ > 0 )
    {
        cache_.push_back( s );
        if( cache_.size() > cache_size_ ) { cache_.pop_front(); }
    }
    for( auto& p: *t ) { if( p ) { p->write( &s[0], s.size(), false ); } } // for( std::size_t i = 0; i < t->size(); ++i ) { if( ( *t )[i] ) { ( *t )[i]->write( &buffer_[0], buffer_.size(), false ); } }
    return handle_sizes_( t );
}

bool publish::write( const char* buf, unsigned int size )
{
    return write( std::string( buf, size ) ); // todo: quick and dirty, watch performance
}

bool publish::read( std::istream& input )
{
    if( is_binary_() )
    {
        input.read( &buffer_[0], buffer_.size() );
        if( input.gcount() < int( buffer_.size() ) || !input.good() ) { return false; }
    }
    else
    {
        std::getline( input, buffer_ );
        buffer_ += '\n';
        if( !input.good() ) { return false; }
    }
    return write( buffer_ );
}

bool publish::handle_sizes_( transaction_t& t ) // todo? why pass transaction? it doen not seem going out of scope at the point of call; remove?
{
    if( !output_number_of_clients_ && !update_no_clients_ ) { return true; }
    unsigned int total = 0;
    bool changed = false;
    has_primary_clients_ = false;
    for( unsigned int i = 0; i < t->size(); ++i )
    {
        unsigned int size = ( *t )[i] ? ( *t )[i]->size() : 0;
        total += size;
        if( !endpoints_[i].secondary && size > 0 ) { has_primary_clients_ = true; }
        if( sizes_[i] == size ) { continue; }
        sizes_[i] = size;
        changed = true;
        num_clients_ = total;
    }
    if( !changed ) { return true; }
    if( output_number_of_clients_ )
    {
        std::cout << boost::posix_time::to_iso_string( boost::posix_time::microsec_clock::universal_time() );
        for( unsigned int i = 0; i < sizes_.size(); ++i ) { std::cout << ',' << sizes_[i]; }
        std::cout << std::endl;
    }
    if( update_no_clients_ )
    {
        if( total > 0 ) { got_first_client_ever_ = true; }
        else if( got_first_client_ever_ ) { comma::saymore() << "the last client exited" << std::endl; return false; }
    }
    return true;
}

void publish::accept_()
{
    comma::io::select select;
    {
        transaction_t t( publishers_ );
        for( unsigned int i = 0; i < t->size(); ++i )
        {
            if( !( *t )[i] ) { continue; }
            if( ( *t )[i]->acceptor_file_descriptor() != comma::io::invalid_file_descriptor ) { select.read().add( ( *t )[i]->acceptor_file_descriptor() ); }
        }
    }
    while( !is_shutdown_ )
    {
        select.wait( boost::posix_time::millisec( 100 ) ); // todo? make timeout configurable?
        transaction_t t( publishers_ );
        for( unsigned int i = 0; i < t->size(); ++i )
        {
            if( ( *t )[i] && select.read().ready( ( *t )[i]->acceptor_file_descriptor() ) )
            {
                const auto& streams = ( *t )[i]->accept();
                for( auto s: streams )
                {
                    for( const auto& c: cache_ ) { ( *s )->write( &c[0], c.size() ); }
                    if( flush_ ) { ( *s )->flush(); }
                }
            }
        }
        handle_sizes_( t );
        if( has_primary_clients_ )
        {
            for( unsigned int i = 0; i < t->size(); ++i )
            {
                if( !endpoints_[i].secondary || ( *t )[i] ) { continue; }
                ( *t )[i].reset( new comma::io::publisher( endpoints_[i].address, is_binary_() ? comma::io::mode::binary : comma::io::mode::ascii, !discard_, flush_ ) );
                if( ( *t )[i]->acceptor_file_descriptor() != comma::io::invalid_file_descriptor ) { select.read().add( ( *t )[i]->acceptor_file_descriptor() ); }
            }
        }
        else
        {
            for( unsigned int i = 0; i < t->size(); ++i )
            {
                if( !endpoints_[i].secondary || !( *t )[i] ) { continue; }
                select.read().remove( ( *t )[i]->acceptor_file_descriptor() );
                ( *t )[i].reset();
            }
        }
    }
}

} } } // namespace comma { namespace io { namespace detail {
