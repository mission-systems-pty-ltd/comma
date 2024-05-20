// Copyright (c) 2024 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <fstream>
#include "../base/exception.h"
#include "../base/none.h"
#include "../io/stream.h"
#include "../string/string.h"
#include "../timing/duration.h"
#include "options.h"
#include "stream.h"

namespace comma { namespace csv {

namespace splitting {

std::string usage( unsigned int indent = 0, bool verbose = false );

template < typename T >
struct type_traits
{
    static boost::posix_time::ptime time( const T& t ) { return t.t; }
    static unsigned int block( const T& t ) { return t.block; }
    static unsigned int id( const T& t ) { return t.id; }
};

template < typename T >
struct method
{
    virtual ~method() {}
    virtual void wrote( unsigned int size ) {}
    virtual std::ostream* stream( const T& t, unsigned int size = 0 ) = 0;
};

template < typename T >
class none: public method< T >
{
    public:
        none( const std::string& address ): _ostream( address ) {}
        std::ostream* stream( const T&, unsigned int ) { return _ostream(); }
    
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

template < typename T >
class by_time: public method< T >
{
    public:
        by_time( boost::posix_time::time_duration max_duration, const std::string& dir, const options& csv, bool align = false );
        by_time( double max_duration, const std::string& dir, const options& csv, bool align ): by_time( timing::duration::from_seconds( max_duration ), dir, csv, align ) {}
        std::ostream* stream( const T& t, unsigned int ) { auto d = splitting::type_traits< T >::time( t ); return _is_due( d ) ? _ofs.update( d ) : _ofs(); }
    
    private:
        splitting::ofstream _ofs;
        boost::posix_time::time_duration _max_duration;
        bool _align{false};
        boost::posix_time::ptime _deadline;

        bool _is_due( boost::posix_time::ptime t );
};

template < typename T >
class by_size: public method< T >
{
    public:
        by_size( std::size_t size, const std::string& dir, const options& csv );
        std::ostream* stream( const T& t, unsigned int size = 0 ) { return _is_due( size ) ? _ofs.update( t ) : _ofs(); }
        void wrote( unsigned int size );

    private:
        splitting::ofstream _ofs;
        std::size_t _size{0};
        std::size_t _record_size{0};
        double _average_record_size{0};
        std::size_t _count{0};
        std::size_t _remaining{0};

        bool _is_due( unsigned int extra_size );
};

template < typename T >
class by_block: public method< T >
{
    public:
        by_block( const std::string& dir, const options& csv ): _ofs( dir, csv ), _block( silent_none< unsigned int >() ) {}
        std::ostream* stream( const T& t, unsigned int ) { return _is_due( splitting::type_traits< T >::block( t ) ) ? _ofs.update( t ) : _ofs(); }

    private:
        splitting::ofstream _ofs;
        boost::optional< unsigned int > _block;

        bool _is_due( unsigned int block );
};

template < typename T >
class by_id; // todo

} // namespace splitting {

template < typename T >
class split
{
    public:
        split( splitting::method< T >* how, const options& csv, const T& sample = T() ): _how( how ), _options( csv ), _sample( sample ) {}
        split& write( const T& t, const char* buf, unsigned int size );
        split& operator<<( const T& t ) { return write( t, nullptr, 0 ); }
        bool eof() const { return _eof || ( _os && _os->eof() ); }
        static split< T >* make( const std::string& options, const csv::options& csv, bool permissive = false, const T& sample = T() );

