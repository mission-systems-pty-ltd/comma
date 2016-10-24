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

#ifdef WIN32
#include <io.h>
#endif

#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "../../base/last_error.h"
#include "../../io/file_descriptor.h"
#include "publish.h"

namespace comma { namespace io { namespace applications {

publish::publish( const std::vector< std::string >& filenames, unsigned int n, unsigned int c, unsigned int packet_size, bool discard, bool flush )
    : packet_( packet_size )
    , packet_offset_( 0U )
    , packet_size_( packet_size )
    , packet_counter_( 0U )
    , buffer_discarding_( false )
{
    if( is_binary_() ) { if( c > 0 ) { char_buffer_.reset( new comma::cyclic_buffer< std::vector< char > >( c ) ); } }
    else { if( n > 0 ) { line_buffer_.reset( new comma::cyclic_buffer< std::string >( n ) ); } }
    // redirect SIGPIPE to SIG_IGN so that it does not kill the application when it tries to write on a closed pipe
#ifndef WIN32
    struct sigaction new_action, old_action;
    new_action.sa_handler = SIG_IGN;
    sigemptyset(&new_action.sa_mask);
    sigaction(SIGPIPE, NULL, &old_action);
    sigaction(SIGPIPE, &new_action, NULL);
    select_.read().add( stdin_fd );
#endif
    for( std::size_t i = 0; i < filenames.size(); ++i ) { publishers_.push_back( new io::publisher( filenames[i], is_binary_() ? io::mode::binary : io::mode::ascii, !discard, flush ) ); }
    for( std::size_t i = 0; i < publishers_.size(); ++i ) { if( publishers_[i].acceptor_file_descriptor() != io::invalid_file_descriptor ) { select_.read().add( publishers_[i].acceptor_file_descriptor() ); } }
}

publish::~publish() { for( std::size_t i = 0; i < publishers_.size(); ++i ) { publishers_[i].close(); } }

void publish::accept_() { for( std::size_t i = 0; i < publishers_.size(); ++i ) { if( select_.read().ready( publishers_[i].acceptor_file_descriptor() ) ) { publishers_[i].accept(); } } }

bool publish::read_line()
{
    if( is_binary_() ) { COMMA_THROW( comma::exception, "read_line() called in binary publisher" ); } // todo: just have separate classes for ascii and binary readers
    if( line_buffer_ && line_buffer_->empty() ) { select_.wait(); }
    accept_(); // todo: does it even work?
    if( !line_buffer_ || line_buffer_->empty() || select_.read().ready( stdin_fd ) )
    {
        std::string line;
        std::getline( std::cin, line );
        if( std::cin.good() )
        {
            line += '\n';
            push_( line );
        }
    }
    while( line_buffer_ && !line_buffer_->empty() ) // pop the buffer
    {
        if( write_( line_buffer_->front().data(), line_buffer_->front().size() ) ) { line_buffer_->pop(); }
    }
    return true;
}

bool publish::read_bytes()
{
    if( !is_binary_() ) { COMMA_THROW( comma::exception, "read_line() called in binary publisher" ); } // todo: just have separate classes for ascii and binary readers
    if( char_buffer_ && char_buffer_->empty() ) { select_.wait(); }
    accept_(); // todo: does it even work?
    if( !char_buffer_ || char_buffer_->empty() || select_.read().ready( stdin_fd ) )
    {
        std::vector< char > w( packet_size_ );
        char* buf = &w[0];
#ifndef WIN32
        ::ssize_t gcount = ::read( 0, buf, packet_size_ );
#else
        int gcount = _read( 0, buf, packet_size_ );
#endif
        if( gcount <= 0 ) { return false; } // i.e. end of file
        push_( buf, gcount );
    }
    while( char_buffer_ && !char_buffer_->empty() ) // pop the buffer packet by packet
    {
        if( write_( &char_buffer_->front()[0], packet_size_ ) ) { char_buffer_->pop(); }
    }
    return true;
}

std::size_t publish::write_( const char* buffer, std::size_t size )
{
    std::size_t count = 0;
    for( std::size_t i = 0; i < publishers_.size(); ++i ) { count += publishers_[i].write( buffer, size ); }
    return count;
}


unsigned int publish::push_( const std::string& line )
{
    unsigned int written_count = 0; // streams written to
    if( !line_buffer_ || line_buffer_->empty() )
    { 
        written_count = write_( line.data(), line.size() );
    }
    else if( line_buffer_ )
    {
        if( line_buffer_->size() >= line_buffer_->capacity() )
        {
            line_buffer_->pop(); // throw oldest line away
            if( !buffer_discarding_ )
            {
                buffer_discarding_ = true;
                std::cerr << "io-publish: started discarding on packet " << packet_counter_ << std::endl;
                first_discarded_ = packet_counter_;
            }
        }
        line_buffer_->push( line );
    }
    return written_count;
}

unsigned int publish::push_( char* buffer, std::size_t size )
{
    // fill out the packet buffer
    char* p = buffer;
    bool full_packet = ( packet_offset_ == 0U ) && ( size >= packet_size_ );
    char* packet = buffer;
    if( full_packet )
    {
        // can copy whole packet, but no need yet, only if buffering
        p += packet_size_;
        packet_offset_ = packet_size_;
    }
    else
    {
        // packet_ partly filled or not enough data to fill it, fill as much as possible
        for( ; packet_offset_ < packet_size_ && p < buffer + size; ++p )
        {
            packet_[packet_offset_] = *p;
            packet_offset_++;
        }
        packet = &packet_[0];
    }
    unsigned int written_count = 0;
    if( packet_offset_ == packet_size_ )
    {
        // try to write packet buffer to the pipes
        if( !char_buffer_ || char_buffer_->empty() )
        { 
            written_count = write_( packet, packet_size_ );
        }
        else if( char_buffer_ )
        {
            if( char_buffer_->size() >= char_buffer_->capacity() )
            {
                char_buffer_->pop(); // throw oldest packet away
                if( !buffer_discarding_ )
                {
                    buffer_discarding_ = true;
                    std::cerr << "io-publish: started discarding on packet " << packet_counter_ << std::endl;
                    first_discarded_ = packet_counter_;
                }
            }
            if( full_packet ) { ::memcpy( &packet_[0], buffer, packet_size_ ); }
            char_buffer_->push( packet_ );
        }
        packet_offset_ = 0U;
    }
    // handle the remaining bytes
    if( p < buffer + size ) { written_count += push_( p, size - static_cast< std::size_t >( p - buffer ) ); }
    return written_count;
}

} } } /// namespace comma { namespace io { namespace applications {
