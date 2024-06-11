// Copyright (c) 2023 Mission Systems Pty Ltd

#include <gtest/gtest.h>
#include "../ordered/multiqueue.h"

TEST( multiqueue, usage )
{
    typedef comma::containers::ordered::multiqueue< int, int, int > multiqueue_t;
    multiqueue_t q{ 2 /*timeout*/  };

    EXPECT_EQ( std::get<0>(q.queues).size(), 0 );
    EXPECT_EQ( std::get<1>(q.queues).size(), 0 );
    EXPECT_EQ( q.ready(), false );

    std::get<0>(q.queues).push( std::make_pair( 0, 1 ) );
    EXPECT_EQ( std::get<0>(q.queues).front().second, 1 );
    EXPECT_EQ( std::get<0>(q.queues).size(), 1 );
    EXPECT_EQ( std::get<1>(q.queues).size(), 0 );
    EXPECT_EQ( q.ready(), false );

    std::get<1>(q.queues).push( std::make_pair( 0, 1 ) );
    EXPECT_EQ( std::get<1>(q.queues).front().second, 1 );
    EXPECT_EQ( std::get<0>(q.queues).size(), 1 );
    EXPECT_EQ( std::get<1>(q.queues).size(), 1 );
    EXPECT_EQ( q.ready(), true );

    // Purge should only remove items if they are unsynced
    q.purge();
    EXPECT_EQ( std::get<0>(q.queues).front().second, 1 );
    EXPECT_EQ( std::get<1>(q.queues).front().second, 1 );
    EXPECT_EQ( std::get<0>(q.queues).size(), 1 );
    EXPECT_EQ( std::get<1>(q.queues).size(), 1 );
    EXPECT_EQ( q.ready(), true );
}

TEST( multiqueue, sync_first_to_second ){
    typedef comma::containers::ordered::multiqueue< float, int, int > multiqueue_t;
    multiqueue_t q{ 2 /*timeout*/  };

    std::get<0>(q.queues) = std::queue< std::pair< float, int > >({ {0, 0}, {2, 0}, {4, 5} });
    std::get<1>(q.queues) = std::queue< std::pair< float, int > >({ {5, 5} });
    EXPECT_EQ( q.ready(), false );
    q.purge();
    EXPECT_EQ( std::get<0>(q.queues).front().second, 5 );
    EXPECT_EQ( std::get<1>(q.queues).front().second, 5 );
    EXPECT_EQ( std::get<0>(q.queues).front().first, 4 );
    EXPECT_EQ( std::get<1>(q.queues).front().first, 5 );    
    EXPECT_EQ( std::get<0>(q.queues).size(), 1 );
    EXPECT_EQ( std::get<1>(q.queues).size(), 1 );
    EXPECT_EQ( q.ready(), true );

}

TEST( multiqueue, sync_second_to_first ){
    typedef comma::containers::ordered::multiqueue< float, int, int > multiqueue_t;
    multiqueue_t q{ 2 /*timeout*/  };

    std::get<0>(q.queues) = std::queue< std::pair< float, int > >({ {5, 5} });
    std::get<1>(q.queues) = std::queue< std::pair< float, int > >({ {1, 0}, {2, 0}, {4, 5} });
    EXPECT_EQ( q.ready(), false );
    q.purge();
    EXPECT_EQ( std::get<0>(q.queues).front().second, 5 );
    EXPECT_EQ( std::get<1>(q.queues).front().second, 5 );
    EXPECT_EQ( std::get<0>(q.queues).front().first, 5 );
    EXPECT_EQ( std::get<1>(q.queues).front().first, 4 );    
    EXPECT_EQ( std::get<0>(q.queues).size(), 1 );
    EXPECT_EQ( std::get<1>(q.queues).size(), 1 );
    EXPECT_EQ( q.ready(), true );

}

TEST( multiqueue, empty_list_before_sync ){
    typedef comma::containers::ordered::multiqueue< float, int, int > multiqueue_t;
    multiqueue_t q{ 2 /*timeout*/ };
    std::get<0>(q.queues) = std::queue< std::pair< float, int > >({ {0, 0}, {1, 0}, {2, 5} });
    std::get<1>(q.queues) = std::queue< std::pair< float, int > >({ {5, 5} });
    q.purge();
    EXPECT_EQ( std::get<0>(q.queues).size(), 0 );
    EXPECT_EQ( std::get<1>(q.queues).size(), 1 );
    EXPECT_EQ( q.ready(), false );
}

