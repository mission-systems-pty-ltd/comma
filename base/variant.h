// Copyright (c) 2024 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

// todo
// #if __cplusplus >= 201703L
// #include <optional>
// #else
// #include <boost/optional.hpp>
// #endif
#include <type_traits>
#include <boost/optional.hpp>

namespace comma {

// todo
//   ? use tuple instead?
//   - check that types don't repeat

namespace impl {

template < typename T, bool B > struct variant_traits
{
    template < typename S > static void set( boost::optional< T >& t, const S& ) { t.reset(); }
    template < typename S > static void set( boost::optional< T >& t, const boost::optional< S >& ) { t.reset(); }
};

template < typename T > struct variant_traits< T, true >
{
    template < typename S > static void set( boost::optional< T >& t, const S& s ) { t = s; }
    template < typename S > static void set( boost::optional< T >& t, const boost::optional< S >& s ) { t = s; }
};

template < typename T, typename... Args > struct variant  // todo? use tuple instead?
{
    boost::optional< T > t;
    variant< Args... > values;
 
    template < typename S > bool is() const { return ( std::is_same< T, S >::value && bool( t ) ) || values.template is< S >(); }
    operator bool() const { return bool( t ) || bool( values ); }
    template < typename S > void set( const S& s ) { variant_traits< T, std::is_same< T, S >::value >::set( t, s ); values.set( s ); }
    template < typename S > void set( const boost::optional< S >& s ) { variant_traits< T, std::is_same< T, S >::value >::set( t, s ); values.set( s ); }
};

template < typename T > struct variant< T >  // todo? use tuple instead?
{
    boost::optional< T > t;

    template < typename S > bool is() const { return std::is_same< T, S >::value && bool( t ); }
    operator bool() const { return bool( t ); }
    template < typename S > void set( const S& s ) { variant_traits< T, std::is_same< T, S >::value >::set( t, s ); }
    template < typename S > void set( const boost::optional< S >& s ) { variant_traits< T, std::is_same< T, S >::value >::set( t, s ); }
};

} // namespace impl {

template < typename... Args >
class variant
{
    public:
        // todo
    private:
        impl::variant< Args... > _values;
};

} // namespace comma {
