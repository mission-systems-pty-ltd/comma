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

#include <sstream>
#include <boost/thread/thread.hpp>
#include "../../../string/string.h"
#include "multiplay.h"

namespace comma { namespace csv { namespace applications { namespace play {

Multiplay::Multiplay( const std::vector< SourceConfig >& configs
                    , double speed
                    , bool quiet
                    , const boost::posix_time::time_duration& resolution
                    , boost::posix_time::ptime from
                    , boost::posix_time::ptime to
                    , bool flush )
    : m_configs( configs )
    , istreams_( configs.size() )
    , _input_streams( configs.size() )
    , _publishers( configs.size() )
    , m_play( speed, quiet, resolution )
    , m_timestamps( configs.size() )
    , m_started( false )
    , m_from( from )
    , m_to( to )
    , ascii_( configs.size() )
    , binary_( configs.size() )
{
    for( unsigned int i = 0; i < configs.size(); i++ )
    {
        // todo: quick and dirty for now: blocking streams for named pipes
        istreams_[i].reset( new io::istream( configs[i].options.filename, m_configs[i].options.binary() ? io::mode::binary : io::mode::ascii, io::mode::blocking ) );
        if( !( *istreams_[i] )() ) { COMMA_THROW( comma::exception, "named pipe " << configs[i].options.filename << " is closed (todo: support closed named pipes)" ); }
        _input_streams[i].reset( new csv::input_stream< time >( *( *istreams_[i] )(), m_configs[i].options ) );
        unsigned int j;
        for( j = 0; j < i && configs[j].outputFileName != configs[i].outputFileName; ++j ); // quick and dirty: unique publishers
        if( j == i ) { _publishers[i].reset( new comma::csv::applications::play::server_publisher( configs[i].outputFileName, m_configs[i].options.binary(), flush ) ); }
        else { _publishers[i] = _publishers[j]; }
        boost::posix_time::time_duration d;
        if( configs[i].offset.total_microseconds() != 0 )
        {
            if( m_configs[i].options.binary() )
            {
                binary_[i].reset( new csv::binary< time >( m_configs[i].options.fields ) );
                _buffer.resize( m_configs[i].options.format().size() );
            }
            else
            {
                ascii_[i].reset( new csv::ascii< time >( m_configs[i].options.fields ) );
            }
        }
    }
}

void Multiplay::close()
{
    for( unsigned int i = 0U; i < m_configs.size(); i++ )
    {
        istreams_[i]->close();
        _publishers[i]->close();
    }
}

namespace impl {
    
static std::string endl()
{
    std::ostringstream oss;
    oss << std::endl;
    return oss.str();
}

} // namespace impl {

bool Multiplay::ready() // quick and dirty; should not it be in io::Publisher?
{
    if( m_started ) { return true; }
    for( unsigned int i = 0; i < m_configs.size(); ++i )
    {
        _publishers[i]->accept();
        if( _publishers[i]->size() < m_configs[i].minNumberOfClients ) { boost::this_thread::sleep( boost::posix_time::millisec( 200 ) ); return false; }
    }
    m_started = true;
    return true;
}
    
/// @brief try to read from all files and write the oldest; return true if at least one file could be read
bool Multiplay::read()
{
    if( !ready() ) { return true; }
    bool end = true;
    for( unsigned int i = 0U; i < m_configs.size(); ++i )
    {
        if( !m_timestamps[i].is_not_a_date_time() ) { end = false; continue; }
        const time* time = _input_streams[i]->read();
        if( time == NULL ) { continue; }
        boost::posix_time::ptime t = time->timestamp;
        if( m_configs[i].offset.total_microseconds() != 0 ) { t += m_configs[i].offset; }
        end = false;
        if( ( ( !m_from.is_not_a_date_time() ) && ( t < m_from ) ) || ( ( !m_to.is_not_a_date_time() ) && ( t > m_to ) ) ) { i--; continue; }
        m_timestamps[i] = t;
    }
    if( end ) { return false; }
    boost::posix_time::ptime oldest;
    std::size_t index = 0;
    for( unsigned int i = 0; i < m_timestamps.size(); ++i )
    {
        if( !oldest.is_not_a_date_time() && ( m_timestamps[i].is_not_a_date_time() || m_timestamps[i] >= oldest ) ) { continue; }
        oldest = m_timestamps[i];
        index = i;
    }
    if( ( ( !m_from.is_not_a_date_time() ) && ( oldest < m_from ) ) || ( ( !m_to.is_not_a_date_time() ) && ( oldest > m_to ) ) ) { return true; }
    now_ = oldest;
    m_play.wait( oldest );
    if( m_configs[index].options.binary() )
    {
        if( binary_[index] )
        {
            ::memcpy( &_buffer[0], _input_streams[index]->binary().last(), _buffer.size() );
            binary_[index]->put( time( oldest ), &_buffer[0] );
            _publishers[index]->write( &_buffer[0], _buffer.size() );
        }
        else
        {
            _publishers[index]->write( _input_streams[index]->binary().last(), m_configs[index].options.format().size() );
        }
    }
    else
    {
        static std::string endl = impl::endl(); // quick and dirty, since publisher is not std::stream
        if( ascii_[index] )
        {
            std::vector< std::string > last = _input_streams[index]->ascii().last();
            ascii_[index]->put( time( oldest ), last );
            _publishers[index]->write_line( comma::join( last, m_configs[index].options.delimiter ) );
        }
        else
        {
            _publishers[index]->write_line( comma::join( _input_streams[index]->ascii().last(), m_configs[index].options.delimiter ) );
        }
    }
    m_timestamps[index] = boost::posix_time::not_a_date_time;
    return true;
}

} } } } // namespace comma { namespace csv { namespace applications { namespace play {
