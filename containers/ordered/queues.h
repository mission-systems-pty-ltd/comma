// Copyright (c) 2024 Mission Systems

/// @authors aspen eyers, vsevolod vlaskine

#pragma once

#include <deque>
#include <tuple>
#include "../../base/optional.h" // since old compilers are not aware of std::optional

namespace comma { namespace containers { namespace ordered {

/// @todo variadic types
/// @todo don't use std::pair, use traits instead?
/// @todo max_diff, greater or greater_equal? & document; unit test on ints; unit test on max diff 0]
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
        template < unsigned int I > auto& get() { return std::get< I >( *this ); } // convenience method
        template < unsigned int I > const auto& get() const { return std::get< I >( *this ); } // convenience method
        auto& first() { return get< 0 >(); } // convenience method
        const auto& first() const { return get< 0 >(); } // convenience method
        auto& second() { return get< 1 >(); } // convenience method
        const auto& second() const { return get< 1 >(); } // convenience method

    protected:
        diff_type _max_diff;
        static diff_type _abs_diff(K lhs, K rhs) { return lhs < rhs ? (rhs - lhs) : (lhs - rhs); }
        template < unsigned int I, unsigned int J > bool _purge();
};

template < typename K, typename T, typename S >
class bounded_queues: public queues< K, T, S >
{
    public:
        using base_type = queues< K, T, S >;
        using diff_type = typename base_type::diff_type;

        bounded_queues( diff_type max_diff ): base_type( max_diff ) {}

        bounded_queues( diff_type max_diff, diff_type max_bounding_diff ): base_type( max_diff ), _max_bounding_diff( max_bounding_diff ) {}

        template < unsigned int BoundingIndex >
        bool front_bounded_by() const;

        template < unsigned int BoundingIndex >
        void pop_stale();

    protected:
        comma::optional< diff_type > _max_bounding_diff{};
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
    while( !std::get<0>(*this).empty() && !std::get<1>(*this).empty() )
    {
        auto diff = std::get<0>(*this).front().first - std::get<1>(*this).front().first;
        if( diff > _max_diff ) { std::get<1>(*this).pop_front(); continue; }
        if( -diff > _max_diff ) { std::get<0>(*this).pop_front(); continue; }
        break;
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

template < typename K, typename T, typename S >
template < unsigned int N >
inline bool bounded_queues< K, T, S >::front_bounded_by() const
{
    static_assert( N == 0 || N == 1 );
    const auto& bounding = std::get< N >( *this );
    const auto& bound = std::get< 1 - N >( *this );
    return this->ready()
        || (    !bound.empty()
             && bounding.size() > 1
             && ( !_max_bounding_diff || bounding[1].first - bounding[0].first <= *_max_bounding_diff )
             && bounding[0].first <= bound.front().first
             && bounding[1].first >= bound.front().first );
}

template < typename K, typename T, typename S >
template < unsigned int N >
inline void bounded_queues< K, T, S >::pop_stale()
{
    static_assert( N == 0 || N == 1 );
    auto& bounding = std::get< N >( *this );
    auto& bound = std::get< 1 - N >( *this );
    while( !bound.empty() && !bounding.empty() )
    {
        if( this->ready() ) { return; }
        
        if( bound.front().first < bounding.front().first ) { bound.pop_front(); continue; }
        if( _max_bounding_diff && bound.front().first - bounding.front().first >= *_max_bounding_diff ) { bounding.pop_front(); continue; }
        if( bounding.size() == 1 ) { return; }
        if( bounding[1].first < bound.front().first || ( _max_bounding_diff && bounding[1].first - bounding[0].first > *_max_bounding_diff ) ) { bounding.pop_front(); continue; }
        return;
    }
}

} } } // namespace comma { namespace containers { namespace ordered {
