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

#include <string>
#include <vector>

namespace comma {

namespace string {
    
bool is_one_of( char c, const char* characters );

} // namespace string {

/// split string into tokens (a quick implementation); always contains at least one element
std::vector< std::string > split( const std::string& s, const char* separators = " ", bool empty_if_empty_input = false );

/// split string into tokens (a quick implementation); always contains at least one element
std::vector< std::string > split( const std::string& s, char separator, bool empty_if_empty_input = false );

/// Split string into tokens; always contains at least one element;
/// skips backslash escaped separator, handle non-nested quotes;
/// exceptions thrown on errors.
///
/// An escape character can be anywhere in the string.
/// An escape character at end of string.will be kept.
/// An escape character will only escape a delimiter, quote or escape character;
/// escaping any other character will result in both being kept;
/// e.g. c:\windows\ will be kept as c:\windows\ with the trailing backslash
/// e.g. filename;delimiter=\\;fields=a,b will be kept as filename;delimiter=\;fields=a,b 
///
/// A quote may be anywhere in a string. Quotes must be closed; i.e Each start
/// quote must be paired with an end quote, or an exception is thrown.
/// Quotes don't nest and can not be mixed; e.g. a ' will not close a " quoted string.
/// However "'" and '"' are perfectly legal strings of ' and "
std::vector< std::string > split_escaped( const std::string& s, const char * separators = " ", const char * quotes = "\"\'", char escape = '\\' );
/// split string into tokens; always contains at least one element;
/// skips backslash escaped seperator, handle boolean quotes 
std::vector< std::string > split_escaped( const std::string& s, char separator, const char * quotes = "\"\'", char escape = '\\' );
/// skips bracketed separators
std::vector< std::string > split_bracketed( const std::string& s, const char * separators = " ", char lbracket = '(', char rbrackets = ')', bool strip_brackets = true );
/// skips bracketed separators
std::vector< std::string > split_bracketed( const std::string& s, char separator, char lbracket = '(', char rbracket = ')', bool strip_brackets = true );

} // namespace comma {

