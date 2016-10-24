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

#pragma once

#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "../../containers/cyclic_buffer.h"
#include "../../io/publisher.h"

namespace comma { namespace io { namespace applications {

class publish
{
    public:
        typedef boost::ptr_vector< io::publisher > publishers_t;
        publish( const std::vector< std::string >& file_names, unsigned int n = 10u, unsigned int c = 10u, unsigned int packet_size = 0, bool discard = true, bool flush = true );
        ~publish();
        bool read_line();
        bool read_bytes();
        const publishers_t& publishers() const { return publishers_; }

    private:
        std::size_t write_(const char* buffer, std::size_t size);
        unsigned int push_(const std::string& line);
        unsigned int push_(char* buffer, std::size_t size);
        void pop();
        bool is_binary_() { return packet_size_ > 0; }
        void accept_();
        
        publishers_t publishers_;
        io::select select_;
        boost::scoped_ptr< comma::cyclic_buffer< std::string > > line_buffer_;
        boost::scoped_ptr< comma::cyclic_buffer< std::vector<char> > > char_buffer_;
        std::vector<char> packet_; // for reassembly of packets
        unsigned int packet_offset_; // for reassembly of packets
        unsigned int packet_size_;
        unsigned long int packet_counter_;
        unsigned long int first_discarded_;
        bool buffer_discarding_;
        bool discard_;
};

} } } /// namespace comma { namespace io { namespace applications {
