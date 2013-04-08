// This file is part of comma, a generic and flexible library
// for robotics research.
//
// Copyright (C) 2011 The University of Sydney
//
// comma is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// comma is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
// for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with comma. If not, see <http://www.gnu.org/licenses/>.

/// @author vsevolod vlaskine

#include <comma/csv/options.h>
#include <comma/string/string.h>

namespace comma { namespace csv {

const csv::format& options::format() const { return *format_; }

csv::format& options::format() { return *format_; }

void options::format( const std::string& s ) { format_ = csv::format( s ); }

void options::format( const csv::format& f ) { format_ = f; }

bool options::binary() const { return format_; }

namespace impl {

inline static void init( comma::csv::options& csvoptions, const comma::command_line_options& options, const std::string& defaultFields )
{
    csvoptions.full_xpath = options.exists( "--full-xpath" );
    csvoptions.fields = options.value( "--fields", defaultFields );
    if( options.exists( "--binary" ) )
    {
        boost::optional< std::string > format = options.optional< std::string >( "--binary" );
        if( format )
        {
            csvoptions.format( options.value< std::string >( "--binary" ) );
        }
    }
    csvoptions.precision = options.value< unsigned int >( "--precision", 6 );
    csvoptions.delimiter = options.exists( "--delimiter" ) ? options.value( "--delimiter", ',' ) : options.value( "-d", ',' );
}

} // namespace impl {

options::options() : full_xpath( false ), delimiter( ',' ), precision( 6 ) {}

options::options( int argc, char** argv, const std::string& defaultFields )
{
    impl::init( *this, comma::command_line_options( argc, argv ), defaultFields );
}

options::options( const comma::command_line_options& options, const std::string& defaultFields )
{
    impl::init( *this, options, defaultFields );
}

std::string options::usage()
{
    std::ostringstream oss;
    oss << "    --binary,-b <format> : use binary format" << std::endl;
    oss << "    --delimiter,-d <delimiter> : default: ','" << std::endl;
    oss << "    --fields,-f <names> : field names, e.g. t,,x,y,z" << std::endl;
    oss << "    --full-xpath : expect full xpaths as field names" << std::endl;
    oss << "    --precision <precision> : floating point precision; default: 6" << std::endl;
    oss << format::usage();
    return oss.str();
}

bool options::has_field( const std::string& field ) const
{
    const std::vector< std::string >& v = split( fields, ',' );
    const std::vector< std::string >& f = split( field, ',' );
    for( unsigned int i = 0; i < f.size(); ++i ) { if( std::find( v.begin(), v.end(), f[i] ) == v.end() ) { return false; } }
    return true;
}

} } // namespace comma { namespace csv {
