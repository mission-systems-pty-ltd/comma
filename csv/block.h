// Copyright (c) 2022 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

namespace comma { namespace csv {

class block_counter
{
    public:
        block_counter( comma::uint32 b = 0, comma::uint32 s = 0 ): _block( 0 ), _size( 0 ) {}
        comma::uint32 operator()() const { return _block; }
        comma::uint32 size() const { return _size; }
        bool fixed() const { return _size > 0; }
        template < typename T > bool operator==( const T& t ) const { return _size > 0 ? _block < _size : t.block == _block; }
        template < typename T > bool operator!=( const T& t ) const { return !operator==( t ); }
        template < typename T > comma::uint32 update( const T& t ) { _block = _size > 0 ? _block + 1 == _size ? 0 : _block + 1 : t.block; return _block; }
        template < typename T > bool ready( const T& t ) const { return _size > 0 ? _block + 1 == _size : t.block != _block; }
    private:
        comma::uint32 _block{0}; 
        comma::uint32 _size{0}; 
};

} } // namespace comma { namespace csv {
