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

#ifndef COMMA_CSV_OPTIONS_H_
#define COMMA_CSV_OPTIONS_H_

#include <comma/application/command_line_options.h>
#include <comma/csv/format.h>
#include <comma/visiting/traits.h>

namespace comma { namespace csv {

/// a helper class to extract csv-related command line options
class options
{
    public:
        /// constructor
        options();

        /// constructor
        options( int argc, char** argv, const std::string& defaultFields = "" );

        /// constructor
        options( const comma::command_line_options& options, const std::string& defaultFields = "" );

        /// return usage to incorporate into application usage
        static std::string usage();

        /// filename (optional)
        std::string filename;

        /// if true, expect full xpaths as field names;
        /// e.g. "point/scalar" rather than "scalar"
        bool full_xpath;

        /// field (column) names
        std::string fields;

        /// csv delimiter
        char delimiter;

        /// precision
        unsigned int precision;

        /// return format
        const csv::format& format() const;

        /// return format
        csv::format& format();

        /// set format
        void format( const std::string& s );

        /// set format
        void format( const csv::format& f );

        /// true, if --binary specified
        bool binary() const;

        /// return true, if fields have given field (convenience function, slow)
        /// @param field comma-separated fields, e.g. "x,y,z"
        bool has_field( const std::string& field ) const;

    private:
        boost::optional< csv::format > format_;
};

} } // namespace comma { namespace csv {

namespace comma { namespace visiting {

/// visiting traits
template <> struct traits< comma::csv::options >
{
    /// const visiting
    template < typename Key, class Visitor >
    static void visit( const Key&, const comma::csv::options& p, Visitor& v )
    {
        v.apply( "filename", p.filename );
        v.apply( "delimiter", p.delimiter );
        v.apply( "fields", p.fields );
        v.apply( "full-xpath", p.full_xpath );
        v.apply( "precision", p.precision );
        if( p.binary() ) { v.apply( "binary", p.format().string() ); }
    }

    /// visiting
    template < typename Key, class Visitor >
    static void visit( Key, comma::csv::options& p, Visitor& v )
    {
    	v.apply( "filename", p.filename );
        v.apply( "delimiter", p.delimiter );
        v.apply( "fields", p.fields );
        v.apply( "full-xpath", p.full_xpath );
        v.apply( "precision", p.precision );
        std::string s;
        v.apply( "binary", s );
        if( s != "" ) { p.format( s ); }
    }
};

} } // namespace comma { namespace visiting {

#endif /*COMMA_CSV_OPTIONS_H_*/
