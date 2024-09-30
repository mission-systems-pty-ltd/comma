// Copyright (c) 2011 The University of Sydney
// All rights reserved.

/// @author cedric wohlleber

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/bind/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "../../base/exception.h"
#include "../../io/file_descriptor.h"
#include "../../string/string.h"
#include "server.h"

namespace comma { namespace io { namespace impl {

template < typename Stream > struct stream_traits;

template <> struct stream_traits< io::istream >
{
    static constexpr bool is_input_stream{true};
    static constexpr bool is_output_stream{false};
};

template <> struct stream_traits< io::ostream >
{
    static constexpr bool is_input_stream{false};
    static constexpr bool is_output_stream{true};
};

template <> struct stream_traits< io::iostream >
{
    static constexpr bool is_input_stream{true};
    static constexpr bool is_output_stream{true};
};

template < typename Stream > class file_acceptor : public acceptor< Stream >
{
    public:
        file_acceptor( const std::string& name, io::mode::value mode ): name_( name ), mode_( mode ), closed_( true ), fd_( io::invalid_file_descriptor ) {}

        ~file_acceptor()
        {
#ifndef WIN32
            ::close( fd_ );
#else
            _close( fd_ );
#endif
        }

        Stream* accept( boost::posix_time::time_duration )
        {
            if( !closed_ ) { return NULL; }
#ifndef WIN32
            fd_ = ::open( &name_[0], O_WRONLY | O_CREAT | O_NONBLOCK, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ); // quick and dirty
#else
            fd_ = _open( &name_[0], O_WRONLY | _O_CREAT, _S_IWRITE );
#endif
            if( fd_ == io::invalid_file_descriptor ) { return nullptr; }
            closed_ = false;
            return new Stream( name_, mode_, io::mode::non_blocking ); // quick and dirty
        }

        void notify_closed() { closed_ = true; ::close( fd_ ); }
        
        io::file_descriptor fd() const { return fd_; }

    private:
        const std::string name_;
        const io::mode::value mode_;
        bool closed_{false};
        io::file_descriptor fd_{0}; // todo: make io::istream, io::ostream non-throwing on construction
};

struct Tcp {};
template < typename S > struct socket_traits {};

template <> struct socket_traits< Tcp >
{
    typedef boost::asio::ip::tcp::endpoint endpoint_type;
    typedef boost::asio::ip::tcp::acceptor acceptor;
    typedef boost::asio::ip::tcp::iostream iostream;
    typedef unsigned short name_type;
    static endpoint_type endpoint( unsigned short port ) { return endpoint_type( boost::asio::ip::tcp::v4(), port ); }
};

#ifndef WIN32
struct local {};
template <> struct socket_traits< local >
{
    typedef boost::asio::local::stream_protocol::endpoint endpoint_type;
    typedef boost::asio::local::stream_protocol::acceptor acceptor;
    typedef boost::asio::local::stream_protocol::iostream iostream;
    typedef std::string name_type;
    static endpoint_type endpoint( const std::string& name ) { return endpoint_type( name ); }
};
#endif

template < typename Stream, typename S > class socket_acceptor : public acceptor< Stream >
{
    public:
        socket_acceptor( const typename socket_traits< S >::name_type& name, io::mode::value mode )
            : mode_( mode )
            , _acceptor( m_service, socket_traits< S >::endpoint( name ) )
        {
#ifndef WIN32
#if (BOOST_VERSION >= 106600)
            select_.read().add( _acceptor.native_handle() );
#else
            select_.read().add( _acceptor.native() );
#endif
#else
#if (BOOST_VERSION >= 106600)
            SOCKET socket = _acceptor.native_handle();
#else
            SOCKET socket = _acceptor.native();
#endif
            select_.read().add( socket );
#endif
        }

        Stream* accept( boost::posix_time::time_duration timeout )
        {
            select_.wait( timeout );
#ifndef WIN32
#if (BOOST_VERSION >= 106600)
            if( !select_.read().ready( _acceptor.native_handle() ) ) { return nullptr; }
#else
            if( !select_.read().ready( _acceptor.native() ) ) { return nullptr; }
#endif
#else
#if (BOOST_VERSION >= 106600)
            SOCKET socket = _acceptor.native_handle();
#else
            SOCKET socket = _acceptor.native();
#endif
            if( !select_.read().ready( socket ) ) { return nullptr; }
#endif
            typename socket_traits< S >::iostream* stream = new typename socket_traits< S >::iostream;
            _acceptor.accept( *( stream->rdbuf() ) );
#if (BOOST_VERSION >= 106600)
            return new Stream( stream, stream->rdbuf()->native_handle(), mode_, boost::bind( &socket_traits< S >::iostream::close, stream ) );
#else
            return new Stream( stream, stream->rdbuf()->native(), mode_, boost::bind( &socket_traits< S >::iostream::close, stream ) );
#endif
        }

        void close() { _acceptor.close(); }

#ifndef WIN32
#if (BOOST_VERSION >= 106600)
        io::file_descriptor fd() const { return const_cast< typename socket_traits< S >::acceptor& >( _acceptor ).native_handle(); }
#else
        io::file_descriptor fd() const { return const_cast< typename socket_traits< S >::acceptor& >( _acceptor ).native(); }
#endif
#else
        io::file_descriptor fd() const { return io::invalid_file_descriptor; }
#endif

    private:
        io::mode::value mode_{io::mode::binary};
        io::select select_;
#if (BOOST_VERSION >= 106600)
        boost::asio::io_context m_service;
#else
        boost::asio::io_service m_service;
#endif
        typename socket_traits< S >::acceptor _acceptor;
};

template < typename Stream >
class zero_acceptor_ : public acceptor< Stream >
{
    public:
        zero_acceptor_( const std::string& name, io::mode::value mode ): stream_( new Stream( name, mode ) ), accepted_( false ) {}

