// Copyright (c) 2024 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

// todo
// #if __cplusplus >= 201703L
// #include <optional>
// #else
// #include <boost/optional.hpp>
// #endif
#if __cplusplus >= 201703L
    #include <variant>
#else
    // ...
#endif
#include <type_traits>
#include <boost/optional.hpp>
#include "exception.h"

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
    enum { size = variant< Args... >::size + 1 };
    boost::optional< T > t;
    variant< Args... > values;

    template < typename S > bool is() const { return ( std::is_same< T, S >::value && bool( t ) ) || values.template is< S >(); }
    operator bool() const { return bool( t ) || bool( values ); }
    template < typename S > void set( const S& s ) { variant_traits< T, std::is_same< T, S >::value >::set( t, s ); values.set( s ); }
    template < typename S > void set( const boost::optional< S >& s ) { variant_traits< T, std::is_same< T, S >::value >::set( t, s ); values.set( s ); }
    template < typename S > const S& get() const { return variant_traits< T, std::is_same< T, S >::value >::template get< S >( t, values ); }
    template < typename S > S& get() { return variant_traits< T, std::is_same< T, S >::value >::template get< S >( t, values ); }
    template < typename S > const boost::optional< S >& optional() const { return variant_traits< T, std::is_same< T, S >::value >::template optional< S >( t, values ); }
    template < unsigned int I > auto at() const { using type = typename std::tuple_element< I, std::tuple< T, Args... > >::type; auto v = optional< type >(); return v ? *v : type(); }
    void reset() { t.reset(); values.reset(); }
    template < typename S > static unsigned int rindex() { return std::is_same< T, S >::value ? size - 1 : variant< Args... >::template rindex< S >(); }
    unsigned int index( unsigned int i = 0 ) const { return t ? i : values.index( i + 1 ); }
    template <  typename V > V& as( V& v ) const { if( t ) { v = *t; return v; }; return values.as( v ); }
};

template < typename T > struct variant< T >  // todo? use tuple instead?
{
    enum { size = 1 };
    boost::optional< T > t;

    template < typename S > bool is() const { return std::is_same< T, S >::value && bool( t ); }
    operator bool() const { return bool( t ); }
    template < typename S > void set( const S& s ) { variant_traits< T, std::is_same< T, S >::value >::set( t, s ); }
    template < typename S > void set( const boost::optional< S >& s ) { variant_traits< T, std::is_same< T, S >::value >::set( t, s ); }
    template < typename S > const S& get() const { return variant_traits< T, std::is_same< T, S >::value >::template get< S >( t, type_is_not_on_type_list() ); }
    template < typename S > S& get() { return variant_traits< T, std::is_same< T, S >::value >::template get< S >( t, type_is_not_on_type_list() ); }
    template < typename S > const boost::optional< S >& optional() const { return variant_traits< T, std::is_same< T, S >::value >::template optional< S >( t, type_is_not_on_type_list() ); }
    template < unsigned int I > auto at() const { using type = typename std::tuple_element< I, std::tuple< T > >::type; auto v = optional< type >(); return v ? *v : type(); }
    void reset() { t.reset(); }
    template < typename S > static unsigned int rindex() { bool same_type = std::is_same< T, S >::value; COMMA_ASSERT( same_type, "type not found in type list" ); return 0; }
    unsigned int index( unsigned int i = 0 ) const { return t ? i : ( i + 1 ); }
    template <  typename V > V& as( V& v ) const { if( t ) { v = *t; }; return v; }
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
        enum { size = impl::variant< Args... >::size };
        variant() = default;
        template < typename S > variant( const S& s ) { set( s ); }
        template < typename S > bool is() const { return _values.template is< S >(); }
        operator bool() const { return bool( _values ); }
        template < typename S > void set( const S& s ) { _values.set( s ); }
        template < typename S > void set( const boost::optional< S >& s ) { _values.set( s ); }
        template < typename S > const S& get() const { return _values.template get< S >(); }
        template < typename S > const boost::optional< S >& optional() const { return _values.template optional< S >(); }
        void reset() { _values.reset(); }
        template < typename S > static unsigned int index_of() { return impl::variant< Args... >::size - impl::variant< Args... >::template rindex< S >() - 1; }
        unsigned int index() const { return _values.index(); }
        const impl::variant< Args... >& operator()() const { return _values; }
        operator impl::variant< Args... >() const { return _values; }
        template < typename F > bool visit( F&& f ) { return _values.visit( f ); }
        template < typename V > V as() const { V v; return _values.as( v ); }
        template < unsigned int I > auto at() const { return _values.template at< I >(); }
        variant& touch_at( unsigned int i );
        // auto at( unsigned int i ) const;
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
    template < typename S > static auto name_of() { return Names::names()[ variant_t::template index_of< S >() ]; }
    auto name() const { COMMA_ASSERT( bool( *this ), "asked for name, but value is not set" ); return this->names()[this->index()]; }
    static unsigned int index_of( const std::string& n );
    using variant_t::touch_at;
    named_variant& touch_at( const std::string& n );
};

