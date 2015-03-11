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

#ifndef COMMA_NAME_VALUE_SERIALIZE_H_
#define COMMA_NAME_VALUE_SERIALIZE_H_

#include <fstream>
#include <iostream>
#include <string>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <comma/base/exception.h>
#include <comma/name_value/ptree.h>
#include <comma/xpath/xpath.h>
#include <comma/visiting/apply.h>

namespace comma {

/// guess format and read boost property tree from file or stream connected to a file (pipe or terminal input is not accepted)
void ptree_from_stream( std::istream& stream, boost::property_tree::ptree& p
    , comma::property_tree::check_repeated_paths check_type = comma::property_tree::no_check, char equal_sign = '=', char delimiter = ','  );

/// guess format and read object from file or stream connected to a file (pipe or terminal input is not accepted)
/// convenience wrappers for comma::property_tree boiler-plate code
template < typename T > T read( const std::string& filename, const xpath& root, bool permissive );
template < typename T > T read( const std::string& filename, const char* root, bool permissive );
template < typename T > T read( const std::string& filename, const xpath& root );
template < typename T > T read( const std::string& filename, const char* root );
template < typename T > T read( const std::string& filename, bool permissive );
template < typename T > T read( const std::string& filename );
template < typename T > void read( T& t, const std::string& filename, const xpath& root, bool permissive );
template < typename T > void read( T& t, const std::string& filename, const char* root, bool permissive );
template < typename T > void read( T& t, const std::string& filename, const xpath& root );
template < typename T > void read( T& t, const std::string& filename, const char* root );
template < typename T > void read( T& t, const std::string& filename, bool permissive );
template < typename T > void read( T& t, const std::string& filename );
template < typename T > T read( std::istream& stream, const xpath& root, bool permissive );
template < typename T > T read( std::istream& stream, const char* root, bool permissive );
template < typename T > T read( std::istream& stream, const xpath& root );
template < typename T > T read( std::istream& stream, const char* root );
template < typename T > T read( std::istream& stream, bool permissive );
template < typename T > T read( std::istream& stream );
template < typename T > void read( T& t, std::istream& stream, const xpath& root, bool permissive );
template < typename T > void read( T& t, std::istream& stream, const char* root, bool permissive );
template < typename T > void read( T& t, std::istream& stream, const xpath& root );
template < typename T > void read( T& t, std::istream& stream, const char* root );
template < typename T > void read( T& t, std::istream& stream, bool permissive );
template < typename T > void read( T& t, std::istream& stream );

/// read object from json file or stream
/// convenience wrappers for comma::property_tree boiler-plate code
template < typename T > T read_json( const std::string& filename, const xpath& root, bool permissive );
template < typename T > T read_json( const std::string& filename, const char* root, bool permissive );
template < typename T > T read_json( const std::string& filename, const xpath& root );
template < typename T > T read_json( const std::string& filename, const char* root );
template < typename T > T read_json( const std::string& filename, bool permissive );
template < typename T > T read_json( const std::string& filename );
template < typename T > void read_json( T& t, const std::string& filename, const xpath& root, bool permissive );
template < typename T > void read_json( T& t, const std::string& filename, const char* root, bool permissive );
template < typename T > void read_json( T& t, const std::string& filename, const xpath& root );
template < typename T > void read_json( T& t, const std::string& filename, const char* root );
template < typename T > void read_json( T& t, const std::string& filename, bool permissive );
template < typename T > void read_json( T& t, const std::string& filename );
template < typename T > T read_json( std::istream& stream, const xpath& root, bool permissive );
template < typename T > T read_json( std::istream& stream, const char* root, bool permissive );
template < typename T > T read_json( std::istream& stream, const xpath& root );
template < typename T > T read_json( std::istream& stream, const char* root );
template < typename T > T read_json( std::istream& stream, bool permissive );
template < typename T > T read_json( std::istream& stream );
template < typename T > void read_json( T& t, std::istream& stream, const xpath& root, bool permissive );
template < typename T > void read_json( T& t, std::istream& stream, const char* root, bool permissive );
template < typename T > void read_json( T& t, std::istream& stream, const xpath& root );
template < typename T > void read_json( T& t, std::istream& stream, const char* root );
template < typename T > void read_json( T& t, std::istream& stream, bool permissive );
template < typename T > void read_json( T& t, std::istream& stream );

/// read object from xml file or stream
/// convenience wrappers for comma::property_tree boiler-plate code
template < typename T > T read_xml( const std::string& filename, const xpath& root, bool permissive );
template < typename T > T read_xml( const std::string& filename, const char* root, bool permissive );
template < typename T > T read_xml( const std::string& filename, const xpath& root );
template < typename T > T read_xml( const std::string& filename, const char* root );
template < typename T > T read_xml( const std::string& filename, bool permissive );
template < typename T > T read_xml( const std::string& filename );
template < typename T > void read_xml( T& t, const std::string& filename, const xpath& root, bool permissive );
template < typename T > void read_xml( T& t, const std::string& filename, const char* root, bool permissive );
template < typename T > void read_xml( T& t, const std::string& filename, const xpath& root );
template < typename T > void read_xml( T& t, const std::string& filename, const char* root );
template < typename T > void read_xml( T& t, const std::string& filename, bool permissive );
template < typename T > void read_xml( T& t, const std::string& filename );
template < typename T > T read_xml( std::istream& stream, const xpath& root, bool permissive );
template < typename T > T read_xml( std::istream& stream, const char* root, bool permissive );
template < typename T > T read_xml( std::istream& stream, const xpath& root );
template < typename T > T read_xml( std::istream& stream, const char* root );
template < typename T > T read_xml( std::istream& stream, bool permissive );
template < typename T > T read_xml( std::istream& stream );
template < typename T > void read_xml( T& t, std::istream& stream, const xpath& root, bool permissive );
template < typename T > void read_xml( T& t, std::istream& stream, const char* root, bool permissive );
template < typename T > void read_xml( T& t, std::istream& stream, const xpath& root );
template < typename T > void read_xml( T& t, std::istream& stream, const char* root );
template < typename T > void read_xml( T& t, std::istream& stream, bool permissive );
template < typename T > void read_xml( T& t, std::istream& stream );

/// read object from path-value file or stream
/// convenience wrappers for comma::property_tree boiler-plate code
template < typename T > T read_path_value( const std::string& filename, const xpath& root, bool permissive );
template < typename T > T read_path_value( const std::string& filename, const char* root, bool permissive );
template < typename T > T read_path_value( const std::string& filename, const xpath& root );
template < typename T > T read_path_value( const std::string& filename, const char* root );
template < typename T > T read_path_value( const std::string& filename, bool permissive );
template < typename T > T read_path_value( const std::string& filename );
template < typename T > void read_path_value( T& t, const std::string& filename, const xpath& root, bool permissive );
template < typename T > void read_path_value( T& t, const std::string& filename, const char* root, bool permissive );
template < typename T > void read_path_value( T& t, const std::string& filename, const xpath& root );
template < typename T > void read_path_value( T& t, const std::string& filename, const char* root );
template < typename T > void read_path_value( T& t, const std::string& filename, bool permissive );
template < typename T > void read_path_value( T& t, const std::string& filename );
template < typename T > T read_path_value( std::istream& stream, const xpath& root, bool permissive );
template < typename T > T read_path_value( std::istream& stream, const char* root, bool permissive );
template < typename T > T read_path_value( std::istream& stream, const xpath& root );
template < typename T > T read_path_value( std::istream& stream, const char* root );
template < typename T > T read_path_value( std::istream& stream, bool permissive );
template < typename T > T read_path_value( std::istream& stream );
template < typename T > void read_path_value( T& t, std::istream& stream, const xpath& root, bool permissive );
template < typename T > void read_path_value( T& t, std::istream& stream, const char* root, bool permissive );
template < typename T > void read_path_value( T& t, std::istream& stream, const xpath& root );
template < typename T > void read_path_value( T& t, std::istream& stream, const char* root );
template < typename T > void read_path_value( T& t, std::istream& stream, bool permissive );
template < typename T > void read_path_value( T& t, std::istream& stream );

/// read object from name-value file or stream
/// convenience wrappers for comma::property_tree boiler-plate code
template < typename T > T read_name_value( const std::string& filename, const xpath& root, bool permissive );
template < typename T > T read_name_value( const std::string& filename, const char* root, bool permissive );
template < typename T > T read_name_value( const std::string& filename, const xpath& root );
template < typename T > T read_name_value( const std::string& filename, const char* root );
template < typename T > T read_name_value( const std::string& filename, bool permissive );
template < typename T > T read_name_value( const std::string& filename );
template < typename T > void read_name_value( T& t, const std::string& filename, const xpath& root, bool permissive );
template < typename T > void read_name_value( T& t, const std::string& filename, const char* root, bool permissive );
template < typename T > void read_name_value( T& t, const std::string& filename, const xpath& root );
template < typename T > void read_name_value( T& t, const std::string& filename, const char* root );
template < typename T > void read_name_value( T& t, const std::string& filename, bool permissive );
template < typename T > void read_name_value( T& t, const std::string& filename );
template < typename T > T read_name_value( std::istream& stream, const xpath& root, bool permissive );
template < typename T > T read_name_value( std::istream& stream, const char* root, bool permissive );
template < typename T > T read_name_value( std::istream& stream, const xpath& root );
template < typename T > T read_name_value( std::istream& stream, const char* root );
template < typename T > T read_name_value( std::istream& stream, bool permissive );
template < typename T > T read_name_value( std::istream& stream );
template < typename T > void read_name_value( T& t, std::istream& stream, const xpath& root, bool permissive );
template < typename T > void read_name_value( T& t, std::istream& stream, const char* root, bool permissive );
template < typename T > void read_name_value( T& t, std::istream& stream, const xpath& root );
template < typename T > void read_name_value( T& t, std::istream& stream, const char* root );
template < typename T > void read_name_value( T& t, std::istream& stream, bool permissive );
template < typename T > void read_name_value( T& t, std::istream& stream );

/// read object from ini file or stream
/// convenience wrappers for comma::property_tree boiler-plate code
template < typename T > T read_ini( const std::string& filename, const xpath& root, bool permissive );
template < typename T > T read_ini( const std::string& filename, const char* root, bool permissive );
template < typename T > T read_ini( const std::string& filename, const xpath& root );
template < typename T > T read_ini( const std::string& filename, const char* root );
template < typename T > T read_ini( const std::string& filename, bool permissive );
template < typename T > T read_ini( const std::string& filename );
template < typename T > void read_ini( T& t, const std::string& filename, const xpath& root, bool permissive );
template < typename T > void read_ini( T& t, const std::string& filename, const char* root, bool permissive );
template < typename T > void read_ini( T& t, const std::string& filename, const xpath& root );
template < typename T > void read_ini( T& t, const std::string& filename, const char* root );
template < typename T > void read_ini( T& t, const std::string& filename, bool permissive );
template < typename T > void read_ini( T& t, const std::string& filename );
template < typename T > T read_ini( std::istream& stream, const xpath& root, bool permissive );
template < typename T > T read_ini( std::istream& stream, const char* root, bool permissive );
template < typename T > T read_ini( std::istream& stream, const xpath& root );
template < typename T > T read_ini( std::istream& stream, const char* root );
template < typename T > T read_ini( std::istream& stream, bool permissive );
template < typename T > T read_ini( std::istream& stream );
template < typename T > void read_ini( T& t, std::istream& stream, const xpath& root, bool permissive );
template < typename T > void read_ini( T& t, std::istream& stream, const char* root, bool permissive );
template < typename T > void read_ini( T& t, std::istream& stream, const xpath& root );
template < typename T > void read_ini( T& t, std::istream& stream, const char* root );
template < typename T > void read_ini( T& t, std::istream& stream, bool permissive );
template < typename T > void read_ini( T& t, std::istream& stream );

/// write json object to file or stream
/// convenience wrappers for comma::property_tree boiler-plate code
template < typename T > void write_json( const T& t, const std::string& filename, const xpath& root );
template < typename T > void write_json( const T& t, const std::string& filename, const char* root );
template < typename T > void write_json( const T& t, const std::string& filename );
template < typename T > void write_json( const T& t, std::ostream& stream, const xpath& root );
template < typename T > void write_json( const T& t, std::ostream& stream, const char* root );
template < typename T > void write_json( const T& t, std::ostream& stream );

/// write xml object to file or stream
/// convenience wrappers for comma::property_tree boiler-plate code
template < typename T > void write_xml( const T& t, const std::string& filename, const xpath& root );
template < typename T > void write_xml( const T& t, const std::string& filename, const char* root );
template < typename T > void write_xml( const T& t, const std::string& filename );
template < typename T > void write_xml( const T& t, std::ostream& stream, const xpath& root );
template < typename T > void write_xml( const T& t, std::ostream& stream, const char* root );
template < typename T > void write_xml( const T& t, std::ostream& stream );

/// write path-value object to file or stream
/// convenience wrappers for comma::property_tree boiler-plate code
/// @todo parametrize on equality sign and delimiter?
template < typename T > void write_path_value( const T& t, const std::string& filename, const xpath& root );
template < typename T > void write_path_value( const T& t, const std::string& filename, const char* root );
template < typename T > void write_path_value( const T& t, const std::string& filename );
template < typename T > void write_path_value( const T& t, std::ostream& stream, const xpath& root );
template < typename T > void write_path_value( const T& t, std::ostream& stream, const char* root );
template < typename T > void write_path_value( const T& t, std::ostream& stream );

/// write name-value object to file or stream
/// convenience wrappers for comma::property_tree boiler-plate code
template < typename T > void write_name_value( const T& t, const std::string& filename, const xpath& root );
template < typename T > void write_name_value( const T& t, const std::string& filename, const char* root );
template < typename T > void write_name_value( const T& t, const std::string& filename );
template < typename T > void write_name_value( const T& t, std::ostream& stream, const xpath& root );
template < typename T > void write_name_value( const T& t, std::ostream& stream, const char* root );
template < typename T > void write_name_value( const T& t, std::ostream& stream );

/// write ini object to file or stream
/// convenience wrappers for comma::property_tree boiler-plate code
template < typename T > void write_ini( const T& t, const std::string& filename, const xpath& root );
template < typename T > void write_ini( const T& t, const std::string& filename, const char* root );
template < typename T > void write_ini( const T& t, const std::string& filename );
template < typename T > void write_ini( const T& t, std::ostream& stream, const xpath& root );
template < typename T > void write_ini( const T& t, std::ostream& stream, const char* root );
template < typename T > void write_ini( const T& t, std::ostream& stream );


template < typename T > inline void read_json( T& t, const std::string& filename, const xpath& root, bool permissive )
{
    std::ifstream ifs( &filename[0] );
    if( !ifs.is_open() ) { COMMA_THROW( comma::exception, "failed to open \"" << filename << "\"" ); }
    read_json< T >( t, ifs, root, permissive );
    ifs.close();
}

template < typename T > inline void read_json( T& t, std::istream& stream, const xpath& root, bool permissive )
{
    boost::property_tree::ptree p;
    boost::property_tree::read_json( stream, p );
    comma::from_ptree from_ptree( p, root, permissive );
    comma::visiting::apply( from_ptree ).to( t );
}

template < typename T > inline T read_json( const std::string& filename, const xpath& root, bool permissive ) { T t; read_json< T >( t, filename, root, permissive ); return t; }
template < typename T > inline T read_json( const std::string& filename, const char* root, bool permissive ) { return root ? read_json< T >( filename, xpath( root ), permissive ) : read_json< T >( filename, permissive ); }
template < typename T > inline T read_json( const std::string& filename, const xpath& root ) { return read_json< T >( filename, root, true ); }
template < typename T > inline T read_json( const std::string& filename, const char* root ) { return root ? read_json< T >( filename, xpath( root ), true ) : read_json< T >( filename, true ); }
template < typename T > inline T read_json( const std::string& filename, bool permissive ) { return read_json< T >( filename, xpath(), permissive ); }
template < typename T > inline T read_json( const std::string& filename ) { return read_json< T >( filename, xpath(), true ); }
template < typename T > inline T read_json( std::istream& stream, const xpath& root, bool permissive ) { T t; read_json< T >( t, stream, root, permissive ); return t; }
template < typename T > inline T read_json( std::istream& stream, const char* root, bool permissive ) { return root ? read_json< T >( stream, xpath( root ), permissive ) : read_json< T >( stream, permissive ); }
template < typename T > inline T read_json( std::istream& stream, const xpath& root ) { return read_json< T >( stream, root, true ); }
template < typename T > inline T read_json( std::istream& stream, const char* root ) { return root ? read_json< T >( stream, xpath( root ), true ) : read_json< T >( stream, true ); }
template < typename T > inline T read_json( std::istream& stream, bool permissive ) { return read_json< T >( stream, xpath(), permissive ); }
template < typename T > inline T read_json( std::istream& stream ) { return read_json< T >( stream, xpath(), true ); }
template < typename T > inline void read_json( T& t, const std::string& filename, const char* root, bool permissive ) { if( root ) { read_json< T >( t, filename, xpath( root ), permissive ); } else { read_json< T >( t, filename, permissive ); } }
template < typename T > inline void read_json( T& t, const std::string& filename, const xpath& root ) { read_json< T >( t, filename, root, true ); }
template < typename T > inline void read_json( T& t, const std::string& filename, const char* root ) { if( root ) { read_json< T >( t, filename, xpath( root ), true ); } else { read_json< T >( t, filename, true ); } }
template < typename T > inline void read_json( T& t, const std::string& filename, bool permissive ) { read_json< T >( t, filename, xpath(), permissive ); }
template < typename T > inline void read_json( T& t, const std::string& filename ) { return read_json< T >( t, filename, xpath(), true ); }
template < typename T > inline void read_json( T& t, std::istream& stream, const char* root, bool permissive ) { if( root ) { read_json< T >( t, stream, xpath( root ), permissive ); } else { read_json< T >( t, stream, permissive ); } }
template < typename T > inline void read_json( T& t, std::istream& stream, const xpath& root ) { read_json< T >( t, stream, root, true ); }
template < typename T > inline void read_json( T& t, std::istream& stream, const char* root ) { if( root ) { read_json< T >( t, stream, xpath( root ), true ); } else { read_json< T >( t, stream, true ); } }
template < typename T > inline void read_json( T& t, std::istream& stream, bool permissive ) { read_json< T >( t, stream, xpath(), permissive ); }
template < typename T > inline void read_json( T& t, std::istream& stream ) { read_json< T >( t, stream, xpath(), true ); }

template < typename T > inline void read_xml( T& t, const std::string& filename, const xpath& root, bool permissive )
{
    std::ifstream ifs( &filename[0] );
    if( !ifs.is_open() ) { COMMA_THROW( comma::exception, "failed to open \"" << filename << "\"" ); }
    read_xml< T >( t, ifs, root, permissive );
    ifs.close();
}

template < typename T > inline void read_xml( T& t, std::istream& stream, const xpath& root, bool permissive )
{
    boost::property_tree::ptree p;
    boost::property_tree::read_xml( stream, p );
    comma::from_ptree from_ptree( p, root, permissive );
    comma::visiting::apply( from_ptree ).to( t );
}

template < typename T > inline T read_xml( const std::string& filename, const xpath& root, bool permissive ) { T t; read_xml< T >( t, filename, root, permissive ); return t; }
template < typename T > inline T read_xml( const std::string& filename, const char* root, bool permissive ) { return root ? read_xml< T >( filename, xpath( root ), permissive ) : read_xml< T >( filename, permissive ); }
template < typename T > inline T read_xml( const std::string& filename, const xpath& root ) { return read_xml< T >( filename, root, true ); }
template < typename T > inline T read_xml( const std::string& filename, const char* root ) { return root ? read_xml< T >( filename, xpath( root ), true ) : read_xml< T >( filename, true ); }
template < typename T > inline T read_xml( const std::string& filename, bool permissive ) { return read_xml< T >( filename, xpath(), permissive ); }
template < typename T > inline T read_xml( const std::string& filename ) { return read_xml< T >( filename, xpath(), true ); }
template < typename T > inline T read_xml( std::istream& stream, const xpath& root, bool permissive ) { T t; read_xml< T >( t, stream, root, permissive ); return t; }
template < typename T > inline T read_xml( std::istream& stream, const char* root, bool permissive ) { return root ? read_xml< T >( stream, xpath( root ), permissive ) : read_xml< T >( stream, permissive ); }
template < typename T > inline T read_xml( std::istream& stream, const xpath& root ) { return read_xml< T >( stream, root, true ); }
template < typename T > inline T read_xml( std::istream& stream, const char* root ) { return root ? read_xml< T >( stream, xpath( root ), true ) : read_xml< T >( stream, true ); }
template < typename T > inline T read_xml( std::istream& stream, bool permissive ) { return read_xml< T >( stream, xpath(), permissive ); }
template < typename T > inline T read_xml( std::istream& stream ) { return read_xml< T >( stream, xpath(), true ); }
template < typename T > inline void read_xml( T& t, const std::string& filename, const char* root, bool permissive ) { if( root ) { read_xml< T >( t, filename, xpath( root ), permissive ); } else { read_xml< T >( t, filename, permissive ); } }
template < typename T > inline void read_xml( T& t, const std::string& filename, const xpath& root ) { read_xml< T >( t, filename, root, true ); }
template < typename T > inline void read_xml( T& t, const std::string& filename, const char* root ) { if( root ) { read_xml< T >( t, filename, xpath( root ), true ); } else { read_xml< T >( t, filename, true ); } }
template < typename T > inline void read_xml( T& t, const std::string& filename, bool permissive ) { read_xml< T >( t, filename, xpath(), permissive ); }
template < typename T > inline void read_xml( T& t, const std::string& filename ) { return read_xml< T >( t, filename, xpath(), true ); }
template < typename T > inline void read_xml( T& t, std::istream& stream, const char* root, bool permissive ) { if( root ) { read_xml< T >( t, stream, xpath( root ), permissive ); } else { read_xml< T >( t, stream, permissive ); } }
template < typename T > inline void read_xml( T& t, std::istream& stream, const xpath& root ) { read_xml< T >( t, stream, root, true ); }
template < typename T > inline void read_xml( T& t, std::istream& stream, const char* root ) { if( root ) { read_xml< T >( t, stream, xpath( root ), true ); } else { read_xml< T >( t, stream, true ); } }
template < typename T > inline void read_xml( T& t, std::istream& stream, bool permissive ) { read_xml< T >( t, stream, xpath(), permissive ); }
template < typename T > inline void read_xml( T& t, std::istream& stream ) { read_xml< T >( t, stream, xpath(), true ); }

template < typename T > inline void read_path_value( T& t, std::istream& stream, const xpath& root, bool permissive )
{
    boost::property_tree::ptree p;
    comma::property_tree::from_path_value( stream, p, comma::property_tree::no_check, '=', '\n' );
    comma::from_ptree from_ptree( p, root, permissive );
    comma::visiting::apply( from_ptree ).to( t );
}

template < typename T > inline void read_path_value( T& t, const std::string& filename, const xpath& root, bool permissive )
{
    std::ifstream ifs( &filename[0] );
    if( !ifs.is_open() ) { COMMA_THROW( comma::exception, "failed to open \"" << filename << "\"" ); }
    read_path_value< T >( t, ifs, root, permissive );
    ifs.close();
}

template < typename T > inline T read_path_value( const std::string& filename, const xpath& root, bool permissive ) { T t; read_path_value< T >( t, filename, root, permissive ); return t; }
template < typename T > inline T read_path_value( const std::string& filename, const char* root, bool permissive ) { return root ? read_path_value< T >( filename, xpath( root ), permissive ) : read_path_value< T >( filename, permissive ); }
template < typename T > inline T read_path_value( const std::string& filename, const xpath& root ) { return read_path_value< T >( filename, root, true ); }
template < typename T > inline T read_path_value( const std::string& filename, const char* root ) { return root ? read_path_value< T >( filename, xpath( root ), true ) : read_path_value< T >( filename, true ); }
template < typename T > inline T read_path_value( const std::string& filename, bool permissive ) { return read_path_value< T >( filename, xpath(), permissive ); }
template < typename T > inline T read_path_value( const std::string& filename ) { return read_path_value< T >( filename, xpath(), true ); }
template < typename T > inline T read_path_value( std::istream& stream, const xpath& root, bool permissive ) { T t; read_path_value< T >( t, stream, root, permissive ); return t; }
template < typename T > inline T read_path_value( std::istream& stream, const char* root, bool permissive ) { return root ? read_path_value< T >( stream, xpath( root ), permissive ) : read_path_value< T >( stream, permissive ); }
template < typename T > inline T read_path_value( std::istream& stream, const xpath& root ) { return read_path_value< T >( stream, root, true ); }
template < typename T > inline T read_path_value( std::istream& stream, const char* root ) { return root ? read_path_value< T >( stream, xpath( root ), true ) : read_path_value< T >( stream, true ); }
template < typename T > inline T read_path_value( std::istream& stream, bool permissive ) { return read_path_value< T >( stream, xpath(), permissive ); }
template < typename T > inline T read_path_value( std::istream& stream ) { return read_path_value< T >( stream, xpath(), true ); }
template < typename T > inline void read_path_value( T& t, const std::string& filename, const char* root, bool permissive ) { if( root ) { read_path_value< T >( t, filename, xpath( root ), permissive ); } else { read_path_value< T >( t, filename, permissive ); } }
template < typename T > inline void read_path_value( T& t, const std::string& filename, const xpath& root ) { read_path_value< T >( t, filename, root, true ); }
template < typename T > inline void read_path_value( T& t, const std::string& filename, const char* root ) { if( root ) { read_path_value< T >( t, filename, xpath( root ), true ); } else { read_path_value< T >( t, filename, true ); } }
template < typename T > inline void read_path_value( T& t, const std::string& filename, bool permissive ) { read_path_value< T >( t, filename, xpath(), permissive ); }
template < typename T > inline void read_path_value( T& t, const std::string& filename ) { return read_path_value< T >( t, filename, xpath(), true ); }
template < typename T > inline void read_path_value( T& t, std::istream& stream, const char* root, bool permissive ) { if( root ) { read_path_value< T >( t, stream, xpath( root ), permissive ); } else { read_path_value< T >( t, stream, permissive ); } }
template < typename T > inline void read_path_value( T& t, std::istream& stream, const xpath& root ) { read_path_value< T >( t, stream, root, true ); }
template < typename T > inline void read_path_value( T& t, std::istream& stream, const char* root ) { if( root ) { read_path_value< T >( t, stream, xpath( root ), true ); } else { read_path_value< T >( t, stream, true ); } }
template < typename T > inline void read_path_value( T& t, std::istream& stream, bool permissive ) { read_path_value< T >( t, stream, xpath(), permissive ); }
template < typename T > inline void read_path_value( T& t, std::istream& stream ) { read_path_value< T >( t, stream, xpath(), true ); }

template < typename T > inline void read_name_value( T& t, std::istream& stream, const xpath& root, bool permissive )
{
    boost::property_tree::ptree p;
    comma::property_tree::from_name_value( stream, p );
    //std::cerr << property_tree::to_name_value_string( p ) << std::endl;
    comma::from_ptree from_ptree( p, root, permissive );
    comma::visiting::apply( from_ptree ).to( t );
}

template < typename T > inline void read_name_value( T& t, const std::string& filename, const xpath& root, bool permissive )
{
    std::ifstream ifs( &filename[0] );
    if( !ifs.is_open() ) { COMMA_THROW( comma::exception, "failed to open \"" << filename << "\"" ); }
    read_name_value< T >( t, ifs, root, permissive );
    ifs.close();
}

template < typename T > inline T read_name_value( const std::string& filename, const xpath& root, bool permissive ) { T t; read_name_value< T >( t, filename, root, permissive ); return t; }
template < typename T > inline T read_name_value( const std::string& filename, const char* root, bool permissive ) { return root ? read_name_value< T >( filename, xpath( root ), permissive ) : read_name_value< T >( filename, permissive ); }
template < typename T > inline T read_name_value( const std::string& filename, const xpath& root ) { return read_name_value< T >( filename, root, true ); }
template < typename T > inline T read_name_value( const std::string& filename, const char* root ) { return root ? read_name_value< T >( filename, xpath( root ), true ) : read_name_value< T >( filename, true ); }
template < typename T > inline T read_name_value( const std::string& filename, bool permissive ) { return read_name_value< T >( filename, xpath(), permissive ); }
template < typename T > inline T read_name_value( const std::string& filename ) { return read_name_value< T >( filename, xpath(), true ); }
template < typename T > inline T read_name_value( std::istream& stream, const xpath& root, bool permissive ) { T t; read_name_value< T >( t, stream, root, permissive ); return t; }
template < typename T > inline T read_name_value( std::istream& stream, const char* root, bool permissive ) { return root ? read_name_value< T >( stream, xpath( root ), permissive ) : read_name_value< T >( stream, permissive ); }
template < typename T > inline T read_name_value( std::istream& stream, const xpath& root ) { return read_name_value< T >( stream, root, true ); }
template < typename T > inline T read_name_value( std::istream& stream, const char* root ) { return root ? read_name_value< T >( stream, xpath( root ), true ) : read_name_value< T >( stream, true ); }
template < typename T > inline T read_name_value( std::istream& stream, bool permissive ) { return read_name_value< T >( stream, xpath(), permissive ); }
template < typename T > inline T read_name_value( std::istream& stream ) { return read_name_value< T >( stream, xpath(), true ); }
template < typename T > inline void read_name_value( T& t, const std::string& filename, const char* root, bool permissive ) { if( root ) { read_name_value< T >( t, filename, xpath( root ), permissive ); } else { read_name_value< T >( t, filename, permissive ); } }
template < typename T > inline void read_name_value( T& t, const std::string& filename, const xpath& root ) { read_name_value< T >( t, filename, root, true ); }
template < typename T > inline void read_name_value( T& t, const std::string& filename, const char* root ) { if( root ) { read_name_value< T >( t, filename, xpath( root ), true ); } else { read_name_value< T >( t, filename, true ); } }
template < typename T > inline void read_name_value( T& t, const std::string& filename, bool permissive ) { read_name_value< T >( t, filename, xpath(), permissive ); }
template < typename T > inline void read_name_value( T& t, const std::string& filename ) { return read_name_value< T >( t, filename, xpath(), true ); }
template < typename T > inline void read_name_value( T& t, std::istream& stream, const char* root, bool permissive ) { if( root ) { read_name_value< T >( t, stream, xpath( root ), permissive ); } else { read_name_value< T >( t, stream, permissive ); } }
template < typename T > inline void read_name_value( T& t, std::istream& stream, const xpath& root ) { read_name_value< T >( t, stream, root, true ); }
template < typename T > inline void read_name_value( T& t, std::istream& stream, const char* root ) { if( root ) { read_name_value< T >( t, stream, xpath( root ), true ); } else { read_name_value< T >( t, stream, true ); } }
template < typename T > inline void read_name_value( T& t, std::istream& stream, bool permissive ) { read_name_value< T >( t, stream, xpath(), permissive ); }
template < typename T > inline void read_name_value( T& t, std::istream& stream ) { read_name_value< T >( t, stream, xpath(), true ); }

template < typename T > inline void read_ini( T& t, std::istream& stream, const xpath& root, bool permissive )
{
    boost::property_tree::ptree p;
    boost::property_tree::read_ini( stream, p );
    //std::cerr << property_tree::to_name_value_string( p ) << std::endl;
    comma::from_ptree from_ptree( p, root, permissive );
    comma::visiting::apply( from_ptree ).to( t );
}

template < typename T > inline void read_ini( T& t, const std::string& filename, const xpath& root, bool permissive )
{
    std::ifstream ifs( &filename[0] );
    if( !ifs.is_open() ) { COMMA_THROW( comma::exception, "failed to open \"" << filename << "\"" ); }
    read_ini< T >( t, ifs, root, permissive );
    ifs.close();
}

template < typename T > inline T read_ini( const std::string& filename, const xpath& root, bool permissive ) { T t; read_ini< T >( t, filename, root, permissive ); return t; }
template < typename T > inline T read_ini( const std::string& filename, const char* root, bool permissive ) { return root ? read_ini< T >( filename, xpath( root ), permissive ) : read_ini< T >( filename, permissive ); }
template < typename T > inline T read_ini( const std::string& filename, const xpath& root ) { return read_ini< T >( filename, root, true ); }
template < typename T > inline T read_ini( const std::string& filename, const char* root ) { return root ? read_ini< T >( filename, xpath( root ), true ) : read_ini< T >( filename, true ); }
template < typename T > inline T read_ini( const std::string& filename, bool permissive ) { return read_ini< T >( filename, xpath(), permissive ); }
template < typename T > inline T read_ini( const std::string& filename ) { return read_ini< T >( filename, xpath(), true ); }
template < typename T > inline T read_ini( std::istream& stream, const xpath& root, bool permissive ) { T t; read_ini< T >( t, stream, root, permissive ); return t; }
template < typename T > inline T read_ini( std::istream& stream, const char* root, bool permissive ) { return root ? read_ini< T >( stream, xpath( root ), permissive ) : read_ini< T >( stream, permissive ); }
template < typename T > inline T read_ini( std::istream& stream, const xpath& root ) { return read_ini< T >( stream, root, true ); }
template < typename T > inline T read_ini( std::istream& stream, const char* root ) { return root ? read_ini< T >( stream, xpath( root ), true ) : read_ini< T >( stream, true ); }
template < typename T > inline T read_ini( std::istream& stream, bool permissive ) { return read_ini< T >( stream, xpath(), permissive ); }
template < typename T > inline T read_ini( std::istream& stream ) { return read_ini< T >( stream, xpath(), true ); }
template < typename T > inline void read_ini( T& t, const std::string& filename, const char* root, bool permissive ) { if( root ) { read_ini< T >( t, filename, xpath( root ), permissive ); } else { read_ini< T >( t, filename, permissive ); } }
template < typename T > inline void read_ini( T& t, const std::string& filename, const xpath& root ) { read_ini< T >( t, filename, root, true ); }
template < typename T > inline void read_ini( T& t, const std::string& filename, const char* root ) { if( root ) { read_ini< T >( t, filename, xpath( root ), true ); } else { read_ini< T >( t, filename, true ); } }
template < typename T > inline void read_ini( T& t, const std::string& filename, bool permissive ) { read_ini< T >( t, filename, xpath(), permissive ); }
template < typename T > inline void read_ini( T& t, const std::string& filename ) { return read_ini< T >( t, filename, xpath(), true ); }
template < typename T > inline void read_ini( T& t, std::istream& stream, const char* root, bool permissive ) { if( root ) { read_ini< T >( t, stream, xpath( root ), permissive ); } else { read_ini< T >( t, stream, permissive ); } }
template < typename T > inline void read_ini( T& t, std::istream& stream, const xpath& root ) { read_ini< T >( t, stream, root, true ); }
template < typename T > inline void read_ini( T& t, std::istream& stream, const char* root ) { if( root ) { read_ini< T >( t, stream, xpath( root ), true ); } else { read_ini< T >( t, stream, true ); } }
template < typename T > inline void read_ini( T& t, std::istream& stream, bool permissive ) { read_ini< T >( t, stream, xpath(), permissive ); }
template < typename T > inline void read_ini( T& t, std::istream& stream ) { read_ini< T >( t, stream, xpath(), true ); }

template < typename T > inline void write_json( const T& t, const std::string& filename, const xpath& root )
{
    std::ofstream ofs( &filename[0] );
    if( !ofs.is_open() ) { COMMA_THROW( comma::exception, "failed to open \"" << filename << "\"" ); }
    write_json< T >( t, ofs, root );
    ofs.close();
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
    write_xml< T >( t, ofs, root );
    ofs.close();
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

template < typename T > inline void write_path_value( const T& t, std::ostream& stream, const xpath& root )
{
    boost::property_tree::ptree p;
    comma::to_ptree to_ptree( p, root );
    comma::visiting::apply( to_ptree ).to( t );
    comma::property_tree::to_path_value( stream, p, comma::property_tree::disabled, '=', '\n' );
}

template < typename T > inline void write_path_value( const T& t, const std::string& filename, const xpath& root )
{
    std::ofstream ofs( &filename[0] );
    if( !ofs.is_open() ) { COMMA_THROW( comma::exception, "failed to open \"" << filename << "\"" ); }
    write_path_value< T >( t, ofs, root );
    ofs.close();
}

template < typename T > inline void write_path_value( const T& t, const std::string& filename, const char* root ) { write_path_value( t, filename, xpath( root ) ); }
template < typename T > inline void write_path_value( const T& t, const std::string& filename ) { write_path_value( t, filename, xpath() ); }
template < typename T > inline void write_path_value( const T& t, std::ostream& stream, const char* root ) { write_path_value( t, stream, xpath( root ) ); }
template < typename T > inline void write_path_value( const T& t, std::ostream& stream ) { write_path_value( t, stream, xpath() ); }

template < typename T > inline void write_name_value( const T& t, std::ostream& stream, const xpath& root )
{
    boost::property_tree::ptree p;
    comma::to_ptree to_ptree( p, root );
    comma::visiting::apply( to_ptree ).to( t );
    comma::property_tree::to_name_value( stream, p );
}

template < typename T > inline void write_name_value( const T& t, const std::string& filename, const xpath& root )
{
    std::ofstream ofs( &filename[0] );
    if( !ofs.is_open() ) { COMMA_THROW( comma::exception, "failed to open \"" << filename << "\"" ); }
    write_name_value< T >( t, ofs, root );
    ofs.close();
}

template < typename T > inline void write_name_value( const T& t, const std::string& filename, const char* root ) { write_name_value( t, filename, xpath( root ) ); }
template < typename T > inline void write_name_value( const T& t, const std::string& filename ) { write_name_value( t, filename, xpath() ); }
template < typename T > inline void write_name_value( const T& t, std::ostream& stream, const char* root ) { write_name_value( t, stream, xpath( root ) ); }
template < typename T > inline void write_name_value( const T& t, std::ostream& stream ) { write_name_value( t, stream, xpath() ); }

template < typename T > inline void write_ini( const T& t, std::ostream& stream, const xpath& root )
{
    boost::property_tree::ptree p;
    comma::to_ptree to_ptree( p, root );
    comma::visiting::apply( to_ptree ).to( t );
    boost::property_tree::write_ini( stream, p );
}

template < typename T > inline void write_ini( const T& t, const std::string& filename, const xpath& root )
{
    std::ofstream ofs( &filename[0] );
    if( !ofs.is_open() ) { COMMA_THROW( comma::exception, "failed to open \"" << filename << "\"" ); }
    write_ini< T >( t, ofs, root );
    ofs.close();
}

template < typename T > inline void write_ini( const T& t, const std::string& filename, const char* root ) { write_ini( t, filename, xpath( root ) ); }
template < typename T > inline void write_ini( const T& t, const std::string& filename ) { write_ini( t, filename, xpath() ); }
template < typename T > inline void write_ini( const T& t, std::ostream& stream, const char* root ) { write_ini( t, stream, xpath( root ) ); }
template < typename T > inline void write_ini( const T& t, std::ostream& stream ) { write_ini( t, stream, xpath() ); }

inline void ptree_from_stream( std::istream& stream, boost::property_tree::ptree& p, comma::property_tree::check_repeated_paths check_type, char equal_sign, char delimiter)
{
    if( !stream.seekg( 1, std::ios::beg ).good() ) { COMMA_THROW( comma::exception, "input stream is not seekable, e.g. if a pipe or terminal input are used" ) }
    try
    {
        stream.clear();
        stream.seekg( 0, std::ios::beg );
        if( !stream.good() ) { COMMA_THROW( comma::exception, "failed to reset stream" ) }
        boost::property_tree::read_json( stream, p );
        return;
    }
    catch( const boost::property_tree::ptree_error&  ex ) {}
    catch(...) { throw; }
    try
    {
        stream.clear();
        stream.seekg( 0, std::ios::beg );
        if( !stream.good() ) { COMMA_THROW( comma::exception, "failed to reset stream" ) }
        boost::property_tree::read_xml( stream, p );
        return;
    }
    catch( const boost::property_tree::ptree_error&  ex ) {}
    catch(...) { throw; }
    try
    {
        stream.clear();
        stream.seekg( 0, std::ios::beg );
        if( !stream.good() ) { COMMA_THROW( comma::exception, "failed to reset stream" ) }
        comma::property_tree::from_path_value( stream, p, check_type, equal_sign, delimiter );
        return;
    }
    catch( const boost::property_tree::ptree_error&  ex ) {}
    catch( const comma::exception&  ex ) {}
    catch(...) { throw; }
    COMMA_THROW( comma::exception, "failed to guess format" );
}
template < typename T > inline void read( T& t, std::istream& stream, const xpath& root, bool permissive )
{
    boost::property_tree::ptree p;
    ptree_from_stream( stream, p );
    comma::from_ptree from_ptree( p, root, permissive );
    comma::visiting::apply( from_ptree ).to( t );
}

template < typename T > inline void read( T& t, const std::string& filename, const xpath& root, bool permissive )
{
    std::ifstream stream( &filename[0] );
    if( !stream.is_open() ) { COMMA_THROW( comma::exception, "failed to open \"" << filename << "\"" ); }
    read< T >( t, stream, root, permissive );
    stream.close();
}

template < typename T > inline T read( const std::string& filename, const xpath& root, bool permissive ) { T t; read< T >( t, filename, root, permissive ); return t; }
template < typename T > inline T read( const std::string& filename, const char* root, bool permissive ) { return root ? read_json< T >( filename, xpath( root ), permissive ) : read< T >( filename, permissive ); }
template < typename T > inline T read( const std::string& filename, const xpath& root ) { return read< T >( filename, root, true ); }
template < typename T > inline T read( const std::string& filename, const char* root ) { return root ? read< T >( filename, xpath( root ), true ) : read< T >( filename, true ); }
template < typename T > inline T read( const std::string& filename, bool permissive ) { return read< T >( filename, xpath(), permissive ); }
template < typename T > inline T read( const std::string& filename ) { return read< T >( filename, xpath(), true ); }
template < typename T > inline T read( std::istream& stream, const xpath& root, bool permissive ) { T t; read< T >( t, stream, root, permissive ); return t; }
template < typename T > inline T read( std::istream& stream, const char* root, bool permissive ) { return root ? read< T >( stream, xpath( root ), permissive ) : read< T >( stream, permissive ); }
template < typename T > inline T read( std::istream& stream, const xpath& root ) { return read< T >( stream, root, true ); }
template < typename T > inline T read( std::istream& stream, const char* root ) { return root ? read< T >( stream, xpath( root ), true ) : read< T >( stream, true ); }
template < typename T > inline T read( std::istream& stream, bool permissive ) { return read< T >( stream, xpath(), permissive ); }
template < typename T > inline T read( std::istream& stream ) { return read< T >( stream, xpath(), true ); }
template < typename T > inline void read( T& t, const std::string& filename, const char* root, bool permissive ) { if( root ) { read< T >( t, filename, xpath( root ), permissive ); } else { read< T >( t, filename, permissive ); } }
template < typename T > inline void read( T& t, const std::string& filename, const xpath& root ) { read< T >( t, filename, root, true ); }
template < typename T > inline void read( T& t, const std::string& filename, const char* root ) { if( root ) { read< T >( t, filename, xpath( root ), true ); } else { read< T >( t, filename, true ); } }
template < typename T > inline void read( T& t, const std::string& filename, bool permissive ) { read< T >( t, filename, xpath(), permissive ); }
template < typename T > inline void read( T& t, const std::string& filename ) { return read< T >( t, filename, xpath(), true ); }
template < typename T > inline void read( T& t, std::istream& stream, const char* root, bool permissive ) { if( root ) { read< T >( t, stream, xpath( root ), permissive ); } else { read< T >( t, stream, permissive ); } }
template < typename T > inline void read( T& t, std::istream& stream, const xpath& root ) { read< T >( t, stream, root, true ); }
template < typename T > inline void read( T& t, std::istream& stream, const char* root ) { if( root ) { read< T >( t, stream, xpath( root ), true ); } else { read< T >( t, stream, true ); } }
template < typename T > inline void read( T& t, std::istream& stream, bool permissive ) { read< T >( t, stream, xpath(), permissive ); }
template < typename T > inline void read( T& t, std::istream& stream ) { read< T >( t, stream, xpath(), true ); }

} // namespace comma {

#endif // COMMA_NAME_VALUE_SERIALIZE_H_
