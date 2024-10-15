// Copyright (c) 2023 Mission Systems Pty Ltd
// All rights reserved.

/// @author dave jennings

#include "serial.h"

namespace comma { namespace io { namespace serial {

port::port( const std::string& name ): _port( _service, name ) {}

port::port( const std::string& name, unsigned int baud_rate )
    : _port( _service, name )
{
    set_baud_rate( baud_rate );
    set_character_size( 8 );
    set_flow_control( boost::asio::serial_port_base::flow_control::none );
    set_parity( boost::asio::serial_port_base::parity::none );
    set_stop_bits( boost::asio::serial_port_base::stop_bits::one );
}

void port::set_baud_rate( unsigned int baud_rate ) { _port.set_option( boost::asio::serial_port_base::baud_rate( baud_rate )); }

void port::set_character_size( unsigned int character_size ) { _port.set_option( boost::asio::serial_port_base::character_size( character_size )); }

void port::set_flow_control( boost::asio::serial_port_base::flow_control::type flow_control ) { _port.set_option( boost::asio::serial_port_base::flow_control( flow_control )); }

void port::set_parity( boost::asio::serial_port_base::parity::type parity ) { _port.set_option( boost::asio::serial_port_base::parity( parity )); }

void port::set_stop_bits( boost::asio::serial_port_base::stop_bits::type stop_bits ) { _port.set_option( boost::asio::serial_port_base::stop_bits( stop_bits )); }

std::size_t port::read_some( unsigned char* buf, std::size_t buf_size )
{
    boost::system::error_code ec;
    std::size_t count = _port.read_some( boost::asio::buffer( buf, buf_size ), ec );
    COMMA_ASSERT( !ec, ec.message() );
    return count;
}

std::size_t port::read_some( unsigned char* buf
                           , std::size_t buf_size
                           , const boost::asio::deadline_timer::duration_type& timeout )
{
    boost::optional< boost::system::error_code > timer_result;
    boost::asio::deadline_timer timer( _service );
    timer.expires_from_now( timeout );
    timer.async_wait( [&timer_result]( const boost::system::error_code& error ) { timer_result.reset( error ); });

    boost::optional< boost::system::error_code > read_result;
    std::size_t count = 0;
    _port.async_read_some( boost::asio::buffer( buf, buf_size )
                         , [&read_result, &count]( const boost::system::error_code& error, std::size_t count_ )
                         {
                             read_result.reset( error );
                             count = count_;
                         });
    _service.reset();
    while( _service.run_one() )
    {
        if( read_result ) { timer.cancel(); }
        if( timer_result ) { _port.cancel(); }
    }
    if( *read_result )
    {
        // https://www.boost.org/doc/libs/1_65_1/libs/system/doc/reference.html#Header-error_code
        if( read_result->value() != boost::system::errc::operation_canceled ) { throw boost::system::system_error( *read_result ); }
    }
    return count;
}

std::size_t port::write( const unsigned char* buf, std::size_t to_write ) { return boost::asio::write( _port, boost::asio::buffer( buf, to_write )); }

} } } // namespace comma { namespace io { namespace serial {
