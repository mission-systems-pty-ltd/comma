// Copyright (c) 2022 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <fstream>
#include <string>
#include <vector>
#include "impl/filesystem.h"
#include "../base/exception.h"

namespace comma { namespace io {

/// convenience function to load a vector-like contiguous container from file
template < typename T, template < typename S, typename A > class C = std::vector, typename A = std::allocator< T > >
C< T, A >& load_array( C< T, A >& a, const std::string& path );
    
/// convenience function to load a vector-like contiguous container from file
/// @note allocates and returns the container, thus it is up to the user to use move semantics
template < typename T, template < typename S, typename A > class C = std::vector, typename A = std::allocator< T > >
C< T, A > load_array( const std::string& path );


template < typename T, template < typename S, typename A > class C = std::vector, typename A >
inline C< T, A >& load_array( C< T, A >& a, const std::string& path )
{
    std::ifstream ifs( path );
    if( !ifs.is_open() ) { COMMA_THROW( comma::exception, "failed to open \"" << path << "\"" ); }
    a.resize( comma::filesystem::file_size( path ) / sizeof( T ) ); // todo? will file_size work on symlinks?
    ifs.read( reinterpret_cast< char * >( &a[0] ), a.size() * sizeof( T ) );
    if( ifs.gcount() != int( a.size() * sizeof( T ) ) ) { COMMA_THROW( comma::exception, "expected to read " << a.size() * sizeof( T ) << " bytes (" << a.size() << " elements " << sizeof( T ) << " byte(s) each) from \"" << path << "\"; got: " << ifs.gcount() << " byte(s)" ); }
    return a;
}

template < typename T, template < typename S, typename A > class C = std::vector, typename A >
inline C< T, A > load_array( const std::string& path )
{
    C< T, A > a;
    return load_array( a, path );
}
    
} } // namespace comma { namespace io {
