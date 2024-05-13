// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#pragma once

#include <vector>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include "../base/exception.h"
#include "../math/cyclic.h"

namespace comma {

/// an arbitrary cyclic buffer on top of the given container;
/// allocates size copies of given instance (or default-constructed T);
/// see unit test for more usage;
/// @todo: maybe, allocate buffers not during the construction,
///        but on push() and deallocate on pop()
template < typename T >
class cyclic_buffer
{
    public:
        cyclic_buffer( std::size_t size, const T& t = T() );
        
        cyclic_buffer( const cyclic_buffer& rhs ) { operator=( rhs ); }
        
        const cyclic_buffer& operator=( const cyclic_buffer& rhs );
        
        T& front();
        
        const T& front() const;
        
        T& back();
        
        const T& back() const;

        std::size_t front_index() const { return begin_(); }

        std::size_t end_index() const { return end_(); }
        
        void push( const T& t, bool force = false );
        
        template < typename Iterator >
        void push( Iterator begin, Iterator end, bool force = false );
        
        void pop( std::size_t n = 1 );
        
        std::size_t size() const;
        
        std::size_t capacity() const;
        
        bool empty() const;
        
        void clear();

        const std::vector< T >& data() const { return vector_; }

        std::vector< T >& data() { return vector_; }
        
    protected:
        std::vector< T > vector_;
        math::cyclic< std::size_t > begin_;
        math::cyclic< std::size_t > end_;
        bool empty_;
};

/// an arbitrary cyclic buffer of fixed size on top of the given container,
/// i.e. you cannot Push() and Pop() on it, but it is preallocated
template < typename T, std::size_t S >
class fixed_cyclic_buffer : public cyclic_buffer< T >
{
    public:
        /// constructor
        fixed_cyclic_buffer( const T& t = T() );
        
        /// copy constructor
        fixed_cyclic_buffer( const fixed_cyclic_buffer& rhs ) { operator=( rhs ); }
        
        /// shift indices up by given number
        void operator>>( std::size_t i );
        
        /// shift indices down by given number
        void operator<<( std::size_t i );
        
        /// return reference to element relative to the current begin
        T& operator[]( std::size_t i );
        
        /// return reference to element relative to the current begin
        const T& operator[]( std::size_t i ) const;
        
    private:
        void push();
        void pop();
        void clear();
};

template < typename T >
inline cyclic_buffer< T >::cyclic_buffer( std::size_t size, const T& t )
    : vector_( size, t )
    , begin_( 0, size )
    , end_( 0, size )
    , empty_( true )
{
    assert( size > 0 );
}

template < typename T >
inline T& cyclic_buffer< T >::front()
{ 
    if( empty() ) { COMMA_THROW( comma::exception, "empty" ); }
    return vector_[ begin_() ];
}

template < typename T >
inline const T& cyclic_buffer< T >::front() const
{ 
    if( empty() ) { COMMA_THROW( comma::exception, "empty" ); }
    return vector_[ begin_() ];
}

template < typename T >
inline T& cyclic_buffer< T >::back()
{ 
    if( empty() ) { COMMA_THROW( comma::exception, "empty" ); }
    return vector_[ ( end_ - 1 )() ];
}

template < typename T >
inline const T& cyclic_buffer< T >::back() const
{ 
    if( empty() ) { COMMA_THROW( comma::exception, "empty" ); }
    return vector_[ ( end_ - 1 )() ];
}

template < typename T >
inline std::size_t cyclic_buffer< T >::capacity() const { return vector_.size(); }

template < typename T >
inline void cyclic_buffer< T >::push( const T& t, bool force )
{
    if( size() == vector_.size() )
    { 
        if( !force ) { COMMA_THROW( comma::exception, "full" ); }
        vector_[ begin_() ] = t;
        ++begin_;
    }
    else
    {
        vector_[ end_() ] = t;
    }
    ++end_;
    empty_ = false;
}

template < typename T >
template < typename Iterator >
inline void cyclic_buffer< T >::push( Iterator begin, Iterator end, bool force )
{
    for( Iterator it = begin; it != end; ++it ) { push( *it, force ); }
}

template < typename T >
inline void cyclic_buffer< T >::pop( std::size_t n )
{
    if( empty() ) { return; }
    if( n >= size() ) { clear(); return; }
    begin_ += n;
    empty_ = begin_ == end_;
}

template < typename T >
inline std::size_t cyclic_buffer< T >::size() const
{
    std::size_t s = ( end_ - begin_ )();
    return s == 0 && !empty_ ? vector_.size() : s;
}

template < typename T >
inline bool cyclic_buffer< T >::empty() const { return empty_; }

template < typename T >
inline void cyclic_buffer< T >::clear() { empty_ = true; end_ = begin_; }

template < typename T, std::size_t S >
inline fixed_cyclic_buffer< T, S >::fixed_cyclic_buffer( const T& t )
    : cyclic_buffer< T >( S, t )
{
    this->empty_ = false;
}

template < typename T, std::size_t S >
inline void fixed_cyclic_buffer< T, S >::operator>>( std::size_t i )
{
    this->begin_ += i;
    this->end_ = this->begin_;
}

template < typename T, std::size_t S >
inline void fixed_cyclic_buffer< T, S >::operator<<( std::size_t i )
{
    this->begin_ -= i;
    this->end_ = this->begin_;
}

template < typename T, std::size_t S >
inline T& fixed_cyclic_buffer< T, S >::operator[]( std::size_t i )
{
    return this->vector_[ ( this->begin_ + i )() ];
}

template < typename T, std::size_t S >
inline const T& fixed_cyclic_buffer< T, S >::operator[]( std::size_t i ) const
{
    return this->vector_[ ( this->begin_ + i )() ];
}

} // namespace comma {
