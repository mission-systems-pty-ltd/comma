// Copyright (c) 2024 Mission Systems

/// @authors aspen eyers, vsevolod vlaskine

#pragma once

#include <deque>
#include <tuple>

namespace comma { namespace containers { namespace ordered {

/// @todo variadic types
/// @todo don't use std::pair, use traits instead?
/// @todo max_diff, grater or grater_equal? & document; unit test on ints; unit test on max diff 0]
/// @todo pop_all will remove the first elements from both queues, but we may want to pop just one element and still keep the other one
///       Use case: we want to get every (valid) element from both queues with its corresponding element from the other queue and process them
///       independently. If we pop both elements, we may lose the correspondence between the elements from the two queues. 
template < typename K, typename T, typename S >
class queues: public std::tuple< std::deque< std::pair< K, T > >, std::deque< std::pair< K, S > > >
{
    public:
        typedef std::tuple< std::deque< std::pair< K, T > >, std::deque< std::pair< K, S > > > queues_type;
        typedef std::tuple< std::pair< K, T >, std::pair< K, S > > values_type;
        typedef std::tuple< const std::pair< K, T >&, const std::pair< K, S >& > ref_type;
        typedef decltype( K() - K() ) diff_type;
        queues( diff_type max_diff ): _max_diff( max_diff ) {}
        bool ready() const;
        void purge();
        void pop_all();
        ref_type front() const;

    private:
        diff_type _max_diff;
        static diff_type _abs_diff(K lhs, K rhs) { return lhs < rhs ? (rhs - lhs) : (lhs - rhs); }
        template < unsigned int I, unsigned int J > bool _purge();
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
    // If not purge 0,1, then purge 1,0
    while( std::get<0>(*this).front().first - std::get<1>(*this).front().first > _max_diff ) 
    { 
        if( std::get<1>(*this).empty() ) { return; }
        std::get<1>(*this).pop_front();
    }
    while( std::get<1>(*this).front().first - std::get<0>(*this).front().first > _max_diff ) 
    { 
        if( std::get<0>(*this).empty() ) { return; }
        std::get<0>(*this).pop_front();
    }
}

template < typename K, typename T, typename S >
void queues<K, T, S>::pop_all()
{
    std::get<0>(*this).pop_front();
    std::get<1>(*this).pop_front();
    return;
}


template < typename K, typename T, typename S >
inline typename queues<K, T, S>::ref_type queues<K, T, S>::front() const
{
    return { std::get<0>(*this).front(), std::get<1>(*this).front() };
}

} } } // namespace comma { namespace containers { namespace ordered {
