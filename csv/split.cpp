// Copyright (c) 2024 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#include "split.h"

namespace comma { namespace csv { namespace splitting {

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
