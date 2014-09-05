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

/// @author m-hounsell-acfr

#ifndef COMMA_XPATH_EXPAT_UTIL_HEADER_GUARD_
#define COMMA_XPATH_EXPAT_UTIL_HEADER_GUARD_

#include <iosfwd>
#include <string>

#include <expat.h>

class simple_expat_application
{
public:
    simple_expat_application(char const * const name);
    
    int
    run(std::string const & filename);

    void 
    default_handler(XML_Char const * const str, int const length);
    
    void
    element_start(char const * const element, char const * const * const attributes);

    void
    element_end(char const * const element);

    unsigned count_of_elements() const { return element_count; }
    
protected:
    char const * const command_name;
    XML_Parser parser;
    
    unsigned element_count;
    unsigned element_found_count;
    unsigned element_depth;
    unsigned element_depth_max;
    
    virtual void 
    do_default(XML_Char const * const str, int const length) {;}
    
    virtual void
    do_element_start(char const * const element, char const * const * const attributes) {;} 

    virtual void
    do_element_end(char const * const element) {;}

private:
    bool
    parse_as_blocks(std::istream & infile);
};

#endif

