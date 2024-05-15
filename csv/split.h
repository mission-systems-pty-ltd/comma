// Copyright (c) 2024 Vsevolod Vlaskine

/// @author vsevolod vlaskine

#pragma once

#include <fstream>
#include "options.h"
#include "stream.h"

namespace comma { namespace csv {

template < typename T, typename How >
class split
{
    public:
        split( const How& how, const options& csv, const T& sample = T() ): _how( how ), _options( csv ), _sample( sample ) { _init(); }
        split( How&& how, const options& csv, const T& sample = T() ): _how( how ), _options( csv ), _sample( sample ) { _init(); }
        split& operator<<( const T& t );

    protected:
        How _how;
        options _options;
        T _sample;
        std::ostream* _os{nullptr};
        std::unique_ptr< output_stream< T > > _ostream;
        void _init();

};

template < typename T, typename How > inline void split< T, How >::_init()
{
    // todo: validate options
    // todo: init
}

template < typename T, typename How > inline split< T, How >& split< T, How >::operator<<( const T& t )
{
    std::ostream* os = _how.stream( t );
    if( _os != os )
    {
        _os->flush(); // paranoia
        _ostream.reset();
        _os = os;
        _ostream = std::make_unique< output_stream< T > >( *_os, _options, _sample );
    }
    _ostream.write( t );
    return *this;
}

} } // namespace comma { namespace csv {
