#pragma once

#include "definitions.h"

#ifdef _WIN32
#define DLL_EXPORT __declspec( dllexport )
#else
#define DLL_EXPORT
#endif

extern "C" {

DLL_EXPORT void comma_free( void* p );

}
