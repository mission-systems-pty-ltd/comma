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