        Stream* accept( boost::posix_time::time_duration )
        {
            if( accepted_ ) { return nullptr; }
            accepted_ = true;
            return stream_;
        }

        void close() { stream_->close(); }
        
        io::file_descriptor fd() const { return io::invalid_file_descriptor; } // quick and dirty

    private:
        Stream* stream_{nullptr};
        bool accepted_{false};
};

template < typename Stream > server< Stream >::server( const std::string& name, io::mode::value mode, bool blocking, bool flush )
    : blocking_( blocking ),
      flush_( flush )
{
    std::vector< std::string > v = comma::split( name, ':' );
    if( v[0] == "tcp" )
    {
        if( v.size() != 2 ) { COMMA_THROW( comma::exception, "expected tcp server endpoint, got " << name ); }
        _acceptor.reset( new socket_acceptor< Stream, Tcp >( boost::lexical_cast< unsigned short >( v[1] ), mode ) );
    }
    else if( v[0] == "udp" )
    {
        COMMA_THROW( comma::exception, "udp: todo" );
    }
    else if( v[0] == "local" )
    {
#ifndef WIN32
        if( v.size() != 2 ) { COMMA_THROW( comma::exception, "expected local socket, got " << name ); }
        _acceptor.reset( new socket_acceptor< Stream, local >( v[1], mode ) );
#endif
    }
    else if( v[0].substr( 0, 4 ) == "zero" )
    {
        _acceptor.reset( new zero_acceptor_< Stream >( name, mode ) );
    }
    else
    {
        if( name == "-" )
        {
            streams_.insert( std::unique_ptr< Stream >( new Stream( name, mode ) ) );
#ifndef WIN32
            if( stream_traits< Stream >::is_input_stream ) { select_.read().add( 0 ); }
            if( stream_traits< Stream >::is_output_stream ) { select_.write().add( 1 ); }
#endif
        }
        else
        {
            _acceptor.reset( new file_acceptor< Stream >( name, mode ) );
            Stream* s = _acceptor->accept( boost::posix_time::time_duration() );
            streams_.insert( std::unique_ptr< Stream >( s ) ); // todo: should we simply abolish file_acceptor and do it in the same way as for stdout?
            if( s->fd() == comma::io::invalid_file_descriptor ) { COMMA_THROW( comma::exception, "failed to open '" << name << "'" ); }
#ifndef WIN32
            if( stream_traits< Stream >::is_input_stream ) { select_.read().add( s->fd() ); }
            if( stream_traits< Stream >::is_output_stream ) { select_.write().add( s->fd() ); }
#endif
        }
    }
}

template < typename Stream > void server< Stream >::close()
{
    if( _acceptor ) { _acceptor->close(); }
    disconnect_all();
}

template < typename Stream > void server< Stream >::disconnect_all()
{
    while( streams_.begin() != streams_.end() ) { remove_( streams_.begin() ); }
}

template < typename Stream > std::vector< Stream* > server< Stream >::accept()
{
    std::vector< Stream* > streams;
    if( !_acceptor ) { return streams; }
    while( true ) // while( streams_.size() < maxSize ?
    {
        Stream* s = _acceptor->accept();
        if( s == nullptr ) { return streams; }
        streams.emplace_back( s );
        streams_.insert( std::unique_ptr< Stream >( s ) );
        if( stream_traits< Stream >::is_input_stream ) { select_.read().add( s->fd() ); }
        if( stream_traits< Stream >::is_output_stream ) { select_.write().add( s->fd() ); }
    }
}

template < typename Stream > void server< Stream >::remove_( typename _streams_type::iterator it )
{
    if( stream_traits< Stream >::is_input_stream ) { select_.read().remove( **it ); }
    if( stream_traits< Stream >::is_output_stream ) { select_.write().remove( **it ); }
    ( *it )->close();
    if( _acceptor ) { _acceptor->notify_closed(); }
    streams_.erase( it );
}

template < typename Stream > std::size_t server< Stream >::size() const { return streams_.size(); }

template < typename Stream > unsigned int server< Stream >::write( server< io::ostream >* s, const char* buf, std::size_t size, bool do_accept )
{
    if( do_accept ) { s->accept(); }
    if( !s->blocking_ ) { s->select_.check(); } // todo: if slow, put all the files in one select
    unsigned int count = 0;
    for( auto i = s->streams_.begin(); i != s->streams_.end(); )
    {
        auto it = i++;
        if( !s->blocking_ && !s->select_.write().ready( **it ) ) { continue; }
        ( **it )->write( buf, size );
        if( s->flush_ ) { ( **it )->flush(); }
        if( ( **it )->good() ) { ++count; }
        else { s->remove_( it ); }
    }
    return count;
}

template struct acceptor< io::istream >;
template struct acceptor< io::ostream >;
// template struct acceptor< io::iostream >;
template struct server< io::istream >;
template struct server< io::ostream >;
// todo: template struct server< io::iostream >;

} } } // namespace comma { namespace io { namespace impl {
