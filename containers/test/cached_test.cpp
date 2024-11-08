// Copyright (c) 2024 Vsevolod Vlaskine
// All Rights Reserved

#include <gtest/gtest.h>
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
    EXPECT_EQ( c( 5 ).calculate( 5 ), 25 );
    EXPECT_EQ( c.values().size(), 1 );
    EXPECT_EQ( c( 5 ).calculate( 5 ), 25 );
    EXPECT_EQ( c.values().size(), 1 );
    EXPECT_EQ( c( 10 ).calculate( 10 ), 100 );
    EXPECT_EQ( c.values().size(), 2 );
    EXPECT_EQ( c( 10 ).calculate( 10 ), 100 );
    EXPECT_EQ( c.values().size(), 2 );
    EXPECT_EQ( c( 20 ).calculate( 20 ), 400 );
    EXPECT_EQ( c.values().size(), 3 );
    EXPECT_EQ( c( 20 ).calculate( 20 ), 400 );
    EXPECT_EQ( c.values().size(), 3 );
    c.pop();
    EXPECT_EQ( c.values().size(), 2 );
    EXPECT_EQ( c.values().size(), 2 );
    EXPECT_EQ( c( 20 ).calculate( 20 ), 400 );
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

// struct pair_hash : public std::unary_function< Array, std::size_t >
// {
//     std::size_t operator()( Array const& array ) const
//     {
//         std::size_t seed = 0;
//         for( std::size_t i = 0; i < Size; ++i ) { boost::hash_combine( seed, array[i] ); }
//         return seed;
//         // return boost::hash_range( &array[0], &array[Size] ); // not so easy...
//     }
// };

TEST( cached, key )
{
    comma::cached< someclass, someclass::key, someclass::hash > c;
    c( 1, 2 ).dummy( 1, 2 );
    EXPECT_EQ( c.values().size(), 1 );
    c( 1, 2 ).dummy( 1, 2 );
    EXPECT_EQ( c.values().size(), 1 );
    c( 3, 1 ).dummy( 3, 1 );
    EXPECT_EQ( c.values().size(), 2 );
    c( 3, 1 ).dummy( 3, 1 );
    EXPECT_EQ( c.values().size(), 2 );
}