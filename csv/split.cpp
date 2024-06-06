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
        oss << indent << "            period=<seconds>: create a new file if next" << std::endl;
        oss << indent << "                              timestamp passes <seconds> deadline" << std::endl;
        oss << indent << "            align : align deadline timestamp and respective filename" << std::endl;
        oss << indent << "                    exactly with the period (todo, just ask)" << std::endl;
        oss << indent << "        by-size;size=<bytes>" << std::endl;
        oss << indent << "            size=<bytes>: create files not larger than <bytes>" << std::endl;
        oss << indent << "                          may not be exact on ascii output" << std::endl;
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
    _time = t;
    _filename = _dir + "/" + timing::to_iso_string( t ) + "." + _suffix;
    _ofs = std::make_unique< std::ofstream >( _filename );
    COMMA_ASSERT( _ofs->is_open(), "failed to open '" << _filename << "'" );
    return _ofs.get();
}

} } } // namespace comma { namespace csv { namespace splitting {
