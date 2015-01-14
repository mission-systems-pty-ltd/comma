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

/// @author vsevolod vlaskine

#include <comma/base/exception.h>
#include <comma/string/string.h>
#include "./fieldwise.h"

namespace comma { namespace csv {

fieldwise::fieldwise( const csv::options& o ) : ascii_( this ), binary_( this ) { init_( o, split( o.fields, ',' ) ); }

fieldwise::fieldwise( const std::string& fields, char delimiter )
    : ascii_( this )
    , binary_( this )
{
    csv::options o;
    o.fields = fields;
    o.delimiter = delimiter;
    init_( o, split( fields, ',' ) );
}

void fieldwise::init_( const csv::options& o, const std::vector< std::string >& fields )
{
    delimiter_ = o.delimiter;
    if( o.binary() ) { format_ = o.format(); }
    for( unsigned int i = 0; i < fields.size(); ++i ) { if( !fields[i].empty() ) { indices_.push_back( i ); } }
}

bool fieldwise::ascii_t::equal( const std::string& lhs, const std::string& rhs ) const
{
    if( f_->format_ ) { COMMA_THROW( comma::exception, "is binary, but asked compare ascii data" ); } // unnecessary limiting, but semantically consistent
    const std::vector< std::string >& left = comma::split( lhs, f_->delimiter_ );
    const std::vector< std::string >& right = comma::split( rhs, f_->delimiter_ );
    for( unsigned int i = 0; i < f_->indices_.size(); ++i ) { if( left[ f_->indices_[i] ] != right[ f_->indices_[i] ] ) { return false; } }
    return true;
}

bool fieldwise::binary_t::equal( const char* lhs, const char* rhs ) const
{
    if( !f_->format_ ) { COMMA_THROW( comma::exception, "is ascii, but asked compare binary data" ); }
    for( unsigned int i = 0; i < f_->indices_.size(); ++i )
    {
        const csv::format::element& e = f_->format_->offset( f_->indices_[i] );
        if( ::memcmp( lhs + e.offset, rhs + e.offset, e.size ) != 0 ) { return false; }
    }
    return true;
}

} } // namespace comma { namespace csv {
