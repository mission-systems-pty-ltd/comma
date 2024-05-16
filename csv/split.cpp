// Copyright (c) 2024 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#include "split.h"
#include "../timing/conversions.h"

namespace comma { namespace csv { namespace splitting {

std::ofstream* ofstream::update( boost::posix_time::ptime t )
{
    if( _ofs ) { _ofs.reset(); }
    std::string filename = _dir + "/" + timing::to_iso_string( t ) + "." + _suffix;
    _ofs = std::make_unique< std::ofstream >( filename );
    COMMA_ASSERT( _ofs->is_open(), "failed to open '" << filename << "'" );
    return _ofs.get();
}

bool by_time::_is_due( boost::posix_time::ptime t ) const
{
    // todo
    return false;
}

by_size::by_size( std::size_t size, const std::string& dir, const options& csv )
    : _ofs( dir, csv )
    , _size( size )
    , _record_size( csv.binary() ? csv.format().size() : 0 )
{
}

bool by_size::_is_due() const
{
    // todo
    return false;
}

bool by_block::_is_due( unsigned int block ) const
{
    // todo
    return false;
}

} } } // namespace comma { namespace csv { namespace splitting {
