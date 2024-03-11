// Copyright (c) 2024 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#include "terminal.h"

namespace comma { namespace io { namespace terminal { namespace controls {

char end::value[] = { 0x07, 0 };

char stderr::start[] = { 0 };

char stderr::end[] = { '\n', 0 }; // quick and dirty

char titlebar::start[] = { 0x1b, ']', '0', ';', 0 };

char titlebar::end[] = { 0x07, 0 };

} } } } // namespace comma { namespace io { namespace terminal { namespace controls {
