// Copyright (c) 2023 Mission Systems Pty Ltd
// All rights reserved.

/// @author dave jennings

#pragma once

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <comma/base/exception.h>

namespace comma { namespace io { namespace serial {

class port
{
    public:
        port( const std::string& name );

        /// open port as 8N1
        port( const std::string& name, unsigned int baud_rate );

        void set_baud_rate( unsigned int baud_rate );

        void set_character_size( unsigned int character_size );

        void set_flow_control( boost::asio::serial_port_base::flow_control::type flow_control );

        void set_parity( boost::asio::serial_port_base::parity::type parity );

        void set_stop_bits( boost::asio::serial_port_base::stop_bits::type stop_bits );

        std::size_t read_some( char* buf, std::size_t buf_size );

        std::size_t read_some( unsigned char* buf, std::size_t buf_size );

        std::size_t read_some( char* buf, std::size_t buf_size, const boost::asio::deadline_timer::duration_type& timeout );

        std::size_t read_some( unsigned char* buf, std::size_t buf_size, const boost::asio::deadline_timer::duration_type& timeout );

        std::size_t write( const char* buf, std::size_t to_write );

        std::size_t write( const unsigned char* buf, std::size_t to_write );

    private:
        boost::asio::io_service _service; // renamed as io_context in Boost 1.66 (io_service remains as typedef)
        boost::asio::serial_port _port;
};

} } } // namespace comma { namespace io { namespace serial {
