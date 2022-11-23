// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#include "../base/exception.h"
#include "../csv/names.h"
#include "../string/split.h"

namespace comma { namespace csv {

bool fields_exist( const std::vector< std::string >& fields, const std::vector< std::string >& subset, bool allow_empty )
{
    for( unsigned int i = 0; i < subset.size(); ++i )
    {
        if( allow_empty && subset[i].empty() ) { continue; }
        bool found = false;
        for( unsigned int j = 0; !found && j < fields.size(); found = fields[j] == subset[i], ++j );
        if( !found ) { return false; }
    }
    return true;
}

bool fields_exist( const std::string& fields, const std::string& subset, char delimiter, bool allow_empty ) { return fields_exist( comma::split( fields, delimiter, true ), comma::split( subset, delimiter, true ), allow_empty ); }

} } // namespace comma { namespace csv {
