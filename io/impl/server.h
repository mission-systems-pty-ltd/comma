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
    
class server;

} } // namespace comma { namespace io {

namespace comma { namespace io { namespace impl {

struct acceptor
{
    virtual ~acceptor() {}
    virtual io::file_descriptor fd() const = 0;
    virtual io::ostream* accept( boost::posix_time::time_duration timeout = boost::posix_time::seconds( 0 ) ) = 0;
    virtual void notify_closed() {} // quick and dirty
    virtual void close() {}
};
    
class server
{
    public:
        server( const std::string& name, io::mode::value mode, bool blocking = false, bool flush = true );

        unsigned int write( const char* buf, std::size_t size, bool do_accept = true );

        template < typename T >
        impl::server& operator<<( const T& lhs ) // quick and dirty, inefficient, but then ascii is meant to be slow...
        {
            accept();
            select_.check();
            unsigned int count = 0;
            for( streams::iterator i = streams_.begin(); i != streams_.end(); )
            {
                streams::iterator it = i++;
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

        std::vector< io::ostream* > accept();
        
        const io::impl::acceptor& acceptor() const { return *acceptor_; }

    private:
        friend class comma::io::server;
        bool blocking_;
        bool flush_;
        boost::scoped_ptr< io::impl::acceptor > acceptor_;
        typedef std::set< boost::shared_ptr< io::ostream > > streams;
        streams streams_;
        io::select select_;
        void remove_( streams::iterator it );
};

} } } // namespace comma { namespace io { namespace impl {
