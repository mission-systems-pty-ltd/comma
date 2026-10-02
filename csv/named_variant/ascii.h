// Copyright (c) 2026 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#include <string>
#include <unordered_map>
#include <vector>
#include "../../base/optional.h"

namespace comma { namespace csv { namespace named_variant { namespace ascii {

template < typename V >
class ascii
{
    public:
        using value_type = V;

        comma::optional< V > get( const std::string& s, char delimiter = ',' );
        comma::optional< V > get( const std::vector< char >& v, char delimiter = ',' );
        comma::optional< V > get( const char* buf, char delimiter = ',' );
        comma::optional< V > get( const std::string& s, bool by_name = false, char delimiter = ',' );
        comma::optional< V > get( const std::vector< char >& v, bool by_name = false, char delimiter = ',' );
        comma::optional< V > get( const char* buf, bool by_name = false, char delimiter = ',' );

    private:
        // todo: std::unordered_map< unsigned int,  >
};

// todo: put

template < typename V > inline comma::optional< V > ascii< V >::get( const std::string& s, char delimiter ) { return this->get( &s[0], false, delimiter ); }
template < typename V > inline comma::optional< V > ascii< V >::get( const std::vector< char >& v, char delimiter ) { return this->get( &v[0], false, delimiter ); }
template < typename V > inline comma::optional< V > ascii< V >::get( const char* buf, char delimiter ) { return this->get( buf, false, delimiter ); } 
template < typename V > inline comma::optional< V > ascii< V >::get( const std::string& s, bool by_name, char delimiter ) { return this->get( &s[0], by_name, delimiter ); }
template < typename V > inline comma::optional< V > ascii< V >::get( const std::vector< char >& v, bool by_name, char delimiter ) { return this->get( &v[0], by_name, delimiter ); }
template < typename V > inline comma::optional< V > ascii< V >::get( const char* buf, bool by_name, char delimiter )
{
    // todo
    return {};
}
    
} } } } // namespace comma { namespace csv { namespace named_variant { namespace ascii {
