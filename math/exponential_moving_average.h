// Copyright (c) 2023 Vsevolod Vlaskine
// All rights reserved.

/// @author vsevolod vlaskine

#pragma once

#include "../base/exception.h"
#include "../base/none.h"
#include "../base/types.h"

namespace comma { namespace math {

template < typename T >
class exponential_moving_average
{
    public:
        typedef T value_type;

        exponential_moving_average( double alpha = 0.5, comma::uint64 initial_count = 1 );

        template < typename S >
        exponential_moving_average& operator+=( const S& lhs );

        const T& operator()() const { if( _count > 0 ) { return _value; } COMMA_THROW( comma::exception, "no values provided yet" ); }

        comma::uint64 count() const { return _count; }

    private:
        double _alpha{0.5};
        comma::uint64 _initial_count{1};
        comma::uint64 _count{0};
        T _value;
};


template < typename T >
inline exponential_moving_average< T >::exponential_moving_average( double alpha, comma::uint64 initial_count )
    : _alpha( alpha )
    , _initial_count( initial_count )
{
}

// todo! better casting, otherwise it does not work well on integral-like values (e.g. integers or boost::posix_time::time_duration)
template < typename T >
template < typename S >
inline exponential_moving_average< T >& exponential_moving_average< T >::operator+=( const S& t )
{
    if( _count == 0 ) { _value = T( t ); }
    else if( _count < _initial_count ) { _value = ( _value * _count + T( t ) ) * 1. / ( _count + 1 ); }
    else { _value = _value * ( 1. - _alpha ) + T( t ) * _alpha; }
    ++_count;
    return *this;
}


} } // namespace comma { namespace math {
