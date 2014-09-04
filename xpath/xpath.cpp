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

#include <sstream>
#include <boost/lexical_cast.hpp>
#include <comma/base/exception.h>
#include <comma/string/split.h>
#include "./xpath.h"

#include <iostream>

namespace comma {

xpath::element::element() {}

xpath::element::element( const xpath::element& rhs ) { operator=( rhs ); }

xpath::element::element( const std::string& n )
{
    std::vector< std::string > v = split( n, '[' );
    switch( v.size() )
    {
        case 1:
            name = v[0];
            break;
        case 2:
            if( v[0] == "" ) { COMMA_THROW_STREAM( comma::exception, "got non-empty index " << v[1] << " in empty element" ); }
            if( v[1].length() < 2 || v[1][ v[1].size() - 1 ] != ']' ) { COMMA_THROW_STREAM( comma::exception, "invalid index in element \"" << n << "\"" ); }
            name = v[0];
            index = boost::lexical_cast< std::size_t >( v[1].substr( 0, v[1].size() - 1 ) );
            break;
        default:
            COMMA_THROW_STREAM( comma::exception, "invalid element \"" << n << "\"" );
    }    
}

xpath::element::element( const std::string& n, std::size_t idx )
    : name( n )
    , index( idx )
{
    if( index && n == "" ) { COMMA_THROW_STREAM( comma::exception, "got non-empty index in empty element" ); }
}

xpath::element::element( const std::string& n, boost::optional< std::size_t > idx )
    : name( n )
    , index( idx )
{
    if( index && name == "" ) { COMMA_THROW_STREAM( comma::exception, "got non-empty index in empty element" ); }
}

bool xpath::element::operator==( const xpath::element& rhs ) const
{
    return name == rhs.name && index == rhs.index;
}
            
bool xpath::element::operator!=( const xpath::element& rhs ) const { return !operator==( rhs ); }

bool xpath::element::operator<( const xpath::element& rhs ) const
{
    return name == rhs.name && index && !rhs.index;
}

bool xpath::element::operator<=( const xpath::element& rhs ) const
{
    return operator<( rhs ) || operator==( rhs );
}

std::string xpath::element::to_string() const
{
    if( !index ) { return name; }
    return name + "[" + boost::lexical_cast< std::string >( *index ) + "]";
}

xpath::xpath() {}

xpath::xpath( const xpath& rhs ) { operator=( rhs ); }

static void init( xpath& x, const std::string& str, char delimiter )
{
    if( str == "" ) { return; }
    std::vector< std::string > v = comma::split( str, delimiter );
    for( std::size_t i = 0; i < v.size(); ++i )
    {
        if( i > 0 && v[i] == "" ) { continue; }
        x.elements.push_back( xpath::element( v[i] ) );
    }	
}

xpath::xpath( const char* str, char delimiter ) { init( *this, str, delimiter ); }

xpath::xpath( const std::string& str, char delimiter ) { init( *this, str, delimiter ); }

xpath::xpath( const xpath::element& rhs, char delimiter ) { init( *this, "", delimiter ); operator/=( rhs ); }

const xpath& xpath::operator/=( const xpath& rhs )
{
    if( !rhs.elements.empty() )
    {
        for( std::size_t i = 0; i < rhs.elements.size(); ++i )
        {
            if( rhs.elements[i] != element( "" ) ) { elements.push_back( rhs.elements[i] ); }
        }
    }
    return *this;
}

const xpath& xpath::operator/=( const xpath::element& rhs )
{
    if( rhs != xpath::element( "" ) ) { elements.push_back( rhs ); }
    return *this;
}

const xpath& xpath::operator/=( const std::string& rhs ) { return operator/=( xpath( rhs ) ); }

xpath xpath::tail() const
{
    xpath x;
    if( elements.size() > 1 )
    {
        x.elements.resize( elements.size() - 1 );
        for( std::size_t i = 1; i < elements.size(); ++i ) { x.elements[ i - 1 ] = elements[i]; }
    }
    return x;
}

xpath xpath::operator/( const xpath& rhs ) const { xpath x( *this ); x /= rhs; return x; }

xpath xpath::operator/( const xpath::element& rhs ) const { xpath x( *this ); x /= rhs; return x; }

xpath xpath::operator/( const std::string& rhs ) const { xpath x( *this ); x /= rhs; return x; }

xpath xpath::head() const
{
    if( *this == xpath( "/" ) ) { return *this; }
    xpath x;
    if( elements.size() > 1 )
    {
        x.elements.resize( elements.size() - 1 );
        for( std::size_t i = 0; i < x.elements.size(); ++i ) { x.elements[i] = elements[i]; }
    }
    return x;
}

std::string xpath::to_string( char delimiter ) const
{
    if( elements.empty() ) { return ""; }
    std::ostringstream oss;
    oss << elements[0].to_string();
    for( std::size_t i = 1; i < elements.size(); ++i ) { oss << delimiter << elements[i].to_string(); }
    return oss.str();
}

bool xpath::operator==( const xpath& rhs ) const
{
    if( elements.size() != rhs.elements.size() ) { return false; }
    for( std::size_t i = 0; i < elements.size(); ++i )
    {
        if( elements[i] != rhs.elements[i] ) { return false; }
    }
    return true;
} 

bool xpath::operator!=( const xpath& rhs ) const { return !operator==( rhs ); }

bool xpath::operator<( const xpath& rhs ) const
{
    if( elements.size() < rhs.elements.size() ) { return false; }
    if( elements.empty() ) { return false; }
    if( rhs.elements.empty() ) { return true; }
    std::size_t i = 0;
    for( ; i < rhs.elements.size() - 1; ++i )
    {
        if( elements[i] != rhs.elements[i] ) { return false; }
    }
    return   elements.size() == rhs.elements.size()
           ? elements.back() < rhs.elements.back()
           : elements[i] <= rhs.elements.back();
}

bool xpath::operator<=( const xpath& rhs ) const { return operator<( rhs ) || operator==( rhs ); }

static std::ostream &
operator <<(std::ostream & os, comma::xpath::element const & value)
{
    return os << value.name;
}

std::ostream &
xpath::output(std::ostream & os) const
{
    std::ostream_iterator<comma::xpath::element const> out_itr(os, "/");
    std::copy(elements.begin(), elements.end(), out_itr);
}


} // namespace comma {
