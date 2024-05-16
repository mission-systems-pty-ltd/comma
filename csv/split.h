// Copyright (c) 2024 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <fstream>
#include "../base/exception.h"
#include "../base/none.h"
#include "../io/stream.h"
#include "../timing/duration.h"
#include "options.h"
#include "stream.h"

namespace comma { namespace csv {

namespace splitting {

template < typename T >
struct type_traits
{
    static boost::posix_time::ptime time( const T& t ) { return t.t; }
    static unsigned int block( const T& t ) { return t.block; }
    static unsigned int id( const T& t ) { return t.id; }
};

class none
{
    public:
        none( const std::string& address ): _ostream( address ) {}
        template < typename T >
        std::ostream* stream( const T& t ) { return _ostream(); }
        void wrote( unsigned int ) {}

    private:
        io::ostream _ostream;
};

class ofstream
{
    public:
        ofstream( const std::string& dir, const std::string& suffix ): _dir( dir ), _suffix( suffix ) {}
        ofstream( const std::string& dir, const options& csv ): _dir( dir ), _suffix( csv.binary() ? "bin" : "csv" ) {}
        std::ofstream* update( boost::posix_time::ptime t );
        template < typename T > std::ofstream* update( const T& t ) { return update( splitting::type_traits< T >::time( t ) ); }
        std::ofstream* operator()() { return _ofs.get(); }
    protected:
        std::string _dir;
        std::string _suffix;
        std::unique_ptr< std::ofstream > _ofs;
};

class by_time
{
    public:
        by_time( boost::posix_time::time_duration max_duration, const std::string& dir, const options& csv ): _ofs( dir, csv ), _max_duration( max_duration ) {}
        by_time( double max_duration, const std::string& dir, const options& csv ): by_time( timing::duration::from_seconds( max_duration ), dir, csv ) {}
        template < typename T > std::ostream* stream( const T& t ) { auto d = splitting::type_traits< T >::time( t ); return _is_due( d ) ? _ofs.update( d ) : _ofs(); }
        void wrote( unsigned int ) {}

    private:
        splitting::ofstream _ofs;
        boost::posix_time::time_duration _max_duration;
        boost::posix_time::ptime _deadline;

        bool _is_due( boost::posix_time::ptime t );
};

class by_size
{
    public:
        by_size( std::size_t size, const std::string& dir, const options& csv );
        template < typename T >
        std::ostream* stream( const T& t ) { return _is_due() ? _ofs.update( t ) : _ofs(); }
        void wrote( unsigned int size );

    private:
        splitting::ofstream _ofs;
        std::size_t _size{0};
        std::size_t _record_size{0};
        std::size_t _estimated_record_size{0};
        std::size_t _remaining{0};

        bool _is_due();
};

class by_block
{
    public:
        by_block( const std::string& dir, const options& csv ): _ofs( dir, csv ), _block( silent_none< unsigned int >() ) {}
        template < typename T >
        std::ostream* stream( const T& t ) { return _is_due( splitting::type_traits< T >::block( t ) ) ? _ofs.update( t ) : _ofs(); }
        void wrote( unsigned int ) {}

    private:
        splitting::ofstream _ofs;
        boost::optional< unsigned int > _block;

        bool _is_due( unsigned int block );
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
    _how.wrote( _ostream.last_size() );
    return *this;
}

} } // namespace comma { namespace csv {