template < typename Names >
struct make_named_variant
{
    template < typename... Args > struct variant { typedef named_variant< Names, Args... > type; };
};

template < unsigned int I, bool Valid > struct _apply;

template < unsigned int I > struct _apply< I, true >
{
    template < typename T > static auto at( const T& t ) { return t.template at< I >(); }
    template < typename T > static T& touch_at( T& t ) { t = t.template at< I >(); return t; } // quick and dirty
};

template < unsigned int I > struct _apply< I, false >
{
    template < typename T > static auto at( const T& t ) { COMMA_THROW( comma::exception, "expected index less than variant size; got: " << I ); } // never gets called
    template < typename T > static T& touch_at( T& t ) { COMMA_THROW( comma::exception, "expected index less than variant size; got: " << I ); } // never gets called
};

// template < typename... Args >
// inline auto variant< Args... >::at( unsigned int i ) const
// {
//     constexpr unsigned int s = std::tuple_size< std::tuple< Args... > >::value;
//     switch( i )
//     {
//         case 0: return _apply< 0, 0 < s >::at( *this );
//         case 1: return _apply< 1, 1 < s >::at( *this );
//         case 2: return _apply< 2, 2 < s >::at( *this );
//         case 3: return _apply< 3, 3 < s >::at( *this );
//         case 4: return _apply< 4, 4 < s >::at( *this );
//         case 5: return _apply< 5, 5 < s >::at( *this );
//         case 6: return _apply< 6, 6 < s >::at( *this );
//         case 7: return _apply< 7, 7 < s >::at( *this );
//         case 8: return _apply< 8, 8 < s >::at( *this );
//         case 9: return _apply< 9, 9 < s >::at( *this );
//         case 10: return _apply< 10, 10 < s >::at( *this );
//         case 11: return _apply< 11, 11 < s >::at( *this );
//     }
//     COMMA_THROW( comma::exception, "currently up to 12 types are supported; got index " << i );
// }

template < typename... Args >
inline variant< Args... >& variant< Args... >::touch_at( unsigned int i )
{
    constexpr unsigned int s = std::tuple_size< std::tuple< Args... > >::value;
    switch( i )
    {
        case 0: return _apply< 0, 0 < s >::touch_at( *this );
        case 1: return _apply< 1, 1 < s >::touch_at( *this );
        case 2: return _apply< 2, 2 < s >::touch_at( *this );
        case 3: return _apply< 3, 3 < s >::touch_at( *this );
        case 4: return _apply< 4, 4 < s >::touch_at( *this );
        case 5: return _apply< 5, 5 < s >::touch_at( *this );
        case 6: return _apply< 6, 6 < s >::touch_at( *this );
        case 7: return _apply< 7, 7 < s >::touch_at( *this );
        case 8: return _apply< 8, 8 < s >::touch_at( *this );
        case 9: return _apply< 9, 9 < s >::touch_at( *this );
        case 10: return _apply< 10, 10 < s >::touch_at( *this );
        case 11: return _apply< 11, 11 < s >::touch_at( *this );
    }
    COMMA_THROW( comma::exception, "currently up to 12 types are supported; got index " << i );
}

template < typename Names, typename... Args >
inline unsigned int named_variant< Names, Args... >::index_of( const std::string& n )
{
    for( unsigned int i = 0; i < Names::names().size(); ++i ) { if( Names::names()[i] == n ) { return i; } }
    COMMA_THROW( comma::exception, "not found in names: '" << n << "'" );
}

template < typename Names, typename... Args >
inline named_variant< Names, Args... >& named_variant< Names, Args... >::touch_at( const std::string& n ) { this->touch_at( index_of( n ) ); return *this; }

} // namespace comma {
