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

/// @author cedric wohlleber

#ifndef COMMA_IO_IMPL_PUBLISHER_H_
#define COMMA_IO_IMPL_PUBLISHER_H_

#include <set>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include "../file_descriptor.h"
#include "../select.h"
#include "../stream.h"

namespace comma { namespace io {
    
class publisher;

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
    
class publisher
{
    public:
        publisher( const std::string& name, io::mode::value mode, bool blocking = false, bool flush = true );

        unsigned int write( const char* buf, std::size_t size, bool do_accept = true );

        template < typename T >
        impl::publisher& operator<<( const T& lhs ) // quick and dirty, inefficient, but then ascii is meant to be slow...
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

        unsigned int accept();
        
        const io::impl::acceptor& acceptor() const { return *acceptor_; }

    private:
        friend class comma::io::publisher;
        bool blocking_;
        bool flush_;
        boost::scoped_ptr< io::impl::acceptor > acceptor_;
        typedef std::set< boost::shared_ptr< io::ostream > > streams;
        streams streams_;
        io::select select_;
        void remove_( streams::iterator it );
};

} } } // namespace comma { namespace io { namespace impl {

#endif // #ifndef COMMA_IO_IMPL_PUBLISHER_H_
