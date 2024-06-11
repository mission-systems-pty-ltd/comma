// Copyright (c) 2024 Mission Systems

/// @authors aspen eyers, vsevolod vlaskine

#pragma once

#include <queue>
#include <tuple>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace comma { namespace containers { namespace ordered {

/// @todo variadic types
/// @todo don't use std::pair, use traits instead?
template < typename K, typename T, typename S >
class queues: public std::tuple< std::queue< std::pair< K, T > >, std::queue< std::pair< K, S > > >
{
    public:
        typedef std::tuple< std::queue< std::pair< K, T > >, std::queue< std::pair< K, S > > > queues_type;
        typedef decltype( K() - K() ) diff_type;
        queues( diff_type max_diff ): _max_diff( max_diff ) {}
        bool ready() const;
        void purge();

    private:
        diff_type _max_diff;
        static diff_type _abs_diff(K lhs, K rhs) { return lhs < rhs ? (rhs - lhs) : (lhs - rhs); }
};

template < typename K, typename T, typename S >
inline bool queues<K, T, S>::ready() const
{
    if( std::get<0>(*this).empty() || std::get<1>(*this).empty() ) { return false; }
    return _abs_diff( std::get<1>(*this).front().first, std::get<0>(*this).front().first ) <= _max_diff;
}

template < typename K, typename T, typename S >
inline void queues<K, T, S>::purge()
{
    if( std::get<1>(*this).empty() || std::get<0>(*this).empty() ) { return; }
    while( std::get<0>(*this).front().first - std::get<1>(*this).front().first > _max_diff ) 
    { 
        if( std::get<1>(*this).empty() ) { return; }
        std::get<1>(*this).pop(); 
    }
    while( std::get<1>(*this).front().first - std::get<0>(*this).front().first > _max_diff ) 
    { 
        if( std::get<0>(*this).empty() ) { return; }
        std::get<0>(*this).pop(); 
    }
}

} } } // namespace comma { namespace containers { namespace ordered {
