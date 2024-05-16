// Copyright (c) 2024 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#include <sstream>
#include "../timing/conversions.h"
#include "split.h"

namespace comma { namespace csv { namespace splitting {

std::string usage( unsigned int size, bool verbose )
{
    std::string indent( size, ' ' );
    std::ostringstream oss;
    oss << indent << "split:<options>      : todo" << std::endl;
    oss << indent << "log:<dir>;<options>  : log in timestamped files" << std::endl;
    if( verbose )
    {
        oss << indent << "    <options>: <how>[;<parameters>]" << std::endl;
        oss << indent << "        by-time;period=<seconds>[;align]" << std::endl;
        oss << indent << "        by-size;size=<bytes>" << std::endl;
        oss << indent << "        by-block" << std::endl;
    }
    else
    {
        oss << indent << "    run --help --verbose for details..." << std::endl;
    }
    return oss.str();
}

std::ofstream* ofstream::update( boost::posix_time::ptime t )
{
    if( _ofs ) { _ofs.reset(); }
    std::string filename = _dir + "/" + timing::to_iso_string( t ) + "." + _suffix;
    _ofs = std::make_unique< std::ofstream >( filename );
    COMMA_ASSERT( _ofs->is_open(), "failed to open '" << filename << "'" );
    return _ofs.get();
}

by_time::by_time( boost::posix_time::time_duration max_duration, const std::string& dir, const options& csv, bool align )
    : _ofs( dir, csv )
    , _max_duration( max_duration )
    , _align( align )
{
    COMMA_THROW_IF( align, "align: todo" );
}

bool by_time::_is_due( boost::posix_time::ptime t )
{
    if( !_deadline.is_not_a_date_time() && t < _deadline ) { return false; }
    _deadline = t + _max_duration;
    return true;
}

by_size::by_size( std::size_t size, const std::string& dir, const options& csv )
    : _ofs( dir, csv )
    , _size( size )
    , _record_size( csv.binary() ? csv.format().size() : 0 )
{
}

bool by_size::_is_due()
{
    if( _estimated_record_size <= _remaining ) { return false; }
    _remaining = _size;
    return true;
}

void by_size::wrote( unsigned int size )
{
    _remaining = _remaining > size ? _remaining - size : 0;
}

bool by_block::_is_due( unsigned int block )
{
    if( _block && *_block == block ) { return false; }
    _block = block;
    return true;
}

} } } // namespace comma { namespace csv { namespace splitting {
