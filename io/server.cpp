// This file is part of comma, a generic and flexible library
// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2024 Vsevolod Vlaskine
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

#include "server.h"

namespace comma { namespace io {

template < typename Stream > server< Stream >::server( const std::string& name, comma::io::mode::value mode, bool blocking, bool flush ) : pimpl_( new impl::server< Stream >( name, mode, blocking, flush ) ) {}

template < typename Stream > server< Stream >::~server() { delete pimpl_; }

template < typename Stream > std::vector< Stream* > server< Stream >::accept() { return pimpl_->accept(); }

template < typename Stream > void server< Stream >::close() { pimpl_->close(); }

template < typename Stream > void server< Stream >::disconnect_all() { pimpl_->disconnect_all(); }

template < typename Stream > std::size_t server< Stream >::size() const { return pimpl_->size(); }

template < typename Stream > file_descriptor server< Stream >::acceptor_file_descriptor() const { return pimpl_->_acceptor ? pimpl_->acceptor().fd() : comma::io::invalid_file_descriptor; }

template < typename Stream > const io::select& server< Stream >::select() const { return pimpl_->select_; }

std::size_t oserver::write( const char* buf, std::size_t size, bool do_accept ) { return io::impl::server< io::ostream >::write( pimpl_, buf, size, do_accept ); }

std::size_t iserver::read( char* buf, std::size_t size, bool do_accept ) { return io::impl::server< io::istream >::read( pimpl_, buf, size, do_accept ); }

std::string iserver::getline( bool do_accept ) { return io::impl::server< io::istream >::getline( pimpl_, do_accept ); }

std::size_t iserver::available_at_least() const { return io::impl::server< io::istream >::available_at_least( pimpl_ ); }

template class server< io::istream >;
template class server< io::ostream >;
// todo: template class server< io::iostream >;

} } // namespace comma { namespace io {
