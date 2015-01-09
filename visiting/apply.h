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
//    This product includes software developed by the University of Sydney.
// 4. Neither the name of the University of Sydney nor the
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

#ifndef COMMA_VISITING_APPLY_H_
#define COMMA_VISITING_APPLY_H_

namespace comma { namespace visiting {

namespace impl {

template < typename V >
class applier_
{
    public:
        applier_( V& v ) : m_visitor( v ) {}
        template < typename T > V& to( const T& t ) { m_visitor.apply( "", t ); return m_visitor; }
        template < typename T > V& to( T& t ) { m_visitor.apply( "", t ); return m_visitor; }
    private:
        V& m_visitor;
};
    
} // namespace impl {

/// apply visitor function    
template < typename Visitor, typename T >
inline void apply( Visitor& v, T& t ) { v.apply( "", t ); }

/// apply visitor function 
template < typename Visitor, typename T >
inline void apply( Visitor& v, const T& t ) { v.apply( "", t ); }

/// apply visitor function
template < typename Visitor >
inline impl::applier_< Visitor > apply( Visitor& v ) { return impl::applier_< Visitor >( v ); }

} } // namespace comma { namespace visiting {

#endif // COMMA_VISITING_APPLY_H_
