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

        // todo!
        unsigned int write( const char* buf, std::size_t size, bool do_accept = true );

        // todo!
        template < typename T >
        impl::server< Stream >& operator<<( const T& lhs ) // quick and dirty, inefficient, but then ascii is meant to be slow...
        {
            accept();
            select_.check();
            unsigned int count = 0;
            for( typename _streams_type::iterator i = streams_.begin(); i != streams_.end(); )
            {
                typename _streams_type::iterator it = i++;
                if( !blocking_ && !select_.write().ready( **it ) ) { continue; }
                ( ***it ) << lhs;
                if( flush_ ) { ( **it )->flush(); }
                if( ( **it )->good() ) { ++count; }
                else { remove_( it ); }
            }
            return *this;
        }

        void close();
        
        void disconnect_all();

        std::size_t size() const;

        std::vector< Stream* > accept(); // quick and dirty; return naked pointers for now
        
        const io::impl::acceptor< Stream >& acceptor() const { return *_acceptor; }

    private:
        template < typename > friend class comma::io::server;
        bool blocking_;
        bool flush_;
        boost::scoped_ptr< io::impl::acceptor< Stream > > _acceptor;
        typedef std::set< std::unique_ptr< stream_type > > _streams_type;
        _streams_type streams_;
        io::select select_;
        void remove_( typename _streams_type::iterator it );
};

} } } // namespace comma { namespace io { namespace impl {
