// Copyright (c) 2023 Mission Systems Pty Ltd

/// @authors aspen eyers, vsevolod vlaskine

#pragma once 
#include <dlfcn.h>
#include <vector>
#include <memory>
#include "../base/exception.h"

namespace comma { namespace dynamic {

class shared_library
{
    public:
        shared_library( const std::string& lib, std::vector<std::string> link_directories={} );

        ~shared_library() { dlclose(handle_); }

        template < typename T, typename... Args >
        T* make( const std::string& library_symbol, Args... args ) const;

    private:
        void* handle_;
        std::string lib_;
};


inline shared_library::shared_library( const std::string& lib, std::vector<std::string> link_directories )
: lib_(lib)
{
    char* error;
    // sanitise_search_directories_
    for (auto& dir : link_directories) { if( dir.back() != '/' ) { dir += std::string("/"); } }
    for (const auto& dir : link_directories) {
        std::string link_lib = (dir+lib).c_str();
        handle_ = dlopen( &link_lib[0], RTLD_LAZY);
        error = dlerror();
        if( error == NULL ){ break; } 
    }
    if( error != NULL ) handle_ = dlopen(&lib[0], RTLD_LAZY);
    COMMA_ASSERT( error == NULL, error );
}

template < typename T, typename... Args >
inline T* shared_library::make( const std::string& library_symbol, Args... args ) const
{
    char* error;
    auto symbol = dlsym(handle_, &library_symbol[0]);
    COMMA_ASSERT( error == NULL, error );
    if( symbol == nullptr ) { std::cerr << dlerror() << std::endl; }
    COMMA_ASSERT( symbol, "Shared library loading failed: could not find "+library_symbol+" symbol; on library: \"" + lib_ + "\"");
    T* (*create_)(Args...) = reinterpret_cast<T*(*)(Args...)>(symbol);
    COMMA_ASSERT( create_, "Shared library loading failed: could not find "+library_symbol+" symbol; on library: \"" + lib_ + "\"");

    T* object = create_(args...);
    return object;
}

}}; // namespace comma { namespace dynamic {
