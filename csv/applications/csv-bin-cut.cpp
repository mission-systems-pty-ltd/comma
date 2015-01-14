// This file is part of comma, a generic and flexible library
// Copyright (c) 2011 The University of Sydney
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Sydney nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE
// GRANTED BY THIS LICENSE.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
// HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


/// @author vsevolod vlaskine

#ifdef WIN32
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <deque>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <comma/application/contact_info.h>
#include <comma/application/command_line_options.h>
#include <comma/application/signal_flag.h>
#include <comma/base/exception.h>
#include <comma/csv/format.h>
#include <comma/string/string.h>

using namespace comma;

static void usage()
{
    std::cerr << std::endl;
    std::cerr << "simplified, but similar as Linux cut utility, but for \"binary csv\"" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: cat blah.bin | csv-bin-cut <format> --fields=<fields> [--complement]" << std::endl;
    std::cerr << "    <fields>: field numbers, starting from 1 (to keep" << std::endl;
    std::cerr << "              consistent with the standard cut utility)" << std::endl;
    std::cerr << std::endl;
    std::cerr << csv::format::usage() << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( -1 );
}

int main( int ac, char** av )
{
    #ifdef WIN32
    _setmode( _fileno( stdin ), _O_BINARY ); /// @todo move to a library
    _setmode( _fileno( stdout ), _O_BINARY ); /// @todo move to a library
    #endif
    try
    {
        signal_flag shutdownFlag;
        command_line_options options( ac, av );
        if( ac < 2 || options.exists( "--help" ) || options.exists( "-h" ) ) { usage(); }
        comma::csv::format format( av[1] );
        if( !options.exists( "--fields" ) ) { std::cerr << "csv-bin-cut: please specify --fields" << std::endl; return 1; }
        std::vector< std::string > v = comma::split( options.value< std::string >( "--fields" ), ',' );
        std::vector< comma::csv::format::element > offsets;
        std::set< std::size_t > set;
        std::vector< std::size_t > indices;
        for( unsigned int i = 0; i < v.size(); ++i )
        {
            std::vector< std::string > r = comma::split( v[i], '-' );
            if( r.size() == 2 )
            {
                std::size_t begin = boost::lexical_cast< std::size_t >( r[0] );
                if( begin == 0 ) { std::cerr << "csv-bin-cut: field numbers start with 1 (to keep it consistent with linux cut utility)" << std::endl; return 1; }
                --begin;
                std::size_t end = boost::lexical_cast< std::size_t >( r[1] );
                if( end <= begin ) { std::cerr << "csv-bin-cut: expected range, got: " << v[i] << std::endl; return 1; }
                for( unsigned int k = begin; k < end; ++k ) { indices.push_back( k ); set.insert( k ); }
            }
            else
            {
                std::size_t index = boost::lexical_cast< std::size_t >( v[i] );
                if( index == 0 ) { std::cerr << "csv-bin-cut: field numbers start with 1 (to keep it consistent with linux cut utility)" << std::endl; return 1; }
                --index;
                if( set.find( index ) != set.end() ) { std::cerr << "csv-bin-cut: duplicated index " << v[i] << std::endl; return 1; }
                indices.push_back( index );
                set.insert( index );
            }
        }
        if( options.exists( "--complement" ) )
        {
            for( unsigned int i = 0; i < format.count(); ++i ) { if( set.find( i ) == set.end() ) { offsets.push_back( format.offset( i ) ); } }
        }
        else
        {
            for( unsigned int i = 0; i < indices.size(); ++i ) { offsets.push_back( format.offset( indices[i] ) ); }
        }
        std::vector< char > buf( format.size() );
        while( std::cin.good() && !std::cin.eof() )
        {
            if( shutdownFlag ) { std::cerr << "csv-bin-cut: interrupted by signal" << std::endl; return -1; }
            // quick and dirty; if performance is an issue, you could read more than
            // one record every time, but absolutely don't make this read blocking!
            // see comma::csv::binary_input_stream::read() for reference - if you know
            // how to do it better, please tell everyone!
            std::cin.read( &buf[0], format.size() );
            if( std::cin.gcount() == 0 ) { continue; }
            if( std::cin.gcount() < int( format.size() ) ) { std::cerr << "csv-bin-cut: expected " << format.size() << " bytes, got only " << std::cin.gcount() << std::endl; return 1; }
            for( unsigned int i = 0; i < offsets.size(); ++i )
            {
                std::cout.write( &buf[0] + offsets[i].offset, offsets[i].size );
            }
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-bin-cut: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-bin-cut: unknown exception" << std::endl; }
    usage();
}

