// Copyright (c) 2024 Vsevolod Vlaskine
// All Rights Reserved

#include <unordered_map>

namespace comma {

template < typename T, typename K >
class cached
{
    public:
        template < typename... Args >
        T& operator()( Args... args );

        void clear() { _cache.clear(); }

        const std::unordered_map< K, T >& cache() const { return _cache; }

    private:
        std::unordered_map< K, T > _cache;
};

template < typename T, typename K >
template < typename... Args >
T& cached< T, K >::operator()( Args... args )
{
    K k{ args... };
    auto i = _cache.find( k );
    return ( i == _cache.end() ? _cache.insert_emplace( k, T( k ) ).first : i )->second;
}

} // namespace comma {
