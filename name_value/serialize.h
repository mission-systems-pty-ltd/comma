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

#ifndef COMMA_NAME_VALUE_SERIALIZE_H_
#define COMMA_NAME_VALUE_SERIALIZE_H_

#include <fstream>
#include <iostream>
#include <string>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <comma/base/exception.h>
#include <comma/name_value/ptree.h>
#include <comma/xpath/xpath.h>
#include <comma/visiting/apply.h>

namespace comma {

/// read object from json file or stream
/// convenience wrappers for comma::property_tree boiler-plate code
template < typename T > T read_json( const std::string& filename, const xpath& root, bool permissive );
template < typename T > T read_json( const std::string& filename, const char* root, bool permissive );
template < typename T > T read_json( const std::string& filename, const xpath& root );
template < typename T > T read_json( const std::string& filename, const char* root );
template < typename T > T read_json( const std::string& filename, bool permissive );
template < typename T > T read_json( const std::string& filename );
template < typename T > T read_json( std::istream& stream, const xpath& root, bool permissive );
template < typename T > T read_json( std::istream& stream, const char* root, bool permissive );
template < typename T > T read_json( std::istream& stream, const xpath& root );
template < typename T > T read_json( std::istream& stream, const char* root );
template < typename T > T read_json( std::istream& stream, bool permissive );
template < typename T > T read_json( std::istream& stream );

/// read object from xml file or stream
/// convenience wrappers for comma::property_tree boiler-plate code
template < typename T > T read_xml( const std::string& filename, const xpath& root, bool permissive );
template < typename T > T read_xml( const std::string& filename, const char* root, bool permissive );
template < typename T > T read_xml( const std::string& filename, const xpath& root );
template < typename T > T read_xml( const std::string& filename, const char* root );
template < typename T > T read_xml( const std::string& filename, bool permissive );
template < typename T > T read_xml( const std::string& filename );
template < typename T > T read_xml( std::istream& stream, const xpath& root, bool permissive );
template < typename T > T read_xml( std::istream& stream, const char* root, bool permissive );
template < typename T > T read_xml( std::istream& stream, const xpath& root );
template < typename T > T read_xml( std::istream& stream, const char* root );
template < typename T > T read_xml( std::istream& stream, bool permissive );
template < typename T > T read_xml( std::istream& stream );

/// read object from json file or stream
/// convenience wrappers for comma::property_tree boiler-plate code
template < typename T > void write_json( const T& t, const std::string& filename, const xpath& root );
template < typename T > void write_json( const T& t, const std::string& filename, const char* root );
template < typename T > void write_json( const T& t, const std::string& filename );
template < typename T > void write_json( const T& t, std::ostream& stream, const xpath& root );
template < typename T > void write_json( const T& t, std::ostream& stream, const char* root );
template < typename T > void write_json( const T& t, std::ostream& stream );

/// read object from xml file or stream
/// convenience wrappers for comma::property_tree boiler-plate code
template < typename T > void write_xml( const T& t, const std::string& filename, const xpath& root );
template < typename T > void write_xml( const T& t, const std::string& filename, const char* root );
template < typename T > void write_xml( const T& t, const std::string& filename );
template < typename T > void write_xml( const T& t, std::ostream& stream, const xpath& root );
template < typename T > void write_xml( const T& t, std::ostream& stream, const char* root );
template < typename T > void write_xml( const T& t, std::ostream& stream );


template < typename T > inline T read_json( const std::string& filename, const xpath& root, bool permissive )
{
    std::ifstream ifs( &filename[0] );
    if( !ifs.is_open() ) { COMMA_THROW( comma::exception, "failed to open \"" << filename << "\"" ); }
    return read_json< T >( ifs, root, permissive );
}

template < typename T > inline T read_json( std::istream& stream, const xpath& root, bool permissive )
{
    T t;
    boost::property_tree::ptree p;
    boost::property_tree::read_json( stream, p );
    comma::from_ptree from_ptree( p, root, permissive );
    comma::visiting::apply( from_ptree ).to( t );
    return t;
}

template < typename T > inline T read_json( const std::string& filename, const char* root, bool permissive ) { return root ? read_json< T >( filename, xpath( root ), permissive ) : read_json< T >( filename, permissive ); }
template < typename T > inline T read_json( const std::string& filename, const xpath& root ) { return read_json< T >( filename, root, true ); }
template < typename T > inline T read_json( const std::string& filename, const char* root ) { return root ? read_json< T >( filename, xpath( root ), true ) : read_json< T >( filename, true ); }
template < typename T > inline T read_json( const std::string& filename, bool permissive ) { return read_json< T >( filename, xpath(), permissive ); }
template < typename T > inline T read_json( const std::string& filename ) { return read_json< T >( filename, xpath(), true ); }
template < typename T > inline T read_json( std::istream& stream, const char* root, bool permissive ) { return root ? read_json< T >( stream, xpath( root ), permissive ) : read_json< T >( stream, permissive ); }
template < typename T > inline T read_json( std::istream& stream, const xpath& root ) { return read_json< T >( stream, root, true ); }
template < typename T > inline T read_json( std::istream& stream, const char* root ) { return root ? read_json< T >( stream, xpath( root ), true ) : read_json< T >( stream, true ); }
template < typename T > inline T read_json( std::istream& stream, bool permissive ) { return read_json< T >( stream, xpath(), permissive ); }
template < typename T > inline T read_json( std::istream& stream ) { return read_json< T >( stream, xpath(), true ); }

template < typename T > inline T read_xml( const std::string& filename, const xpath& root, bool permissive )
{
    std::ifstream ifs( &filename[0] );
    if( !ifs.is_open() ) { COMMA_THROW( comma::exception, "failed to open \"" << filename << "\"" ); }
    return read_xml< T >( ifs, root, permissive );
}

template < typename T > inline T read_xml( std::istream& stream, const xpath& root, bool permissive )
{
    T t;
    boost::property_tree::ptree p;
    boost::property_tree::read_xml( stream, p );
    comma::from_ptree from_ptree( p, root, permissive );
    comma::visiting::apply( from_ptree ).to( t );
    return t;
}

template < typename T > inline T read_xml( const std::string& filename, const char* root, bool permissive ) { return root ? read_xml< T >( filename, xpath( root ), permissive ) : read_xml< T >( filename, permissive ); }
template < typename T > inline T read_xml( const std::string& filename, const xpath& root ) { return read_xml< T >( filename, root, true ); }
template < typename T > inline T read_xml( const std::string& filename, const char* root ) { return root ? read_xml< T >( filename, xpath( root ), true ) : read_xml< T >( filename, true ); }
template < typename T > inline T read_xml( const std::string& filename, bool permissive ) { return read_xml< T >( filename, xpath(), permissive ); }
template < typename T > inline T read_xml( const std::string& filename ) { return read_xml< T >( filename, xpath(), true ); }
template < typename T > inline T read_xml( std::istream& stream, const char* root, bool permissive ) { return root ? read_xml< T >( stream, xpath( root ), permissive ) : read_xml< T >( stream, permissive ); }
template < typename T > inline T read_xml( std::istream& stream, const xpath& root ) { return read_xml< T >( stream, root, true ); }
template < typename T > inline T read_xml( std::istream& stream, const char* root ) { return root ? read_xml< T >( stream, xpath( root ), true ) : read_xml< T >( stream, true ); }
template < typename T > inline T read_xml( std::istream& stream, bool permissive ) { return read_xml< T >( stream, xpath(), permissive ); }
template < typename T > inline T read_xml( std::istream& stream ) { return read_xml< T >( stream, xpath(), true ); }

template < typename T > inline void write_json( const T& t, const std::string& filename, const xpath& root )
{
    std::ofstream ofs( &filename[0] );
    if( !ofs.is_open() ) { COMMA_THROW( comma::exception, "failed to open \"" << filename << "\"" ); }
    return write_json< T >( ofs, root );
}

template < typename T > inline void write_json( const T& t, std::ostream& stream, const xpath& root )
{
    boost::property_tree::ptree p;
    comma::to_ptree to_ptree( p, root );
    comma::visiting::apply( to_ptree ).to( t );
    boost::property_tree::write_json( stream, p );
}

template < typename T > inline void write_json( const T& t, const std::string& filename, const char* root ) { write_json( t, filename, xpath( root ) ); }
template < typename T > inline void write_json( const T& t, const std::string& filename ) { write_json( t, filename, xpath() ); }
template < typename T > inline void write_json( const T& t, std::ostream& stream, const char* root ) { write_json( t, stream, xpath( root ) ); }
template < typename T > inline void write_json( const T& t, std::ostream& stream ) { write_json( t, stream, xpath() ); }

template < typename T > inline void write_xml( const T& t, const std::string& filename, const xpath& root )
{
    std::ofstream ofs( &filename[0] );
    if( !ofs.is_open() ) { COMMA_THROW( comma::exception, "failed to open \"" << filename << "\"" ); }
    return write_xml< T >( ofs, root );
}

template < typename T > inline void write_xml( const T& t, std::ostream& stream, const xpath& root )
{
    boost::property_tree::ptree p;
    comma::to_ptree to_ptree( p, root );
    comma::visiting::apply( to_ptree ).to( t );
    boost::property_tree::write_xml( stream, p );
}

template < typename T > inline void write_xml( const T& t, const std::string& filename, const char* root ) { write_xml( t, filename, xpath( root ) ); }
template < typename T > inline void write_xml( const T& t, const std::string& filename ) { write_xml( t, filename, xpath() ); }
template < typename T > inline void write_xml( const T& t, std::ostream& stream, const char* root ) { write_xml( t, stream, xpath( root ) ); }
template < typename T > inline void write_xml( const T& t, std::ostream& stream ) { write_xml( t, stream, xpath() ); }
    
} // namespace comma {

#endif // COMMA_NAME_VALUE_SERIALIZE_H_
