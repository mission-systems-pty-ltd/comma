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


/// @author cedric wohlleber

#ifndef COMMA_CSV_PROGRAM_OPTIONS_H_
#define COMMA_CSV_PROGRAM_OPTIONS_H_

#include <string>
#include <boost/program_options.hpp>
#include <comma/csv/options.h>

namespace comma { namespace csv {

struct program_options
{
    /// return option description
    static boost::program_options::options_description description( const char* default_fields = "" );
    
    /// return csv options filled from command line parameters
    static csv::options get( const boost::program_options::variables_map& vm, const csv::options& default_csv = csv::options() );
};

inline boost::program_options::options_description program_options::description( const char* default_fields )
{
    boost::program_options::options_description d;
    d.add_options()
        ( "fields", boost::program_options::value< std::string >()->default_value( default_fields ), "csv fields" )
        ( "binary,b", boost::program_options::value< std::string >(), "csv binary format" )
        ( "delimiter,d", boost::program_options::value< char >()->default_value( ',' ), "csv delimiter" )
        ( "full-xpath", "expect full xpaths as field names" )
        ( "precision", boost::program_options::value< unsigned int >()->default_value( 12 ), "floating point precision" );
    return d;
}

inline csv::options program_options::get( const boost::program_options::variables_map& vm, const csv::options& default_csv )
{
    csv::options csv = default_csv;
    if( vm.count("fields") ) { csv.fields = vm[ "fields" ].as< std::string >(); }
    if( vm.count("delimiter") ) { csv.delimiter = vm[ "delimiter" ].as< char >(); }
    if( vm.count("precision") ) { csv.precision = vm[ "precision" ].as< unsigned int >(); }
    if( vm.count("binary") ) { csv.format( vm[ "binary" ].as< std::string >() ); }
    if( vm.count( "full-xpath" ) ) { csv.full_xpath = true; }
    return csv;
}

} } // namespace comma { namespace csv {

#endif /*COMMA_CSV_PROGRAM_OPTIONS_H_*/
