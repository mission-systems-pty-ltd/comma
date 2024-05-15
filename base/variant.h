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

struct type_is_not_on_type_list{}; // quick and dirty, a tag struct

template < typename T, bool B > struct variant_traits;

template < typename T > struct variant_traits< T, false >
{
    template < typename S > static void set( boost::optional< T >& t, const S& ) { t.reset(); }
    template < typename S > static void set( boost::optional< T >& t, const boost::optional< S >& ) { t.reset(); }
    template < typename S, typename V > static const S& get( const boost::optional< T >&, const V& v ) { return v.template get< S >(); }
    template < typename S, typename V > static const boost::optional< S >& optional( const boost::optional< T >&, const V& v ) { return v.template optional< S >(); }
};

template < typename T > struct variant_traits< T, true >
{
    template < typename S > static void set( boost::optional< T >& t, const S& s ) { t = s; }
    template < typename S > static void set( boost::optional< T >& t, const boost::optional< S >& s ) { t = s; }
    template < typename S, typename V > static const S& get( const boost::optional< T >& t, const V& ) { return *t; }
    template < typename S, typename V > static const boost::optional< S >& optional( const boost::optional< T >& t, const V& ) { return t; }
};

template < typename T, typename... Args > struct variant  // todo? use tuple instead?
{
    boost::optional< T > t;
    variant< Args... > values;

    template < typename S > bool is() const { return ( std::is_same< T, S >::value && bool( t ) ) || values.template is< S >(); }
    operator bool() const { return bool( t ) || bool( values ); }
    template < typename S > void set( const S& s ) { variant_traits< T, std::is_same< T, S >::value >::set( t, s ); values.set( s ); }
    template < typename S > void set( const boost::optional< S >& s ) { variant_traits< T, std::is_same< T, S >::value >::set( t, s ); values.set( s ); }
    template < typename S > const S& get() const { return variant_traits< T, std::is_same< T, S >::value >::template get< S >( t, values ); }
    template < typename S > const boost::optional< S >& optional() const { return variant_traits< T, std::is_same< T, S >::value >::template optional< S >( t, values ); }
    void reset() { t.reset(); values.reset(); }
};

template < typename T > struct variant< T >  // todo? use tuple instead?
{
    boost::optional< T > t;

    template < typename S > bool is() const { return std::is_same< T, S >::value && bool( t ); }
    operator bool() const { return bool( t ); }
    template < typename S > void set( const S& s ) { variant_traits< T, std::is_same< T, S >::value >::set( t, s ); }
    template < typename S > void set( const boost::optional< S >& s ) { variant_traits< T, std::is_same< T, S >::value >::set( t, s ); }
    template < typename S > const S& get() const { return variant_traits< T, std::is_same< T, S >::value >::template get< S >( t, type_is_not_on_type_list() ); }
    template < typename S > const boost::optional< S >& optional() const { return variant_traits< T, std::is_same< T, S >::value >::template optional< S >( t, type_is_not_on_type_list() ); }
    void reset() { t.reset(); }
};

} // namespace impl {

/// @example
///     struct chirp { int a{1}; int b{2}; };
///     struct whistle { int a{3}; int b{4}; };
///     struct warble { int x{5}; int y{6}; };
///     comma::named_variant< naming, chirp, whistle, warble > sound;
template < typename... Args >
class variant
{
    public:
        variant() = default;
        template < typename S > variant( const S& s ) { set( s ); }
        template < typename S > bool is() const { return _values.template is< S >(); }
        operator bool() const { return bool( _values ); }
        template < typename S > void set( const S& s ) { _values.set( s ); }
        template < typename S > void set( const boost::optional< S >& s ) { _values.set( s ); }
        template < typename S > const S& get() const { return _values.template get< S >(); }
        template < typename S > const boost::optional< S >& optional() const { return _values.template optional< S >(); }
        void reset() { _values.reset(); }
    protected:
        impl::variant< Args... > _values;
};

/// @example
///     struct forest
///     {
///         struct chirp { int a{1}; int b{2}; };
///         struct whistle { int a{3}; int b{4}; };
///         struct warble { int x{5}; int y{6}; };
///
///         struct naming { static std::array< std::string, 3 > names() { return { "chirp", "whistle", "warble" }; } };
///
///         comma::named_variant< naming, chirp, whistle, warble > sound;
/// };
template < typename Names, typename... Args >
struct named_variant : public variant< Args... >, public Names
{
    typedef Names names_t;
    typedef variant< Args... > variant_t;
};

template < typename Names >
struct make_named_variant
{
    template < typename... Args > struct variant { typedef named_variant< Names, Args... > type; };
};

} // namespace comma {
