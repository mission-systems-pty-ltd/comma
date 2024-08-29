// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#pragma once

#include "../csv/options.h"
#include "../visiting/traits.h"

namespace comma { namespace visiting {

template <> struct traits< comma::csv::options >
{
    template < typename Key, class Visitor >
    static void visit( const Key&, const comma::csv::options& p, Visitor& v )
    {
        v.apply( "filename", p.filename );
        v.apply( "delimiter", p.delimiter );
        v.apply( "fields", p.fields );
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
