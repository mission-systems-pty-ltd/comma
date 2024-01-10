// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#pragma once

#include <unordered_map>
#include <boost/optional.hpp>
#include "../application/command_line_options.h"
#include "format.h"

namespace comma { namespace csv {

/// a helper class to extract csv-related command line options
class options
{
    public:
        /// constructor
        options();

        /// constructor
        options( int argc, char** argv, const std::string& default_fields = "", bool full_xpath = true );

        /// constructor
        options( const comma::command_line_options& options, const std::string& default_fields = "", bool full_xpath = true );

        /// constructor
        options( const comma::command_line_options& options, const std::unordered_map< std::string, std::string >& field_aliases, const std::string& default_fields = "" );

        /// make options from input options (propagate binary setting, flush, delimiter, etc)
        /// typical use: make output stream options compatible with the input stream options
        template < typename T >
        static options make_same_kind( const options& rhs );

        /// return usage to incorporate into application usage
        static std::string usage( const std::string& default_fields = "", bool verbose = true );

        /// return usage to incorporate into application usage
        static std::string usage( bool verbose ) { return usage( "", verbose ); }

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

        /// quote sign for strings
        boost::optional< char > quote;

        /// if true, flush output stream after each record
        bool flush;

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

        /// return true, if fields have all given fields (convenience function, slow)
        /// @param field comma-separated fields, e.g. "x,y,z"
        bool has_field( const std::string& fields_to_check ) const;

        /// return true, if fields have some given fields (convenience function, slow)
        /// @param field comma-separated fields, e.g. "x,y,z"
        bool has_some_of_fields( const std::string& fields_to_check ) const;

        /// return true, if fields have all given paths (convenience function, slow)
        /// @param field comma-separated fields, e.g. fields "centre/position/x,centre/position/y,centre/position/z,..."
        ///              have paths 'centre/position', 'centre', 'centre/position/x', etc
        bool has_paths( const std::string& paths ) const;

        /// return true, if fields have some given paths (convenience function, slow)
        /// @param field comma-separated fields, e.g. fields "centre/position/x,centre/position/y,centre/position/z,..."
        ///              have paths 'centre/position', 'centre', 'centre/position/x', etc
        bool has_some_of_paths( const std::string& paths ) const;

        /// returns comma separated list of valueless csv options that can be passed to command_line_options.unnamed
        static std::string valueless_options();

    private:
        boost::optional< csv::format > format_;
};

template < typename T >
inline options options::make_same_kind( const options& rhs )
{
    options o;
    o.flush = rhs.flush;
    o.delimiter = rhs.delimiter;
    o.flush = rhs.flush;
    o.precision = rhs.precision;
    o.quote = rhs.quote;
    if( rhs.binary() ) { o.format( format::value< T >() ); }
    return o;
}

} } // namespace comma { namespace csv {
