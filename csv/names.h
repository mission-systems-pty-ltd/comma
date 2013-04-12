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
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by the The University of Sydney.
// 4. Neither the name of the The University of Sydney nor the
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

#ifndef COMMA_CSV_NAMES_H_
#define COMMA_CSV_NAMES_H_

#include <comma/csv/impl/to_names.h>
#include <comma/string/string.h>
#include <comma/visiting/apply.h>
#include <comma/xpath/xpath.h>

namespace comma { namespace csv {

/// the most generic way: return default column names for
/// a given sample and given subtree in comma-separated xpaths
template < typename S >
std::vector< std::string > names( const std::string& paths, bool useFullxpath = true, const S& sample = S() );

/// return default column names for a given sample and given subtree in comma-separated xpaths
template < typename S >
std::vector< std::string > names( const char* paths, bool useFullxpath = true, const S& sample = S() ) { return names( std::string( paths ), useFullxpath, sample ); }

/// return default column names for a given sample
template < typename S >
std::vector< std::string > names( bool useFullxpath, const S& sample = S() ) { return names( "", useFullxpath, sample ); }

/// return default column names for a given sample, use full xpath
template < typename S >
std::vector< std::string > names( const S& sample = S() ) { return names( true, sample ); }

/// return true, if all the fields from subset present in fields
/// @todo make a generic subset application
bool fields_exist( const std::vector< std::string >& fields, const std::vector< std::string >& subset );
bool fields_exist( const std::string& fields, const std::string& subset, char delimiter = ',' );

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

} } // namespace comma { namespace csv {

#endif // COMMA_CSV_NAMES_H_
