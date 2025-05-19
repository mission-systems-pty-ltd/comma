#pragma once

#include <stdlib.h>
#include "memory.h"

DLL_EXPORT void comma_free( void* p ) { ::free( p ); }
