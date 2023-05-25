// Copyright (c) 2023 Vsevolod Vlaskine

#include <gtest/gtest.h>
#include "../multikey_map.h"

TEST( multikey_map, usage )
{
    comma::multikey_map< double, int, 3 > m( { 0, 0, 0 }, { 1, 2, 3 } );
    m.touch_at( { 1, 2, 3 } );
    // todo
}

TEST( multikey_map, index )
{
    typedef comma::multikey_map< double, int, 3 > map_type;
    {
        map_type m( {1, 1, 1} );
        {
            map_type::index_type i = {{ 0, 0, 0 }};
            EXPECT_EQ( i, m.index_of( {0., 0., 0.} ) );
            EXPECT_EQ( i, m.index_of( {0.001, 0.001, 0.001} ) );
            EXPECT_EQ( i, m.index_of( {0.999, 0.999, 0.999} ) );
        }
        {
            map_type::index_type i = {{ 1, 1, 1 }};
            EXPECT_EQ( i, m.index_of( {1.0, 1.0, 1.0} ) );
            EXPECT_EQ( i, m.index_of( {1.001, 1.001, 1.001} ) );
            EXPECT_EQ( i, m.index_of( {1.999, 1.999, 1.999} ) );
        }
        {
            map_type::index_type i = {{ -1, -1, -1 }};
            EXPECT_EQ( i, m.index_of( {-1.0, -1.0, -1.0} ) );
            EXPECT_EQ( i, m.index_of( {-0.999, -0.999, -0.999} ) );
            EXPECT_EQ( i, m.index_of( {-0.001, -0.001, -0.001} ) );
        }
    }
    {
        map_type m( {0.3, 0.3, 0.3} );
        {
            map_type::index_type i = {{ 0, 0, 0 }};
            EXPECT_EQ( i, m.index_of( {0, 0, 0} ) );
            EXPECT_EQ( i, m.index_of( {0.001, 0.001, 0.001} ) );
            EXPECT_EQ( i, m.index_of( {0.299, 0.299, 0.299} ) );
        }        
        {
            map_type::index_type i = {{ 1, 1, 1 }};
            EXPECT_EQ( i, m.index_of( {0.3, 0.3, 0.3} ) );
            EXPECT_EQ( i, m.index_of( {0.3001, 0.3001, 0.3001} ) );
            EXPECT_EQ( i, m.index_of( {0.3999, 0.3999, 0.3999} ) );
        }
        {
            map_type::index_type i = {{ -1, -1, -1 }};
            EXPECT_EQ( i, m.index_of( {-0.3, -0.3, -0.3} ) );
            EXPECT_EQ( i, m.index_of( {-0.299, -0.299, -0.299} ) );
            EXPECT_EQ( i, m.index_of( {-0.001, -0.001, -0.001} ) );
        }        
    }
}

TEST( multikey_map, operations )
{
    typedef comma::multikey_map< double, int, 3 > map_type;
    map_type m( {1, 1, 1} );
    {
        EXPECT_TRUE( ( m.find( map_type::point_type{1., 1., 1.} ) == m.end() ) );
        EXPECT_TRUE( ( m.touch_at( map_type::point_type{1., 1., 1.} ) != m.end() ) );
        EXPECT_EQ( 1, m.size() );
        EXPECT_TRUE( ( m.find( map_type::point_type{1., 1., 1.} ) != m.end() ) );
        EXPECT_TRUE( ( m.find( map_type::point_type{1., 1., 1.} ) == m.find( map_type::point_type{1.1, 1.1, 1.1} ) ) );
        EXPECT_TRUE( ( m.touch_at( {1, 1, 1} ) != m.end() ) );
        EXPECT_EQ( 1, m.size() );
        EXPECT_TRUE( ( m.touch_at( {1.1, 1.1, 1.1} ) != m.end() ) );
        EXPECT_EQ( 1, m.size() );
    }
    {
        EXPECT_TRUE( ( m.find( map_type::point_type{-1., -1., -1.} ) == m.end() ) );
        EXPECT_TRUE( ( m.touch_at( {-1., -1., -1.} ) != m.end() ) );
        EXPECT_EQ( 2, m.size() );
        EXPECT_TRUE( ( m.find( map_type::point_type{-1., -1., -1.} ) != m.end() ) );
        EXPECT_TRUE( ( m.find( map_type::point_type{-1., -1., -1.} ) == m.find( map_type::point_type{-0.1, -0.1, -0.1} ) ) );
        EXPECT_TRUE( ( m.touch_at( {-1., -1., -1.} ) != m.end() ) );
        EXPECT_EQ( 2, m.size() );
        EXPECT_TRUE( ( m.touch_at( {-0.1, -0.1, -0.1} ) != m.end() ) );
        EXPECT_EQ( 2, m.size() );
    }
    {
        EXPECT_TRUE( ( m.find( map_type::point_type{0., 0., 0.} ) == m.end() ) );
        EXPECT_TRUE( ( m.touch_at( {0., 0., 0.} ) != m.end() ) );
        EXPECT_EQ( 3, m.size() );
        EXPECT_TRUE( ( m.find( map_type::point_type{0., 0, 0} ) != m.end() ) );
        EXPECT_TRUE( ( m.find( map_type::point_type{0., 0, 0} ) == m.find( map_type::point_type{0.1, 0.1, 0.1} ) ) );
        EXPECT_TRUE( ( m.touch_at( {0., 0, 0} ) != m.end() ) );
        EXPECT_EQ( 3, m.size() );
        EXPECT_TRUE( ( m.touch_at( {0.1, 0.1, 0.1} ) != m.end() ) );
        EXPECT_EQ( 3, m.size() );
    }
}

TEST( multikey_map, test )
{
    typedef comma::multikey_map< double, int, 3 > map_type;
    map_type m( {1, 1, 1} );
    EXPECT_TRUE( m.empty() );
}

TEST( multikey_map, neighbourhood )
{
    typedef comma::multikey_map< double, int, 3 > map_type;
    map_type m( {1, 1, 1} );
    {
        EXPECT_TRUE( ( m.find( map_type::point_type{1, 1, 1} ) == m.end() ) );
        {
            EXPECT_TRUE( ( m.touch_at( {1, 1, 1} ) != m.end() ) );
            EXPECT_EQ( 1, m.size() );
            m.touch_at( {1, 1, 1} )->second = 111;
            EXPECT_EQ( 111, m.find( map_type::point_type{1, 1, 1} )->second );
            map_type::index_type index = {{ 1, 1, 1 }};
            EXPECT_EQ( 111, m.base_type::find( index )->second );
        }
        {
            EXPECT_TRUE( ( m.touch_at( {2, 2, 2} ) != m.end() ) );
            EXPECT_EQ( 2, m.size() );
            m.touch_at( {2, 2, 2} )->second = 222;
            EXPECT_EQ( 222, m.find( map_type::point_type{2, 2, 2} )->second );
            map_type::index_type index = {{ 2, 2, 2 }};
            EXPECT_EQ( 222, m.base_type::find( index )->second );
        }
        {
            map_type::index_type index = {{ -1, 0, 0 }};
            EXPECT_TRUE( m.base_type::find( index ) == m.end() );
        }
        {
            map_type::index_type index = {{ 0, 0, 0 }};
            EXPECT_TRUE( m.base_type::find( index ) == m.end() );
        }
        {
            map_type::index_type index = {{ 2, 2, 3 }};
            EXPECT_TRUE( m.base_type::find( index ) == m.end() );
        }
    }
}
