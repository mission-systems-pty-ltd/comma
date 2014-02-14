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

#include <set>
#include <boost/optional.hpp>
#include <boost/regex.hpp>
#include <comma/application/command_line_options.h>
#include <comma/base/exception.h>
#include <comma/string/split.h>

namespace comma {

command_line_options::command_line_options( int argc, char ** argv )
{
    argv_.resize( argc );
    for( int i = 0; i < argc; ++i ) { argv_[i] = argv[i]; }
    fill_map_( argv_ );
}

command_line_options::command_line_options( const std::vector< std::string >& argv )
    : argv_( argv )
{
    fill_map_( argv_ );
}

std::string command_line_options::string() const
{
    std::ostringstream out;
    for ( size_t arg = 0; arg < argv_.size(); ++arg )
    {
        if ( arg > 0 ) { out << ' '; }

        // check if string needs to be quoted
        if ( argv_[arg].find_first_of( "; \t\n&<>|$#*?()[]{}\'\"" ) != std::string::npos )
        {
            std::string a = argv_[arg];

            // check for double quotes inside the string
            size_t pos = 0;
            while ( true )
            {
                pos = a.find( '"', pos );
                if ( pos == std::string::npos ) { break; }
                a.replace( pos, 1, "\\\"" );
                pos += 2;
            }

            out << '"' << a << '"';
        }
        else
        {
            out << argv_[arg];
        }
    }
    out << '\n';
    return out.str();
}

command_line_options::command_line_options( const command_line_options& rhs ) { operator=( rhs ); }

const std::vector< std::string >& command_line_options::argv() const { return argv_; }

bool command_line_options::exists( const std::string& name ) const
{
    std::vector< std::string > names = comma::split( name, ',' );
    for( std::size_t i = 0; i < names.size(); ++i )
    {
        if( map_.find( names[i] ) != map_.end() ) { return true; }
    }
    return false;
}

std::vector< std::string > command_line_options::unnamed( const std::string& valueless_options, const std::string& options_with_values ) const
{
    std::vector< std::string > valueless = split( valueless_options, ',' );
    std::vector< std::string > valued = split( options_with_values, ',' );
    std::vector< std::string > w;
    for( unsigned int i = 1; i < argv_.size(); ++i )
    {
        bool is_valueless = false;
        for( unsigned int k = 0; !is_valueless && k < valueless.size(); ++k ) { is_valueless = boost::regex_match( argv_[i], boost::regex( valueless[k] ) ); }
        if( is_valueless ) { continue; }
        bool is_valued = false;
        bool has_equal_sign = false;
        for( unsigned int j = 0; !is_valued && j < valued.size(); ++j )
        {
            has_equal_sign = boost::regex_match( argv_[i], boost::regex( valued[j] + "=.*" ) );
            is_valued = boost::regex_match( argv_[i], boost::regex( valued[j] ) ) || has_equal_sign;
        }
        if( is_valued ) { if( !has_equal_sign ) { ++i; } continue; }
        w.push_back( argv_[i] );
    }
    return w;
}

std::vector< std::string > command_line_options::names() const
{
    return names_;
}

void command_line_options::fill_map_( const std::vector< std::string >& v )
{
    for( std::size_t i = 1; i < v.size(); ++i )
    {
        if( v[i].length() < 2 || v[i].at( 0 ) != '-') { continue; }
        std::string name;
        boost::optional< std::string > value;
        std::size_t equal = v[i].find_first_of( '=' );
        if( equal == std::string::npos )
        {
            name = v[i];
            if( ( i + 1 ) < v.size() ) { value = v[ i + 1 ]; }
        }
        else
        {
            name = v[i].substr( 0, equal );
            if( ( equal + 1 ) < v[i].length() ) { value = v[i].substr( equal + 1 ); }
        }
//         if( v[i].at( 1 ) == '-' )
//         {
//             if( v[i].length() < 3 ) { continue; }
//             if( equal == std::string::npos )
//             {
//                 name = v[i];
//                 // check if the option has a value, i.e. not starting with '-' unless it is a negative number
//                 if( ( i + 1 < v.size() ) && ( ( v[i+1][0] != '-' ) || isdigit( v[i+1][1] ) ) ) { value = v[ i + 1 ]; }
//             }
//             else
//             {
//                 name = v[i].substr( 0, equal );
//                 value = v[i].substr( equal + 1 );
//             }
//         }
//         else
//         {
//             name = v[i];
//             if( i + 1 < v.size() ) { value = v[ i + 1 ]; }
//         }
        std::vector< std::string >& values = map_[name];
        if( value ) { values.push_back( *value ); }
        if( name.size() != 0u )
        {
            names_.push_back( name );
        }
    }
}

void command_line_options::assert_mutually_exclusive( const std::string& names ) const
{
    std::vector< std::string > v = comma::split( names, ',' );
    std::size_t count = 0;
    for( std::size_t i = 0; i < v.size(); ++i )
    {
        count += exists( v[i] );
        if( count > 1 ) { COMMA_THROW( comma::exception, "options " << names << " are mutually exclusive" ); }
    }

}

} // namespace comma {
