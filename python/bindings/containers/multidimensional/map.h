#pragma once

#include "../../definitions.h"
#include "../../memory.h"

extern "C" {

enum types { _comma_int32 = 0, _comma_int64 = 1, _comma_float32 = 2, _comma_float64 = 3 };

DLL_EXPORT void* comma_containers_multidimensional_map_create( int key_type, unsigned int dim, const void* origin, const void* resolution, const void* values, unsigned int size );

DLL_EXPORT void comma_containers_multidimensional_map_destroy( void* p );

DLL_EXPORT unsigned int comma_containers_multidimensional_map_size( const void* p );

DLL_EXPORT unsigned int comma_containers_multidimensional_map_count( const void* p );

DLL_EXPORT const void* comma_containers_multidimensional_map_at( const void* p, const void* k, void* size );

DLL_EXPORT const void* comma_containers_multidimensional_map_nearest( const void* p, const void* k, unsigned int n );

}