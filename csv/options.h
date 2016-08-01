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

#ifndef COMMA_CSV_OPTIONS_H_
#define COMMA_CSV_OPTIONS_H_

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
        options( const comma::command_line_options& options, const std::string& defaultFields = "" );

        /// return usage to incorporate into application usage
        static std::string usage( const std::string& default_fields = "" );

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
        
        /// return format string or empty string if no format specified
        std::string format_string() const;

    private:
        boost::optional< csv::format > format_;
};

} } // namespace comma { namespace csv {

#endif /*COMMA_CSV_OPTIONS_H_*/
