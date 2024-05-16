// Copyright (c) 2024 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <fstream>
#include "../base/exception.h"
#include "../io/stream.h"
#include "../timing/duration.h"
#include "options.h"
#include "stream.h"

namespace comma { namespace csv {

namespace splitting {

class none
{
    public:
        none( const std::string& address ): _ostream( address ) {}
        template < typename T >
        std::ostream* stream( const T& t ) { return _ostream(); }

    private:
        io::ostream _ostream;
};

class by_time
{
    public:
        by_time( boost::posix_time::time_duration period ): _period( period ) {}
        by_time( double period ): _period( timing::duration::from_seconds( period ) ) {}
        template < typename T >
        std::ostream* stream( const T& t );

    private:
        boost::posix_time::ptime _deadline;
        boost::posix_time::time_duration _period;
        std::unique_ptr< std::ofstream > _ofs;
};

class by_size
{
    public:
        by_size( std::size_t size ): _size( size ) {}
        template < typename T >
        std::ostream* stream( const T& t );

    private:
        std::size_t _size{0};
        std::size_t _remaining{0};
        std::unique_ptr< std::ofstream > _ofs;
};

class by_id; // todo

} // namespace splitting {

template < typename T, typename How >
class split
{
    public:
        split( const How& how, const options& csv, const T& sample = T() ): _how( how ), _options( csv ), _sample( sample ) {}
        split( How&& how, const options& csv, const T& sample = T() ): _how( how ), _options( csv ), _sample( sample ) {}
        split& operator<<( const T& t );
        bool eof() const { return _eof; }

    protected:
        How _how;
        options _options;
        T _sample;
        bool _eof{false};
        std::ostream* _os{nullptr};
        std::unique_ptr< output_stream< T > > _ostream;
        void _init();
};

template < typename T, typename How > inline split< T, How >& split< T, How >::operator<<( const T& t )
{
    COMMA_ASSERT( !_eof, "end of stream" );
    std::ostream* os = _how.stream( t );
    if( !os )
    {
        _eof = true;
        _os = nullptr;
        return *this;
    }
    if( _os != os )
    {
        _ostream.reset();
        _os = os;
        _ostream = std::make_unique< output_stream< T > >( *_os, _options, _sample );
    }
    _ostream.write( t );
    return *this;
}

} } // namespace comma { namespace csv {
