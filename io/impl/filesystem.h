// Copyright (c) 2024 Mission Systems Pty Ltd
//
// Allow selection of filesystem library from either boost or C++ standard library
//
// Can assist with interoperation with other libraries that require or conflict
// with one or the other
//
// Usage:
// #include "io/impl/filesystem.h"
// ...
// comma::filesystem::<some-op>
//
// Configuration:
//   in CMake set comma_USE_BOOST_FILESYSTEM
//     "ON" will use boost::filesystem
//     "OFF" will use std::filesystem or std::experimental::filesystem
//           if you're using gcc older than version 8.1

#pragma once

#ifdef COMMA_USE_BOOST_FILESYSTEM
  #include <boost/filesystem.hpp>
  namespace comma { namespace filesystem = boost::filesystem; }
#else
  #if defined(__GNUC__)
    #if __has_include (<filesystem>)
      #include <filesystem>
      namespace comma { namespace filesystem = std::filesystem; }
    #else
      #include <experimental/filesystem>
      namespace comma { namespace filesystem = std::experimental::filesystem; }
    #endif
  #else
    #include <filesystem>
    namespace comma { namespace filesystem = std::filesystem; }
  #endif
#endif
