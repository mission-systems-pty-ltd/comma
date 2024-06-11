// Copyright (c) 2023 Mission Systems Pty Ltd

#include <gtest/gtest.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "../ordered/queues.h"

TEST( queues, usage )
{
    typedef comma::containers::ordered::queues< int, int, int > queues_t;
    queues_t q{ 2 /*timeout*/  };

    EXPECT_EQ( std::get<0>(q).size(), 0 );
    EXPECT_EQ( std::get<1>(q).size(), 0 );
    EXPECT_EQ( q.ready(), false );

    std::get<0>(q).push( std::make_pair( 0, 1 ) );
    EXPECT_EQ( std::get<0>(q).front().second, 1 );
    EXPECT_EQ( std::get<0>(q).size(), 1 );
    EXPECT_EQ( std::get<1>(q).size(), 0 );
    EXPECT_EQ( q.ready(), false );

    std::get<1>(q).push( std::make_pair( 0, 1 ) );
    EXPECT_EQ( std::get<1>(q).front().second, 1 );
    EXPECT_EQ( std::get<0>(q).size(), 1 );
    EXPECT_EQ( std::get<1>(q).size(), 1 );
    EXPECT_EQ( q.ready(), true );

    // Purge should only remove items if they are unsynced
    q.purge();
    EXPECT_EQ( std::get<0>(q).front().second, 1 );
    EXPECT_EQ( std::get<1>(q).front().second, 1 );
    EXPECT_EQ( std::get<0>(q).size(), 1 );
    EXPECT_EQ( std::get<1>(q).size(), 1 );
    EXPECT_EQ( q.ready(), true );
}

TEST( queues, sync_first_to_second ){
    typedef comma::containers::ordered::queues< float, int, int > queues_t;
    queues_t q{ 2 /*timeout*/  };

    std::get<0>(q) = std::queue< std::pair< float, int > >({ {0, 0}, {2, 0}, {4, 5} });
    std::get<1>(q) = std::queue< std::pair< float, int > >({ {5, 5} });
    EXPECT_EQ( q.ready(), false );
    q.purge();
    EXPECT_EQ( std::get<0>(q).front().second, 5 );
    EXPECT_EQ( std::get<1>(q).front().second, 5 );
    EXPECT_EQ( std::get<0>(q).front().first, 4 );
    EXPECT_EQ( std::get<1>(q).front().first, 5 );    
    EXPECT_EQ( std::get<0>(q).size(), 1 );
    EXPECT_EQ( std::get<1>(q).size(), 1 );
    EXPECT_EQ( q.ready(), true );

}

TEST( queues, sync_second_to_first ){
    typedef comma::containers::ordered::queues< float, int, int > queues_t;
    queues_t q{ 2 /*timeout*/  };

    std::get<0>(q) = std::queue< std::pair< float, int > >({ {5, 5} });
    std::get<1>(q) = std::queue< std::pair< float, int > >({ {1, 0}, {2, 0}, {4, 5} });
    EXPECT_EQ( q.ready(), false );
    q.purge();
    EXPECT_EQ( std::get<0>(q).front().second, 5 );
    EXPECT_EQ( std::get<1>(q).front().second, 5 );
    EXPECT_EQ( std::get<0>(q).front().first, 5 );
    EXPECT_EQ( std::get<1>(q).front().first, 4 );    
    EXPECT_EQ( std::get<0>(q).size(), 1 );
    EXPECT_EQ( std::get<1>(q).size(), 1 );
    EXPECT_EQ( q.ready(), true );

}

TEST( queues, empty_list_before_sync ){
    typedef comma::containers::ordered::queues< float, int, int > queues_t;
    queues_t q{ 2 /*timeout*/ };
    std::get<0>(q) = std::queue< std::pair< float, int > >({ {0, 0}, {1, 0}, {2, 5} });
    std::get<1>(q) = std::queue< std::pair< float, int > >({ {5, 5} });
    q.purge();
    EXPECT_EQ( std::get<0>(q).size(), 0 );
    EXPECT_EQ( std::get<1>(q).size(), 1 );
    EXPECT_EQ( q.ready(), false );
}

