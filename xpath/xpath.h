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

#ifndef COMMA_XPATH_XPATH_HEADER_GUARD_
#define COMMA_XPATH_XPATH_HEADER_GUARD_

#ifndef WIN32
#include <float.h>
#include <stdlib.h>
#endif
#include <iostream>
#include <string>
#include <vector>
#include <boost/optional.hpp>

namespace comma {

/// xpath, like "hello/world[5]/moon"
class xpath
{
    public:
        /// xpath element
        struct element
        {
            /// element name 
            std::string name;
            
            /// optionally, element index
            boost::optional< std::size_t > index;
            
            /// constructor
            element( const std::string& name );
            
            /// constructor
            element( const std::string& name, std::size_t index );
            
            /// constructor
            element( const std::string& name, boost::optional< std::size_t > index );
            
            /// default constructor
            element();
            
            /// copy constructor
            element( const element& rhs );
            
            /// comparison
            bool operator==( const element& rhs ) const;
            
            /// comparison
            bool operator!=( const element& rhs ) const;
            
            /// comparison
            bool operator<( const element& rhs ) const;
            
            /// comparison
            bool operator<=( const element& rhs ) const;
            
            /// to string
            std::string to_string() const;
        };
        
        /// constructor
        xpath( const std::string& str, char delimiter = '/' );
        
        /// constructor
        xpath( const char* str, char delimiter = '/' );

        /// constructor
        xpath( const xpath::element& rhs, char delimiter = '/' );
        
        /// default constructor
        xpath();
        
        /// copy constructor
        xpath( const xpath& rhs );
        
        /// append
        const xpath& operator/=( const xpath& );
        const xpath& operator/=( const xpath::element& );
        const xpath& operator/=( const std::string& );
        xpath operator/( const xpath& ) const;
        xpath operator/( const xpath::element& ) const;
        xpath operator/( const std::string& ) const;
        
        /// equality
        bool operator==( const xpath& rhs ) const;
        
        /// inequality
        bool operator!=( const xpath& rhs ) const;
        
        /// return true, if it's a subpath of rhs, e.g hello/world < hello
        bool operator<( const xpath& rhs ) const;
        
        /// return true, if xpath is less or equal to rhs
        bool operator<=( const xpath& rhs ) const;
        
        /// return xpath without the first element; if empty, return empty xpath
        xpath tail() const;
        
        /// return xpath without the last element; if empty, return empty xpath
        xpath head() const;
        
        /// xpath elements
        std::vector< element > elements;
        
        /// return as string
        std::string to_string( char delimiter = '/' ) const;
};

} // namespace comma {

#endif // #ifndef COMMA_XPATH_XPATH_HEADER_GUARD_
