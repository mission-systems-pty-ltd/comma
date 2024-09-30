// Copyright (c) 2011 The University of Sydney
// All rights reserved.

/// @author cedric wohlleber

#pragma once

#include <set>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include "../file_descriptor.h"
#include "../select.h"
#include "../stream.h"

namespace comma { namespace io {

template < typename Stream > class server;

} } // namespace comma { namespace io {

namespace comma { namespace io { namespace impl {

template < typename Stream >
struct acceptor
{
    typedef Stream stream_type;

    virtual ~acceptor() {}
    virtual io::file_descriptor fd() const = 0;
    virtual Stream* accept( boost::posix_time::time_duration timeout = boost::posix_time::seconds( 0 ) ) = 0;
    virtual void notify_closed() {} // quick and dirty
    virtual void close() {}
};

template < typename Stream >
class server
{
    public:
        typedef Stream stream_type;

        server( const std::string& name, io::mode::value mode, bool blocking = false, bool flush = true );

        void close();
        
        void disconnect_all();

        std::size_t size() const;

        std::vector< Stream* > accept(); // quick and dirty; return naked pointers for now
        
        const io::impl::acceptor< Stream >& acceptor() const { return *_acceptor; }

        static unsigned int write( server< io::ostream >* s, const char* buf, std::size_t size, bool do_accept = true );

        template < typename T >
        static void write( server< io::ostream >* s, const T& lhs ) // quick and dirty, inefficient, but then ascii is meant to be slow...
        {
            s->accept();
            s->select_.check();
            unsigned int count = 0;
            for( typename _streams_type::iterator i = s->streams_.begin(); i != s->streams_.end(); )
            {
                typename _streams_type::iterator it = i++;
                if( !s->blocking_ && !s->select_.write().ready( **it ) ) { continue; }
                ( ***it ) << lhs;
                if( s->flush_ ) { ( **it )->flush(); }
                if( ( **it )->good() ) { ++count; }
                else { s->_remove( it ); }
            }
        }

        static unsigned int read( server< io::istream >* s, char* buf, std::size_t size, bool do_accept = true );

        static std::string readline( server< io::istream >* s, bool do_accept = true );

    protected:
        template < typename > friend class comma::io::server;
        template < typename > friend class comma::io::impl::server;
        bool blocking_;
        bool flush_;
        boost::scoped_ptr< io::impl::acceptor< Stream > > _acceptor;
        typedef std::set< std::unique_ptr< Stream > > _streams_type;
        io::file_descriptor _last_read{io::invalid_file_descriptor}; // quick and dirty
        _streams_type streams_;
        io::select select_;
        void _remove( typename _streams_type::iterator it );
        void _remove_bad();
};

} } } // namespace comma { namespace io { namespace impl {
