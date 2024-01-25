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

#include <boost/regex.hpp>
#include "../base/exception.h"
#include "../csv/options.h"
#include "../string/split.h"
#include "../string/string.h"

namespace comma { namespace csv {

const csv::format& options::format() const { return *format_; }

csv::format& options::format() { return *format_; }

void options::format( const std::string& s ) { format_ = csv::format( s ); }

void options::format( const csv::format& f ) { format_ = f; }

bool options::binary() const { return static_cast< bool >( format_ ); }

namespace impl {

static void init( comma::csv::options& csv_options
                , const comma::command_line_options& options
                , const std::string& default_fields
                , bool full_xpath
                , const std::unordered_map< std::string, std::string >& field_aliases )
{
    csv_options.full_xpath = full_xpath;
    csv_options.fields = comma::replace( options.value( "--fields,-f", default_fields ), field_aliases );
    if( options.exists( "--binary,-b" ) )
    {
        boost::optional< std::string > format = options.optional< std::string >( "--binary,-b" );
        if( format ) { csv_options.format( options.value< std::string >( "--binary,-b" ) ); }
    }
    csv_options.precision = options.value< unsigned int >( "--precision", 12 );
    csv_options.delimiter = options.exists( "--delimiter" ) ? options.value( "--delimiter", ',' ) : options.value( "-d", ',' );
    boost::optional< std::string > quote_character = options.optional< std::string >( "--quote" );
    if( quote_character )
    {
        switch( quote_character->size() )
        {
            case 0: csv_options.quote.reset(); break;
            case 1: csv_options.quote = ( *quote_character )[0]; break;
            case 2: COMMA_THROW( comma::exception, "expected a quote character, got \"" << *quote_character << "\"" );
        }
    }
    csv_options.flush = options.exists( "--flush" );
}

} // namespace impl {

options::options(): full_xpath( true ), delimiter( ',' ), precision( 12 ), quote( '"' ), flush( false ) {}

options::options( int argc, char** argv, const std::string& default_fields, bool full_xpath ): options( comma::command_line_options( argc, argv ), default_fields, full_xpath ) {}

options::options( const comma::command_line_options& options, const std::string& default_fields, bool full_xpath ) { impl::init( *this, options, default_fields, full_xpath, {} ); }

options::options( const comma::command_line_options& options, const std::unordered_map< std::string, std::string >& field_aliases, const std::string& default_fields ) { impl::init( *this, options, default_fields, true, field_aliases ); }

std::string options::usage( const std::string& default_fields, bool verbose )
{
    std::ostringstream oss;
    if( verbose )
    {
        oss << "    --delimiter,-d <delimiter>: default: ','" << std::endl;
        oss << "    --fields,-f <names>: comma-separated field names";
        if( !default_fields.empty() ) { oss << "; default: " << default_fields; }
        oss << std::endl;
        oss << "    --precision <precision>: floating point precision; default: 12" << std::endl;
        oss << "    --quote=[<quote_character>]: quote sign to quote strings (ascii only); default: '\"'" << std::endl;
        oss << "    --flush: if present, flush output stream after each record" << std::endl;
        oss << "    --format <format>: explicitly set input format in csv mode (if not set, guess format from first line)" << std::endl;
        oss << "    --binary,-b <format>: use binary format" << std::endl;
        oss << format::usage();
    }
    else
    {
        oss << "    --binary,-b <format>: use binary format" << std::endl;
        oss << "    --delimiter,-d <delimiter>: default: ','" << std::endl;
        oss << "    --fields,-f <names>: comma-separated field names";
        if( !default_fields.empty() ) { oss << "; default: " << default_fields; }
        oss << std::endl;
        oss << "    --flush: if present, flush output stream after each record" << std::endl;
        oss << "    --precision <precision>: floating point precision; default: 12" << std::endl;
        oss << std::endl;
        oss << "    run with verbose for full csv options description..." << std::endl;
    }
    return oss.str();
}

bool options::has_field( const std::string& field ) const
{
    if( field.empty() ) { return false; }
    const auto& v = split( fields, ',' );
    const auto& f = split( field, ',' );
    for( unsigned int i = 0; i < f.size(); ++i ) { if( std::find( v.begin(), v.end(), f[i] ) == v.end() ) { return false; } }
    return true;
}

std::map< std::string, unsigned int > options::indices() const
{
    std::map< std::string, unsigned int > m;
    const auto& v = split( fields, ',' );
    for( unsigned int i = 0; i < v.size(); ++i ) { if( !v[i].empty() ) { m[v[i]] = i; } }
    return m;
}

bool options::has_some_of_fields( const std::string& field ) const
{
    if( field.empty() ) { return false; }
    const std::vector< std::string >& v = split( fields, ',' );
    const std::vector< std::string >& f = split( field, ',' );
    for( unsigned int i = 0; i < f.size(); ++i ) { if( std::find( v.begin(), v.end(), f[i] ) != v.end() ) { return true; } }
    return false;
}

bool options::has_paths( const std::string& paths ) const
{
    if( paths.empty() ) { return false; }
    const std::vector< std::string >& v = split( fields, ',' );
    const std::vector< std::string >& p = split( paths, ',' );
    for( unsigned int i = 0; i < p.size(); ++i )
    {
        std::string regex_string = "^" + boost::regex_replace( boost::regex_replace( p[i], boost::regex( "\\[" ), "\\\\[" ), boost::regex( "\\]" ), "\\\\]" ) + "(([/\\[])(.*)){0,1}";
        boost::regex regex( regex_string, boost::regex::extended );
        bool found = false;
        for( unsigned int j = 0; j < v.size() && !found; ++j ) { if( boost::regex_match( v[j], boost::regex( regex_string, boost::regex::extended ) ) ) { found = true; } }
        if( !found ) { return false; }
    }
    return true;
}

bool options::has_some_of_paths( const std::string& paths ) const
{
    if( paths.empty() ) { return false; }
    const std::vector< std::string >& v = split( fields, ',' );
    const std::vector< std::string >& p = split( paths, ',' );
    for( unsigned int i = 0; i < p.size(); ++i )
    {
        std::string regex_string = "^" + boost::regex_replace( boost::regex_replace( p[i], boost::regex( "\\[" ), "\\\\[" ), boost::regex( "\\]" ), "\\\\]" ) + "(([/\\[])(.*)){0,1}";
        boost::regex regex( regex_string, boost::regex::extended );
        for( unsigned int j = 0; j < v.size(); ++j ) { if( boost::regex_match( v[j], regex ) ) { return true; } }
    }
    return false;
}

std::string options::valueless_options() { return "--flush"; }

} } // namespace comma { namespace csv {
