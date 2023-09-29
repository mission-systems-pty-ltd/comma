#pragma once 
#include "shared_library_test_detail.h"
#define DYNAMICLIB_API

extern "C" DYNAMICLIB_API comma::dynamic::test::simple* comma_dynamic_test_create_simple(){ return new comma::dynamic::test::simple(); }
extern "C" DYNAMICLIB_API comma::dynamic::test::point* comma_dynamic_test_create_point(float x, float y, float z){ return new comma::dynamic::test::point(x, y, z); }
extern "C" DYNAMICLIB_API comma::dynamic::test::polymorphic_point* comma_dynamic_test_create_polymorphic_point(float x, float y, float z){ return new comma::dynamic::test::polymorphic_point(x, y, z); }