    protected:
        std::unique_ptr< splitting::method< T > > _how;
        options _options;
        T _sample;
        bool _eof{false};
        std::ostream* _os{nullptr};
        std::unique_ptr< output_stream< T > > _ostream;
        void _init();
};

template < typename T > inline split< T >& split< T >::write( const T& t, const char* buf, unsigned int size )
{
    COMMA_ASSERT( !_eof, "end of stream" );
    std::ostream* os = _how->stream( t );
    if( !os || os->eof() )
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
    _ostream->write( t );
    if( buf ) { _os->write( buf, size ); }
    _how->wrote( _ostream->last_size() + size );
    return *this;
}

template < typename T > inline split< T >* split< T >::make( const std::string& options, const csv::options& csv, bool permissive, const T& sample )
{
    const auto& v = comma::split_head( options, 2, ':' );
    if( v[0] == "split" ) { COMMA_THROW( comma::exception, "on '" << options << "': 'split': todo, just ask" ); }
    if( v[0] == "log" ) // todo: reimplement using comma::variant
    {
        const auto& w = comma::split( v[1], ';' );
        COMMA_ASSERT( w.size() >= 2, "expected log:<dir>;<method>[;<options>]; got: '" << options << "'" );
        std::string dir = w[0];
        std::string method = w[1];
        if( method == "by-time" )
        {
            boost::optional< double > period = silent_none< double >();
            bool align{false};
            for( unsigned int i = 2; i < w.size(); ++i )
            {
                const auto& s = comma::split( w[i], '=' );
                if( s[0] == "period" && s.size() == 2 ) { period = boost::lexical_cast< double >( s[1] ); }
                else if( ( s[0] == "align" && s.size() == 1 ) || w[i] == "align=true" || w[i] == "align=1" ) { align = true; }
            }
            if( period ) { return new split< T >( new splitting::by_time< T >( *period, dir, csv, align ), csv, sample ); }
            if( permissive ) { return nullptr; }
            COMMA_THROW( comma::exception, "expected 'log:<dir>;by-time;period=<seconds>[;align]'; got: '" << options << "'" );
        }
        if( method == "by-size" )
        {
            for( unsigned int i = 2; i < w.size(); ++i )
            {
                const auto& s = comma::split( w[i], '=' );
                if( s[0] == "size" && s.size() == 2 )
                { 
                    if( permissive ) { return nullptr; }
                    return new split< T >( new splitting::by_size< T >( boost::lexical_cast< unsigned int >( s[1] ), dir, csv ), csv, sample );
                }
            }
            if( permissive ) { return nullptr; }
            COMMA_THROW( comma::exception, "expected 'log:<dir>;by-size;size=<bytes>'; got: '" << options << "'" );
        }
        if( method == "by-block" )
        {
            return new split< T >( new splitting::by_block< T >( dir, csv ), csv, sample );
        }
        if( permissive ) { return nullptr; }
        COMMA_THROW( comma::exception, "expected 'by-time', 'by-size' or 'by-block', got: '" << v[0] << " in '" << options << "'" );
    }
    if( permissive ) { return nullptr; }
    COMMA_THROW( comma::exception, "expected 'split' or 'log', got: '" << v[0] << " in '" << options << "'" );
}

namespace splitting {

template < typename T >
inline by_time< T >::by_time( boost::posix_time::time_duration max_duration, const std::string& dir, const options& csv, bool align )
    : _ofs( dir, csv )
    , _max_duration( max_duration )
    , _align( align )
{
    COMMA_THROW_IF( align, "align: todo, just ask" );
}

template < typename T >
inline bool by_time< T >::_is_due( boost::posix_time::ptime t )
{
    if( !_deadline.is_not_a_date_time() && t < _deadline ) { return false; }
    _deadline = t + _max_duration;
    return true;
}

template < typename T >
inline by_size< T >::by_size( std::size_t size, const std::string& dir, const options& csv )
    : _ofs( dir, csv )
    , _size( size )
    , _record_size( csv.binary() ? csv.format().size() : 0 )
{
}

template < typename T >
inline bool by_size< T >::_is_due( unsigned int extra_size )
{
    if( ( _record_size ? _record_size + extra_size : ( unsigned int )( _average_record_size ) ) <= _remaining ) { return false; }
    _remaining = _size + extra_size;
    return true;
}

template < typename T >
inline void by_size< T >::wrote( unsigned int size )
{
    _remaining = _remaining >= size ? _remaining - size : 0;
    if( _record_size ) { return; }
    ++_count;
    if( _count == 1 ) { _average_record_size = size; return; }
    double r = 1. / _count;
    _average_record_size = ( 1 - r ) * _average_record_size + r * size; // quick and dirty
}

template < typename T >
inline bool by_block< T >::_is_due( unsigned int block )
{
    if( _block && *_block == block ) { return false; }
    _block = block;
    return true;
}

} // namespace splitting {

} } // namespace comma { namespace csv {
