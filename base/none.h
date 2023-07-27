// Copyright (c) 2022 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <boost/optional.hpp>

namespace comma {

/// convenience type to use e.g. as a "tag" type in template definitions
/// since boost::none_t is not default-constructible as it is designed
/// to be a singleton type (see boost/none_t.hpp for details) meaning
/// that it won't compile for some use cases
struct none {};

/// a quick fix for annoying boost::optional compilation warning
/// boost::optional< int > i; // gets compile warning when i first dereferenced
///                           // even if i is initialized for sure somewhere in the code
/// boost::optional< int > i{boost::none}; // still same compile warning
/// boost::optional< int > i = comma::silent_none< int >(); // fine, no warning
template < typename T >
inline boost::optional< T > silent_none() { return boost::optional< T >( boost::none ); }
    
} // namespace comma {