TEST( multiqueue, max_time_offset ){
    typedef comma::containers::ordered::multiqueue< float, int, int > multiqueue_t;
    multiqueue_t q{ 2 /*timeout*/ };
    std::get<0>(q.queues) = std::queue< std::pair< float, int > >({ {3, 5} });
    std::get<1>(q.queues) = std::queue< std::pair< float, int > >({ {5, 5} });
    q.purge();
    EXPECT_EQ( std::get<0>(q.queues).size(), 1 );
    EXPECT_EQ( std::get<1>(q.queues).size(), 1 );
    EXPECT_EQ( std::get<0>(q.queues).front().second, 5 );
    EXPECT_EQ( std::get<1>(q.queues).front().second, 5 );
    EXPECT_EQ( std::get<0>(q.queues).front().first, 3 );
    EXPECT_EQ( std::get<1>(q.queues).front().first, 5 );        
    EXPECT_EQ( q.ready(), true );
}

TEST( multiqueue, floating_point_error ){
    typedef comma::containers::ordered::multiqueue< float, int, int > multiqueue_t;
    {
    multiqueue_t q{ 2 /*timeout*/ };
    std::get<0>(q.queues) = std::queue< std::pair< float, int > >({ {3.000001, 5} });
    std::get<1>(q.queues) = std::queue< std::pair< float, int > >({ {5, 5} });
    q.purge();
    EXPECT_EQ( std::get<0>(q.queues).size(), 1 );
    EXPECT_EQ( std::get<1>(q.queues).size(), 1 );
    EXPECT_EQ( std::get<0>(q.queues).front().second, 5 );
    EXPECT_EQ( std::get<1>(q.queues).front().second, 5 );
    EXPECT_NEAR( std::get<0>(q.queues).front().first, 3.000001, 1e-6 );
    EXPECT_EQ( std::get<1>(q.queues).front().first, 5 );        
    EXPECT_EQ( q.ready(), true );
    }
    {
    multiqueue_t q{ 2 /*timeout*/ };
    std::get<0>(q.queues) = std::queue< std::pair< float, int > >({ {2.999999, 5} });
    std::get<1>(q.queues) = std::queue< std::pair< float, int > >({ {5, 5} });
    q.purge();
    EXPECT_EQ( std::get<0>(q.queues).size(), 0 );
    EXPECT_EQ( std::get<1>(q.queues).size(), 1 );
    EXPECT_EQ( std::get<1>(q.queues).front().second, 5 );
    EXPECT_EQ( std::get<1>(q.queues).front().first, 5 );        
    EXPECT_EQ( q.ready(), false );
    }
    {
    multiqueue_t q{ 2 /*timeout*/ };
    std::get<0>(q.queues) = std::queue< std::pair< float, int > >({ {5, 5} });
    std::get<1>(q.queues) = std::queue< std::pair< float, int > >({ {3.000001, 5} });
    q.purge();
    EXPECT_EQ( std::get<0>(q.queues).size(), 1 );
    EXPECT_EQ( std::get<1>(q.queues).size(), 1 );
    EXPECT_EQ( std::get<0>(q.queues).front().second, 5 );
    EXPECT_EQ( std::get<1>(q.queues).front().second, 5 );
    EXPECT_EQ( std::get<0>(q.queues).front().first, 5 );
    EXPECT_NEAR( std::get<1>(q.queues).front().first, 3.000001, 1e-6 );
    EXPECT_EQ( q.ready(), true );
    }
    {
    multiqueue_t q{ 2 /*timeout*/ };
    std::get<0>(q.queues) = std::queue< std::pair< float, int > >({ {5, 5} });
    std::get<1>(q.queues) = std::queue< std::pair< float, int > >({ {2.999999, 5} });
    q.purge();
    EXPECT_EQ( std::get<0>(q.queues).size(), 1 );
    EXPECT_EQ( std::get<1>(q.queues).size(), 0 );
    EXPECT_EQ( std::get<0>(q.queues).front().second, 5 );
    EXPECT_EQ( std::get<0>(q.queues).front().first, 5 );
    EXPECT_EQ( q.ready(), false );
    }    
}

TEST( multiqueue, type_difference ){
    typedef comma::containers::ordered::multiqueue< float, int, double > multiqueue_t;
    multiqueue_t q{ 2 /*timeout*/ };
    std::get<0>(q.queues) = std::queue< std::pair< float, int > >({ {0, 1} });
    std::get<1>(q.queues) = std::queue< std::pair< float, double > >({ {0, 1.0} });
}

TEST( multiqueue, boost_time ){
    typedef comma::containers::ordered::multiqueue< boost::posix_time::ptime, double, double > multiqueue_t;
    multiqueue_t q{boost::posix_time::seconds( 2 /*timeout*/  ) };

    boost::posix_time::ptime t( boost::gregorian::date( 2023, 1, 1 ) );
    std::get<0>(q.queues) = std::queue< std::pair< boost::posix_time::ptime, double > >({ {t, 1.0} });
    std::get<1>(q.queues) = std::queue< std::pair< boost::posix_time::ptime, double > >({ {t, 1.0} });
    EXPECT_EQ( std::get<0>(q.queues).size(), 1 );
    EXPECT_EQ( std::get<1>(q.queues).size(), 1 );
    EXPECT_EQ( q.ready(), true );
}
