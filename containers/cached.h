// Copyright (c) 2024 Vsevolod Vlaskine
// All Rights Reserved

#include <deque>
#include <unordered_map>

namespace comma {

template < typename T, typename K, typename Hash = std::hash< K > >
class cached
{
    public:
        cached( unsigned int max_size = 0 ): _size( max_size ) {}

        template < typename... Args > T& get( Args... args );

        template < typename... Args > const T& get( Args... args ) const;

        template < typename... Args > auto operator()( Args... args ) { return get( args... )( args... ); }

        template < typename... Args > auto operator()( Args... args ) const { return get( args... )( args... ); }

        void clear() { _values.clear(); }

        void pop( unsigned int size = 1 );

        const std::unordered_map< K, T, Hash >& values() const { return _values; }

    protected:
        mutable std::unordered_map< K, T, Hash > _values;
        mutable std::deque< K > _keys;
        unsigned int _size{0};
};

template < typename T, typename K, typename Hash >
template < typename... Args >
T& cached< T, K, Hash >::get( Args... args )
{
    K k{ args... };
    auto i = _values.find( k );
    if( i != _values.end() ) { return i->second; }
    if( _size > 0 && _values.size() == _size ) { pop(); }
    _keys.emplace_back( k );
    return _values.emplace( std::make_pair( k, T( k ) ) ).first->second;
}

template < typename T, typename K, typename Hash >
template < typename... Args >
const T& cached< T, K, Hash >::get( Args... args ) const
{
    K k{ args... };
    auto i = _values.find( k );
    if( i != _values.end() ) { return i->second; }
    if( _size > 0 && _values.size() == _size ) { pop(); }
    _keys.emplace_back( k );
    return _values.emplace( std::make_pair( k, T( k ) ) ).first->second;
}

template < typename T, typename K, typename Hash >
inline void cached< T, K, Hash >::pop( unsigned int size )
{
    for( unsigned int i = 0; i < size; ++i )
    {
        if( _keys.empty() ) { return; }
        _values.erase( _keys.front() );
        _keys.pop_front();
    }
}

} // namespace comma {
