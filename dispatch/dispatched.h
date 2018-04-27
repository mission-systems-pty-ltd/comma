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

#ifndef COMMA_DISPATCH_DISPATCHED_H_
#define COMMA_DISPATCH_DISPATCHED_H_

namespace comma { namespace dispatch {

/// base class for handler
struct handler
{ 
    /// virtual destructor
    virtual ~handler() {}
};

/// base class for handler of const values
struct handler_const
{
    /// virtual destructor 
    virtual ~handler_const() {}
};

/// base class for dispatched types
struct dispatched_base
{
    /// virtual destructor
    virtual ~dispatched_base() {}
    
    /// dispatch this to a handler
    virtual void dispatch_to( handler& d ) = 0;
    
    /// dispatch this to a handler of const
    virtual void dispatch_as_const_to( handler& d ) const = 0;
    
    /// dispatch this to a handler
    virtual void dispatch_to( handler* d ) = 0;
    
    /// dispatch this to a handler of const
    virtual void dispatch_as_const_to( handler* d ) const = 0;    
};

/// handler of a type
template < typename T >
struct handler_of : virtual public handler
{
    /// virtual destructor
    virtual ~handler_of() {}
    
    /// handle a dispatched value
    virtual void handle( T& t ) = 0;
};

/// handler of a const type
template < typename T >
struct handler_of_const : virtual public handler
{
    /// virtual destructor
    virtual ~handler_of_const() {}
    
    /// handle a dispatched value
    virtual void handle( const T& t ) = 0;
};

/// dispatched type
/// double dispatch implementation
/// 
///             An example of the problem:
///                 - we have a vector of base-class pointers to polymorphic objects
///                     of types A, B, C (dispatched types), etc. all derived from class base
///                 - every type should be handled differently, but we don't want to
///                     put the virtual methods for this handling in the base class
/// 
///             Solution (a double-dispatch pattern):
///                 - classes A, B, C, etc just derive from the common base (base),
///                     but base does not define any virtual methods for the derived
///                     classes
///                 - handler classes deriving from a common base Handler provide
///                     methods for handling various data types
///                 - double dispatching (using two dynamic casts) makes sure that
///                     the correct handlers are called
/// 
template < typename T, typename Base = dispatched_base >
struct dispatched : public Base
{
    /// base type
    typedef Base base_t;
    
    /// dispatch this to a handler
    void dispatch_to( handler* d )
    {
        dynamic_cast< handler_of< T >* >( d )->handle( dynamic_cast< T& >( *this ) );
    }
    
    /// dispatch this to a handler of const
    void dispatch_as_const_to( handler* d ) const
    {
        dynamic_cast< handler_of_const< T >* >( d )->handle( dynamic_cast< const T& >( *this ) );
    }
    
    /// dispatch this to a handler
    void dispatch_to( handler& d )
    {
        dynamic_cast< handler_of< T >& >( d ).handle( dynamic_cast< T& >( *this ) );
    }
    
    /// dispatch this to a handler of const
    void dispatch_as_const_to( handler& d ) const
    {
        dynamic_cast< handler_of_const< T >& >( d ).handle( dynamic_cast< const T& >( *this ) );
    }
};

} } // namespace comma { namespace dispatch {

#endif /*COMMA_DISPATCH_DISPATCHED_H_*/
