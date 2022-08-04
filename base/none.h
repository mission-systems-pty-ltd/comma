// Copyright (c) 2022 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

namespace comma {

/// convenience type to use e.g. as a "tag" type in template definitions
/// since boost::none_t is not default-constructible as it is designed
/// to be a singleton type (see boost/none_t.hpp for details) meaning
/// that it won't compile for some use cases
struct none {};
    
} // namespace comma {
