// Copyright (c) 2024 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <iostream>

namespace comma { namespace io { namespace terminal {

namespace controls {

struct end { static char value[]; };

struct stderr { static char start[]; static char end[]; }; // quick and dirty

struct titlebar { static char start[]; static char end[]; };

template < unsigned int Colour >
struct coloured; // todo

// todo! combining multiple controls

} // namespace controls {

template < typename T >
void write_to( const std::string& s ) { std::cerr << T::start << s << T::end; }

namespace impl {

template < typename S > struct traits
{
    template < typename T > static bool output_if_end( bool started ) { return false; }
    static void output( const S& s ) { std::cerr << s; }
};

template <> struct traits< terminal::controls::end >
{
    template < typename T > static bool output_if_end( bool started ) { if( started ) { std::cerr << T::end; } return true; }
    static void output( const terminal::controls::end& s ) {}
};

} // namespace impl {

template < typename T >
class ostream
{
    public:
        ~ostream() { if( _started ) { std::cerr << T::end; _started = false; } }

        static terminal::controls::end end() { return terminal::controls::end(); } // convenience method

        template < typename S >
        ostream& operator<<( const S& s )
        {
            if( impl::traits< S >::template output_if_end< T >( _started ) )
            {
                _started = false;
            }
            else
            {
                if( !_started ) { std::cerr << T::start; _started = true; }
                impl::traits< S >::output( s );
            }
            return *this;
        }

    protected:
        bool _started{false};
};

typedef ostream< controls::titlebar > titlebar_ostream;

} } } // namespace comma { namespace io { namespace terminal {
