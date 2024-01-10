// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#pragma once

#include <algorithm>
#include <vector>
#include <unordered_map>
#include "../csv/impl/to_names.h"
#include "../string/string.h"
#include "../visiting/apply.h"
#include "../xpath/xpath.h"

namespace comma { namespace csv {

/// the most generic way: return default column names for
/// a given sample and given subtree in comma-separated xpaths
template < typename S > std::vector< std::string > names( const std::string& paths, bool useFullxpath = true, const S& sample = S() );

/// return default column names for a given sample and given subtree in comma-separated xpaths
template < typename S > std::vector< std::string > names( const char* paths, bool useFullxpath = true, const S& sample = S() ) { return names( std::string( paths ), useFullxpath, sample ); }

/// return default column names for a given sample
template < typename S > std::vector< std::string > names( bool useFullxpath, const S& sample = S() ) { return names( "", useFullxpath, sample ); }

/// return default column names for a given sample, use full xpath
template < typename S > std::vector< std::string > names( const S& sample = S() ) { return names( true, sample ); }

template < typename S > std::unordered_map< std::string, std::string > leaves( const std::string& paths, const S& sample = S() );
template < typename S > std::unordered_map< std::string, std::string > leaves( const char* paths, const S& sample = S() ) { return leaves( std::string( paths ), sample ); }
template < typename S > std::unordered_map< std::string, std::string > leaves( const S& sample = S() ) { return leaves( "", sample ); }

/// return true, if all the fields from subset present in fields
/// @todo make a generic subset application
bool fields_exist( const std::vector< std::string >& fields, const std::vector< std::string >& subset, bool allow_empty = false );
bool fields_exist( const std::string& fields, const std::string& subset, char delimiter = ',', bool allow_empty = false );

template < typename S >
inline std::vector< std::string > names( const std::string& paths, bool useFullxpath, const S& sample )
{
    std::vector< std::string > p = split( paths, ',' );
    std::vector< std::string > r;
    for( std::size_t i = 0; i < p.size(); ++i )
    {
        if( p.size() > 1 && p[i] == "" )
        {
            r.push_back( "" );
        }
        else
        {
            impl::to_names v( p[i], useFullxpath );
            visiting::apply( v, sample );
            if( v().empty() ) { r.push_back( p[i] ); } // unknown name, don't replace
            else { r.insert( r.end(), v().begin(), v().end() ); }
        }
    }
    return r;
}

template < typename S >
inline std::unordered_map< std::string, std::string > leaves( const std::string& paths, const S& sample )
{
    const auto& flat = names< S >( paths, false, sample );
    const auto& full = names< S >( paths, true, sample );
    std::unordered_map< std::string, std::string > m;
    std::transform( flat.begin(), flat.end(), full.begin(), std::inserter( m, m.end() ), []( const std::string& k, const std::string& v ) { return std::make_pair( k, v ); } );
    return m;
}

} } // namespace comma { namespace csv {
