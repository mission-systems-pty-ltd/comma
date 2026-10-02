// Copyright (c) 2026 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#include <string>
#include <vector>
#include "../../base/optional.h"

namespace comma { namespace csv { namespace named_variant { namespace ascii {

template < typename V > comma::optional< V > get( const std::string& s, char delimiter = ',' );
template < typename V > comma::optional< V > get( const std::vector< char >& v, char delimiter = ',' );
template < typename V > comma::optional< V > get( const char* buf, char delimiter = ',' );
template < typename V > comma::optional< V > get( const std::string& s, bool by_name = false, char delimiter = ',' );
template < typename V > comma::optional< V > get( const std::vector< char >& v, bool by_name = false, char delimiter = ',' );
template < typename V > comma::optional< V > get( const char* buf, bool by_name = false, char delimiter = ',' );

// todo: put

template < typename V > inline comma::optional< V > get( const std::string& s, char delimiter ) { return ascii::get< V >( &s[0], false, delimiter ); }
template < typename V > inline comma::optional< V > get( const std::vector< char >& v, char delimiter ) { return ascii::get< V >( &v[0], false, delimiter ); }
template < typename V > inline comma::optional< V > get( const char* buf, char delimiter ) { return ascii::get< V >( buf, false, delimiter ); } 
template < typename V > inline comma::optional< V > get( const std::string& s, bool by_name, char delimiter ) { return ascii::get< V >( &s[0], by_name, delimiter ); }
template < typename V > inline comma::optional< V > get( const std::vector< char >& v, bool by_name, char delimiter ) { return ascii::get< V >( &v[0], by_name, delimiter ); }
template < typename V > inline comma::optional< V > get( const char* buf, bool by_name, char delimiter )
{
    // todo
    return {};
}
    
} } } } // namespace comma { namespace csv { namespace named_variant { namespace ascii {
