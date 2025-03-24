
// Copyright (c) 2023 aspen eyers

#include <gtest/gtest.h>
#include "../shared_library.h"
#include "../../base/exception.h"

#include "shared_library_test_detail.h"
#include <dlfcn.h>

namespace comma {

TEST( initialise, no_args )
{
    comma::dynamic::shared_library library( "libcomma_dynamic_test_classes.so", {"/home/aspen/src/comma/build/lib"} );
    auto p = library.make< comma::dynamic::test::simple >( "comma_dynamic_test_create_simple" );
    ( void )p;
}


TEST( initialise, args )
{
    comma::dynamic::test::point* p;
    comma::dynamic::shared_library library( "libcomma_dynamic_test_classes.so", {"/home/aspen/src/comma/build/lib"} );
    p = library.make<comma::dynamic::test::point, float, float, float>( "comma_dynamic_test_create_point", 1.0, 2.0, 3.0 );
    EXPECT_EQ( p->x, 1.0 );
    EXPECT_EQ( p->y, 2.0 );
    EXPECT_EQ( p->z, 3.0 );
}

TEST( initialise, polymorphic )
{
    comma::dynamic::shared_library library( "libcomma_dynamic_test_classes.so", {"/home/aspen/src/comma/build/lib"} );
    auto p = library.make< comma::dynamic::test::polymorphic_point, float, float, float>( "comma_dynamic_test_create_polymorphic_point", 1.0, 2.0, 3.0 );
    EXPECT_EQ( p->get_x(), 1.0 );
    EXPECT_EQ( p->get_y(), 2.0 );
    EXPECT_EQ( p->get_z(), 3.0 );
}


TEST( initialise, vector )
{
    std::vector<comma::dynamic::shared_library*> libraries;
    libraries.emplace_back( new comma::dynamic::shared_library( "libcomma_dynamic_test_classes.so", {"/home/aspen/src/comma/build/lib"} ) );
    auto p = libraries.back()->make< comma::dynamic::test::polymorphic_point, float, float, float>( "comma_dynamic_test_create_polymorphic_point", 1.0, 2.0, 3.0 );
    EXPECT_EQ( p->get_x(), 1.0 );
    EXPECT_EQ( p->get_y(), 2.0 );
    EXPECT_EQ( p->get_z(), 3.0 );

}

TEST( initialise, failure_case )
{
    comma::dynamic::shared_library library( "libcomma_dynamic_test_classes.so", {"/home/aspen/src/comma/build/lib"} );
    EXPECT_THROW({library.make< comma::dynamic::test::simple >( "non_existant_symbol" ); }, comma::exception);
}



} // namespace comma {

int main( int argc, char* argv[] )
{    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