TEST( queues, max_time_offset ){
    typedef comma::containers::ordered::queues< float, int, int > queues_t;
    queues_t q{ 2 /*timeout*/ };
    std::get<0>(q) = std::queue< std::pair< float, int > >({ {3, 5} });
    std::get<1>(q) = std::queue< std::pair< float, int > >({ {5, 5} });
    q.purge();
    EXPECT_EQ( std::get<0>(q).size(), 1 );
    EXPECT_EQ( std::get<1>(q).size(), 1 );
    EXPECT_EQ( std::get<0>(q).front().second, 5 );
    EXPECT_EQ( std::get<1>(q).front().second, 5 );
    EXPECT_EQ( std::get<0>(q).front().first, 3 );
    EXPECT_EQ( std::get<1>(q).front().first, 5 );        
    EXPECT_EQ( q.ready(), true );
}

TEST( queues, floating_point_error ){
    typedef comma::containers::ordered::queues< float, int, int > queues_t;
    {
    queues_t q{ 2 /*timeout*/ };
    std::get<0>(q) = std::queue< std::pair< float, int > >({ {3.000001, 5} });
    std::get<1>(q) = std::queue< std::pair< float, int > >({ {5, 5} });
    q.purge();
    EXPECT_EQ( std::get<0>(q).size(), 1 );
    EXPECT_EQ( std::get<1>(q).size(), 1 );
    EXPECT_EQ( std::get<0>(q).front().second, 5 );
    EXPECT_EQ( std::get<1>(q).front().second, 5 );
    EXPECT_NEAR( std::get<0>(q).front().first, 3.000001, 1e-6 );
    EXPECT_EQ( std::get<1>(q).front().first, 5 );        
    EXPECT_EQ( q.ready(), true );
    }
    {
    queues_t q{ 2 /*timeout*/ };
    std::get<0>(q) = std::queue< std::pair< float, int > >({ {2.999999, 5} });
    std::get<1>(q) = std::queue< std::pair< float, int > >({ {5, 5} });
    q.purge();
    EXPECT_EQ( std::get<0>(q).size(), 0 );
    EXPECT_EQ( std::get<1>(q).size(), 1 );
    EXPECT_EQ( std::get<1>(q).front().second, 5 );
    EXPECT_EQ( std::get<1>(q).front().first, 5 );        
    EXPECT_EQ( q.ready(), false );
    }
    {
    queues_t q{ 2 /*timeout*/ };
    std::get<0>(q) = std::queue< std::pair< float, int > >({ {5, 5} });
    std::get<1>(q) = std::queue< std::pair< float, int > >({ {3.000001, 5} });
    q.purge();
    EXPECT_EQ( std::get<0>(q).size(), 1 );
    EXPECT_EQ( std::get<1>(q).size(), 1 );
    EXPECT_EQ( std::get<0>(q).front().second, 5 );
    EXPECT_EQ( std::get<1>(q).front().second, 5 );
    EXPECT_EQ( std::get<0>(q).front().first, 5 );
    EXPECT_NEAR( std::get<1>(q).front().first, 3.000001, 1e-6 );
    EXPECT_EQ( q.ready(), true );
    }
    {
    queues_t q{ 2 /*timeout*/ };
    std::get<0>(q) = std::queue< std::pair< float, int > >({ {5, 5} });
    std::get<1>(q) = std::queue< std::pair< float, int > >({ {2.999999, 5} });
    q.purge();
    EXPECT_EQ( std::get<0>(q).size(), 1 );
    EXPECT_EQ( std::get<1>(q).size(), 0 );
    EXPECT_EQ( std::get<0>(q).front().second, 5 );
    EXPECT_EQ( std::get<0>(q).front().first, 5 );
    EXPECT_EQ( q.ready(), false );
    }    
}

TEST( queues, type_difference )
{
    typedef comma::containers::ordered::queues< float, int, double > queues_t;
    queues_t q{ 2 /*timeout*/ };
    std::get<0>(q) = std::queue< std::pair< float, int > >({ {0, 1} });
    std::get<1>(q) = std::queue< std::pair< float, double > >({ {0, 1.0} });
}

TEST( queues, boost_time )
{
    typedef comma::containers::ordered::queues< boost::posix_time::ptime, double, double > queues_t;
    queues_t q{boost::posix_time::seconds( 2 /*timeout*/  ) };

    boost::posix_time::ptime t( boost::gregorian::date( 2023, 1, 1 ) );
    std::get<0>(q) = std::queue< std::pair< boost::posix_time::ptime, double > >({ {t, 1.0} });
    std::get<1>(q) = std::queue< std::pair< boost::posix_time::ptime, double > >({ {t, 1.0} });
    EXPECT_EQ( std::get<0>(q).size(), 1 );
    EXPECT_EQ( std::get<1>(q).size(), 1 );
    EXPECT_EQ( q.ready(), true );
}
