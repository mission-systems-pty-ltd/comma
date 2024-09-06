// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2020 Vsevolod Vlaskine
// All rights reserved.

#pragma once

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <deque>
#include <memory>
#include <boost/bind/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include "../../io/file_descriptor.h"
#include "../../io/publisher.h"
#include "../../string/string.h"
#include "../../sync/synchronized.h"

namespace comma { namespace io { namespace impl {

class publish
{
    public:
        typedef comma::synchronized< std::vector< std::unique_ptr< comma::io::publisher > > > publishers_t;
        
        typedef publishers_t::scoped_transaction transaction_t;
        
        struct endpoint
        {
            std::string address;
            bool secondary;
            endpoint( const std::string& address = "", bool secondary = false ): address( address ), secondary( secondary ) {}
        };
        
        publish( const std::vector< std::string >& endpoints
               , unsigned int packet_size
               , bool discard
               , bool flush
               , bool output_number_of_clients
               , bool update_no_clients
               , unsigned int cache_size );
        
        ~publish();
        
        void disconnect_all();
        
        bool read( std::istream& input );

        bool write( const std::string& s );

        bool write( const char* buf, unsigned int size );
        
        unsigned int num_clients() const { return num_clients_; }

    private:
        std::vector< endpoint > endpoints_;
        bool discard_;
        bool flush_;
        publishers_t publishers_;
        std::string buffer_;
        unsigned int packet_size_;
        bool output_number_of_clients_;
        unsigned int cache_size_;
        bool update_no_clients_;
        bool got_first_client_ever_;
        std::vector< unsigned int > sizes_;
        bool has_primary_clients_;
        unsigned int num_clients_;
        boost::scoped_ptr< boost::thread > acceptor_thread_;
        bool is_shutdown_;
        std::deque< std::string > cache_;

        bool is_binary_() const { return packet_size_ > 0; }
        bool handle_sizes_( transaction_t& t ); // todo? why pass transaction? it doen not seem going out of scope at the point of call; remove?
        void accept_();
};

} } } // namespace comma { namespace io { namespace impl {
