// Copyright (c) 2024 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

namespace comma { namespace visiting {

/// convenience class when std::optional or boost::optional is not enough
/// e.g. if in visiting we would like to have an explicit is-set flag field 
/// in csv, json, or alike, where it may be essential in fixed-width data
/// (e.g. csv) where the optional value may or may not be present
template < typename T >
struct optional
{
    T value;
    bool is_set{false};

    optional() = default;
    optional( const T& t ): value( t ), is_set( true ) {}
    template < class... Args > optional( Args... args ): value( args... ), is_set( true ) {}
    template < class... Args > void emplace( Args... args ); // todo
    optional& operator=( const T& rhs ) { value = rhs; is_set = true; return *this; }
    void reset() { is_set = false; }
    operator bool() const { return is_set; }
    T* operator->() { return &value; }
    const T* operator->() const { return &value; }
    T& operator*() { return value; }
    const T& operator*() const { return value; }
};

} } // namespace comma { namespace visiting {
