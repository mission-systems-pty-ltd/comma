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

#ifndef COMMA_CSV_IMPL_FIELDWISE_H_
#define COMMA_CSV_IMPL_FIELDWISE_H_

#include <vector>
#include <boost/optional.hpp>
#include "../../csv/names.h"
#include "../../csv/options.h"

namespace comma { namespace csv {

class fieldwise
{
    public:
        class ascii_t
        {
            public:
                bool equal( const std::string& lhs, const std::string& rhs ) const;
                
            private:
                friend class fieldwise;
                ascii_t( const fieldwise* f ) : f_( f ) {}
                const fieldwise* f_;
        };
        
        class binary_t
        {
            public:
                bool equal( const char* lhs, const char* rhs ) const;
            private:
                friend class fieldwise;
                binary_t( const fieldwise* f ) : f_( f ) {}
                const fieldwise* f_;
        };
        
        fieldwise( const csv::options& o );
        
        fieldwise( const std::string& fields, char delimiter = ',' );
        
        template < typename T >
        fieldwise( const T& sample, const csv::options& o );
        
        template < typename T >
        fieldwise( const T& sample, const std::string& fields, char delimiter = ',', bool full_xpath = true );
        
        const ascii_t& ascii() const { return ascii_; }
        
        const binary_t& binary() const { return binary_; }
        
    private:
        friend class ascii_t;
        friend class binary_t;
        std::vector< unsigned int > indices_;
        boost::optional< csv::format > format_;
        char delimiter_;
        void init_( const csv::options& o, const std::vector< std::string >& fields );
        ascii_t ascii_;
        binary_t binary_;
};

template < typename T >
inline fieldwise::fieldwise( const T& sample, const csv::options& o )
    : ascii_( this )
    , binary_( this )
{
    init_( o, csv::names( o.fields, o.full_xpath, sample ) );
}

template < typename T >
inline fieldwise::fieldwise( const T& sample, const std::string& fields, char delimiter, bool full_xpath )
    : delimiter_( delimiter )
    , ascii_( this )
    , binary_( this )
{
    csv::options o;
    o.fields = fields;
    o.full_xpath = full_xpath;
    o.delimiter = delimiter;
    init_( o, csv::names( fields, full_xpath, sample ) );
}

} } // namespace comma { namespace csv {

#endif // COMMA_CSV_IMPL_FIELDWISE_H_
