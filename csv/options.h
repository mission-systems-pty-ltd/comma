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

#pragma once

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
        options( int argc, char** argv, const std::string& defaultFields = "" );

        /// constructor
        options( const comma::command_line_options& options, const std::string& defaultFields = "", bool set_full_xpath = false );

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

} } // namespace comma { namespace csv {
