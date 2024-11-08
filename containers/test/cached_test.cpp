// Copyright (c) 2024 Vsevolod Vlaskine
// All Rights Reserved

#include <gtest/gtest.h>
#include <vector>
#include <boost/functional/hash.hpp>
#include "../cached.h"

struct square
{
    typedef int key;

    square( int x ): a( x ) {}

    int calculate( int x ) const { return a * x; }

    int a{0};
};

TEST( cached, basics )
{
    comma::cached< square, int > c;
    EXPECT_EQ( c.get( 5 ).calculate( 5 ), 25 );
    EXPECT_EQ( c.values().size(), 1 );
    EXPECT_EQ( c.get( 5 ).calculate( 5 ), 25 );
    EXPECT_EQ( c.values().size(), 1 );
    EXPECT_EQ( c.get( 10 ).calculate( 10 ), 100 );
    EXPECT_EQ( c.values().size(), 2 );
    EXPECT_EQ( c.get( 10 ).calculate( 10 ), 100 );
    EXPECT_EQ( c.values().size(), 2 );
    EXPECT_EQ( c.get( 20 ).calculate( 20 ), 400 );
    EXPECT_EQ( c.values().size(), 3 );
    EXPECT_EQ( c.get( 20 ).calculate( 20 ), 400 );
    EXPECT_EQ( c.values().size(), 3 );
    c.pop();
    EXPECT_EQ( c.values().size(), 2 );
    EXPECT_EQ( c.values().size(), 2 );
    EXPECT_EQ( c.get( 20 ).calculate( 20 ), 400 );
    EXPECT_EQ( c.values().size(), 2 );
}

struct someclass
{
    typedef std::pair< int, int > key;

    struct hash
    {
        std::size_t operator()( key const& p ) const
        {
            std::size_t seed = 0;
            boost::hash_combine( seed, p.first );
            boost::hash_combine( seed, p.second );
            return seed;
        }
    };

    someclass( const key& k ) {}

    void dummy( int x, int y ) const {}
};

TEST( cached, key )
{
    comma::cached< someclass, someclass::key, someclass::hash > c;
    c.get( 1, 2 ).dummy( 1, 2 );
    EXPECT_EQ( c.values().size(), 1 );
    c.get( 1, 2 ).dummy( 1, 2 );
    EXPECT_EQ( c.values().size(), 1 );
    c.get( 3, 1 ).dummy( 3, 1 );
    EXPECT_EQ( c.values().size(), 2 );
    c.get( 3, 1 ).dummy( 3, 1 );
    EXPECT_EQ( c.values().size(), 2 );
}

struct plan
{
    struct params
    {
        int size{0};
        bool real{false};
        bool inverse{false};

        bool operator==( const params& rhs ) const { return size == rhs.size && real == rhs.real && inverse == rhs.inverse; }

        params( const std::vector< int >& v, bool real, bool inverse ): size( v.size() ), real( real ), inverse( inverse ) {}

        params( const std::set< int >& v, bool x ): size( v.size() ), real( x ), inverse( x ) {}
    };

    plan( const params& ) {}

    void operator()( const std::vector< int >&, bool, bool ) {}

    void operator()( const std::set< int >&, bool ) {}

    void size() {}
};

namespace std {

template <> struct hash< plan::params >
{
    std::size_t operator()( plan::params const& k ) const
    {
        std::size_t seed = 0;
        boost::hash_combine( seed, k.size );
        boost::hash_combine( seed, k.real );
        boost::hash_combine( seed, k.inverse );
        return seed;
    }
};

} // namespace std {

TEST( cached, hashing_non_intrusive )
{
    comma::cached< plan, plan::params > c;
    c.get( std::vector{ 1, 2, 3 }, true, false )( std::vector{ 1, 2, 3 }, true, false );
    EXPECT_EQ( c.values().size(), 1 );
    c.get( std::vector{ 1, 2, 3 }, true, false )( std::vector{ 1, 2, 3 }, true, false );
    EXPECT_EQ( c.values().size(), 1 );
    c.get( std::vector{ 1, 2 }, true, false )( std::vector{ 1, 2 }, true, false );
    EXPECT_EQ( c.values().size(), 2 );
    c.get( std::vector{ 1, 2 }, true, false )( std::vector{ 1, 2 }, true, false );
    c.get( std::vector{ 1, 2 }, true, false ).size();
    EXPECT_EQ( c.values().size(), 2 );
}

TEST( cached, operators )
{
    comma::cached< plan, plan::params > plans;
    plans( std::vector{ 1, 2, 3 }, true, false );
    EXPECT_EQ( plans.values().size(), 1 );
    plans( std::vector{ 1, 2, 3 }, true, false );
    EXPECT_EQ( plans.values().size(), 1 );
    plans( std::vector{ 1, 2 }, true, false );
    EXPECT_EQ( plans.values().size(), 2 );
    plans( std::vector{ 1, 2 }, true, false );
    EXPECT_EQ( plans.values().size(), 2 );

    plans( std::set{ 1, 2 }, true );
    EXPECT_EQ( plans.values().size(), 3 );
    plans( std::set{ 1, 2 }, true );
    EXPECT_EQ( plans.values().size(), 3 );
    plans( std::vector{ 1, 2 }, true, true );
    EXPECT_EQ( plans.values().size(), 3 );
}

