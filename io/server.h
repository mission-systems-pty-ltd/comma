// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2024 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#pragma once

#include <stdlib.h>
#include <memory>
#include <string>
#include "stream.h"
#include "impl/server.h"

namespace comma { namespace io {

/// a simple server base class that opens and reads from or writes to streams using services (e.g. tcp, udp, etc)
template < typename Stream >
class server
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

        /// stop accepting clients, disconnect all existing clients
        void close();
        
        /// disconnect all existing clients
        void disconnect_all();

        /// return current number of connected clients
        std::size_t size() const;

        /// return acceptor file descriptor
        file_descriptor acceptor_file_descriptor() const;

        /// publish to all existing connections (blocking)
        /// @note data integrity is the user's responsibility
        ///       i.e. if someone writes:
        ///           server p( "tcp:localhost:1234" );
        ///           p << 1 << "," << 2 << std::endl;
        ///       and a client connects after "1" already
        ///       has been output, this client will receive
        ///       ",2", which most likely was not intended
        std::vector< Stream* > accept(); // quick and dirty, use nacked pointers for now
        
    protected:
        server( const server& );
        server& operator=( const server& );
        impl::server< Stream >* pimpl_;
};

struct oserver: public io::server< io::ostream >
{
    /// @param name ::= tcp:<port> | udp:<port> | <filename>
    ///     if tcp:<port>, create tcp server
    ///     @todo if udp:<port>, broadcast on udp
    ///     if <filename> is a regular file, just write to it
    ///     @todo if <filename> is named pipe, keep reopening it, if closed
    ///     if <filename> is Linux domain socket, create Linux domain socket server
    /// @param mode ascii or binary, a hint for Windows
    /// @param blocking if true, blocking write to a client, otherwise discard, if client not ready
    oserver( const std::string& name, comma::io::mode::value mode, bool blocking = false, bool flush = true ): io::server< io::ostream >( name, mode, blocking, flush ) {}

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
    oserver& operator<<( const T& rhs ) { io::impl::server< io::ostream >::write( pimpl_, rhs ); return *this; }
};

struct iserver: public io::server< io::istream >
{
    /// @param name ::= tcp:<port> | udp:<port> | <filename>
    ///     if tcp:<port>, create tcp server
    ///     @todo if udp:<port>, broadcast on udp
    ///     if <filename> is a regular file, just write to it
    ///     @todo if <filename> is named pipe, keep reopening it, if closed
    ///     if <filename> is Linux domain socket, create Linux domain socket server
    /// @param mode ascii or binary, a hint for Windows
    /// @param blocking if true, blocking write to a client, otherwise discard, if client not ready
    iserver( const std::string& name, comma::io::mode::value mode, bool blocking = false ): io::server< io::istream >( name, mode, blocking ) {}

    /// read <size> bytes to <buf> from first available client
    /// return 0 if no clients have data
    /// start from the next client on the next read to assure round-robin behaviour
    std::size_t read( char* buf, std::size_t size, bool do_accept = true );

    /// read eol-terminated string from first available client
    /// return 0 if no clients have data
    /// start from the next client on the next read to assure round-robin behaviour
    std::string readline( bool do_accept = true );

    std::size_t max_available() const;
};

// struct ioserver: public io::server< io::iostream >
// {
//     // todo
// };

} } // namespace comma { namespace io {
