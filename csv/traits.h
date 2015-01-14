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

#ifndef COMMA_CSV_TRAITS_H_
#define COMMA_CSV_TRAITS_H_

#include <comma/csv/options.h>
#include <comma/visiting/traits.h>

namespace comma { namespace visiting {

template <> struct traits< comma::csv::options >
{
    template < typename Key, class Visitor >
    static void visit( const Key&, const comma::csv::options& p, Visitor& v )
    {
        v.apply( "filename", p.filename );
        v.apply( "delimiter", p.delimiter );
        v.apply( "fields", p.fields );
        v.apply( "full-xpath", p.full_xpath );
        v.apply( "precision", p.precision );
        v.apply( "quote", p.quote ? std::string( 1, *p.quote ) : std::string() );
        v.apply( "flush", p.flush );
        if( p.binary() ) { v.apply( "binary", p.format().string() ); }
        
    }

    template < typename Key, class Visitor >
    static void visit( Key, comma::csv::options& p, Visitor& v )
    {
        v.apply( "filename", p.filename );
        v.apply( "delimiter", p.delimiter );
        v.apply( "fields", p.fields );
        v.apply( "full-xpath", p.full_xpath );
        v.apply( "precision", p.precision );
        std::string quote = p.quote ? std::string( 1, *p.quote ) : std::string();
        v.apply( "quote", p.quote );
        switch( quote.size() )
        {
            case 0: p.quote.reset(); break;
            case 1: p.quote = quote[0]; break;
            case 2: COMMA_THROW( comma::exception, "expected a quote character, got \"" << quote << "\"" );
        }
        v.apply( "flush", p.flush );
        std::string s;
        v.apply( "binary", s );
        if( s != "" ) { p.format( s ); }
    }
};

} } // namespace comma { namespace visiting {

#endif // COMMA_CSV_TRAITS_H_
