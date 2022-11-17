// Copyright (c) 2022 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

namespace comma { namespace csv {

class block_counter
{
    public:
        block_counter( comma::uint32 b = 0, comma::uint32 s = 0 ): _block( b ), _size( s ) {}
        comma::uint32 operator()() const { return _block; }
        comma::uint32 size() const { return _size; }
        comma::uint32 current_size() const { return _current_size; }
        bool fixed() const { return _size > 0; }
        template < typename T > bool operator==( const T& t ) const { return _size > 0 ? _current_size < _size : t.block == _block; } // dodgy?
        template < typename T > bool operator!=( const T& t ) const { return !operator==( t ); }
        template < typename T > void update( const T& t );
        template < typename T > bool ready( const T& t ) const { return _size > 0 ? _current_size == _size : t.block != _block; }
    private:
        comma::uint32 _block{0};
        comma::uint32 _size{0};
        comma::uint32 _current_size{0}; 
};

template < typename T > inline void block_counter::update( const T& t )
{ 
    if( _size == 0 ) { _block = t.block; return; }
    ++_current_size;
    if( _current_size <= _size ) { return; }
    _current_size = 1;
    ++_block;
}

} } // namespace comma { namespace csv {
