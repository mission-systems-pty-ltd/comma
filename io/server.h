// Copyright (c) 2011 The University of Sydney
// All rights reserved.

/// @author vsevolod vlaskine

#pragma once

#include <stdlib.h>
#include <string>
#include <boost/noncopyable.hpp>
#include "stream.h"
#include "impl/server.h"

namespace comma { namespace io {

/// a simple publisher that opens and writes to using services (e.g. tcp, udp, etc)
class server : public boost::noncopyable
{
    public:
        /// constructor
        /// @param name ::= tcp:<port> | udp:<port> | <filename>
        ///     if tcp:<port>, create tcp server
        ///     @todo if udp:<port>, broadcast on udp
        ///     if <filename> is a regular file, just write to it
        ///     @todo if <filename> is named pipe, keep reopening it, if closed
        ///     if <filename> is Linux domain socket, create Linux domain socket server
        /// @param mode ascii or binary, a hint for Windows
        /// @param blocking if true, blocking write to a client, otherwise discard, if client not ready
        server( const std::string& name, io::mode::value mode, bool blocking = false, bool flush = true );

        /// destructor
        ~server();

        /// publish to all existing connections (blocking), return number of clients with successful write
        std::size_t write( const char* buf, std::size_t size, bool do_accept = true );

        /// publish to all existing connections (blocking)
        /// @note data integrity is the user's responsibility
        ///       i.e. if someone writes:
        ///           server p( "tcp:localhost:1234" );
        ///           p << 1 << "," << 2 << std::endl;
        ///       and a client connects after "1" already
        ///       has been output, this client will receive
        ///       ",2", which most likely was not intended
        template < typename T >
        server& operator<<( const T& rhs ) { pimpl_->operator<<( rhs ); return *this; }

        /// close
        void close();
        
        /// disconnect all existing clients
        void disconnect_all();

        /// return current number of connected clients
        std::size_t size() const;

        /// accept waiting clients, non-blocking
        /// @return number of clients accepted
        std::vector< io::ostream* > accept();
        
        /// return acceptor file descriptor
        file_descriptor acceptor_file_descriptor() const;
        
    private:
        impl::server* pimpl_;
};

} } // namespace comma { namespace io {
