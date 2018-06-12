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

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "../../application/command_line_options.h"
#include "../../application/contact_info.h"
#include "../../base/exception.h"
#include "../../csv/format.h"
#include "../../io/stream.h"
#include "../../name_value/parser.h"
#include "../../string/string.h"

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#endif

static void usage( bool verbose )
{
    std::cerr << std::endl;
    std::cerr << "join several csv sources and output to stdout, e.g.:" << std::endl;
    std::cerr << "    1,2,3 + 4,5 + 6 -> 1,2,3,4,5,6" << std::endl;
    std::cerr << std::endl;
    std::cerr << "in the simplest case, the lines in all the given csv sources" << std::endl;
    std::cerr << "are expected to be exactly in the same order;" << std::endl;
    std::cerr << std::endl;
    std::cerr << "something like:" << std::endl;
    std::cerr << "    csv-paste \"file1.csv\" \"file2.csv\" \"value=<value>\" \"line-number\"" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: csv-paste [<options>] <file> <file>..." << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples:" << std::endl;
    std::cerr << "    csv-paste \"file1.bin;binary=%d%d\" \"file2.bin;size=5\" \"value=1,2;binary=ui,d\"" << std::endl;
    std::cerr << "    csv-paste \"file1.csv\" \"file2.csv\" value=1,2,3" << std::endl;
    std::cerr << "    cat file1.csv | csv-paste - line-number" << std::endl;
    std::cerr << "    cat file1.bin | csv-paste \"-;binary=d,d\" \"value=0;binary=ui\"" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options:" << std::endl;
    std::cerr << "    --delimiter,-d <delimiter> : default ','" << std::endl;
    std::cerr << "    <file> : <filename>[;size=<size>|binary=<format>]: file name or \"-\" for stdin; specify size or format, if binary" << std::endl;
    std::cerr << "    <value> : <csv values>[;binary=<format>]; specify size or format, if binary" << std::endl;
    std::cerr << "    line-number[;<options>] : add the line number; as ui, if binary (quick and dirty, will override the file named \"line-number\")" << std::endl;
    std::cerr << "        options" << std::endl;
    std::cerr << "            --begin <index>: start line number count at <index>; default: 0" << std::endl;
    std::cerr << "            --index; instead of block number output record index in the block" << std::endl;
    std::cerr << "            --reverse; if --index, output index in descending order" << std::endl;
    std::cerr << "            --size,--block-size <size>: number of records with the same line number; default: 1" << std::endl;
    std::cerr << "        examples (try them)" << std::endl;
    std::cerr << "            seq 0 20 | csv-paste - line-number --begin 5 --size 3" << std::endl;
    std::cerr << "            csv-paste line-number \"line-number;index;reverse\" --size 10 | head -n20" << std::endl;
    std::cerr << comma::csv::format::usage() << std::endl;
    std::cerr << std::endl;
    std::cerr << comma::contact_info << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

class source
{
    public:
        source( const std::string& properties = "" ) : properties_( properties )
        {
            comma::name_value::map map( properties, ';', '=' );
            format_ = comma::csv::format( map.value< std::string >( "binary", "" ) );
            unsigned int size = map.value< unsigned int >( "size", format_.size() );
            binary_ = size > 0;
            value_ = std::string( size, 0 );
        }
        virtual ~source() {}
        virtual const std::string* read() = 0;
        virtual const char* read( char* buf ) = 0;
        bool binary() const { return binary_; }
        virtual const bool is_stream() const { return false; }
        const std::string& properties() const { return properties_; }
        std::size_t size() const { return value_.size(); }
        
    protected:
        std::string value_;
        bool binary_;
        comma::csv::format format_;
        std::string properties_;
};

class stream : public source
{
    public:
        stream( const std::string& properties )
            : source( properties )
            , stream_( comma::split( properties, ';' )[0], binary() ? comma::io::mode::binary : comma::io::mode::ascii )
        {
        }
        
        const std::string* read()
        {
            while( stream_->good() && !stream_->eof() )
            {
                std::getline( *stream_, value_ );
                if( !value_.empty() && *value_.rbegin() == '\r' ) { value_ = value_.substr( 0, value_.length() - 1 ); } // windows... sigh...
                if( !value_.empty() ) { return &value_; }
            }
            return NULL;
        }

        const char* read( char* buf )
        {
            stream_->read( buf, value_.size() );
            return stream_->gcount() == int( value_.size() ) ? buf : NULL;
        }
        
        const bool is_stream() { return true; }
        
    private:
        comma::io::istream stream_;
};

struct value : public source
{
    value( const std::string& properties ) : source( properties )
    {
        comma::name_value::map map( properties, ';', '=' );
        std::string value = map.value< std::string >( "value" );
        char delimiter = map.value( "delimiter", ',' );
        value_ = binary_ ? format_.csv_to_bin( value, delimiter ) : value;
    }
    const std::string* read() { return &value_; }
    const char* read( char* buf ) { ::memcpy( buf, &value_[0], value_.size() ); return buf; } // quick and dirty
};

class line_number : public source
{
    public:
        class options
        {
            public:
                comma::uint32 size;
                bool index;
                bool reverse;
                comma::uint32 begin;
                
                options( boost::optional< comma::uint32 > b = boost::optional< comma::uint32 >(), comma::uint32 size = 1, bool index = false, bool reverse = false )
                    : size( size )
                    , index( index )
                    , reverse( reverse )
                    , begin( begin_( b ) )
                {
                }
                
                options( const std::string& properties, const comma::command_line_options& o ) // quick and dirty: use visiting instead
                {
                    options defaults( boost::optional< comma::uint32 >(), o.value< comma::uint32 >( "--size,--block-size", 1 ), o.exists( "--index" ), o.exists( "--reverse" ) );
                    comma::name_value::map map( properties, ';', '=' );
                    size = map.value< comma::uint32 >( "size", defaults.size );
                    index = map.value< bool >( "index", defaults.index );
                    reverse = map.value< bool >( "reverse", defaults.reverse );
                    auto b = map.optional< comma::uint32 >( "begin" );
                    if( !b ) { b = o.optional< comma::uint32 >( "--begin" ); }
                    begin = begin_( b );
                }
                
            private:
                comma::uint32 begin_( const boost::optional< comma::uint32 >& b )
                {
                    if( index && reverse && b && ( *b + 1 ) < size ) { COMMA_THROW( comma::exception, "for --reverse --index, for --size " << size << " expected --begin not less than " << ( size - 1 ) << "; got: " << *b ); }
                    return b ? *b : reverse ? size - 1 : 0;
                }
        };
        
        line_number( bool is_binary, const options& options )
            : source( is_binary ? "binary=ui" : "" )
            , options_( options )
            , count_( 0 )
            , value_( options_.begin )
        {
        }
        
        const std::string* read()
        { 
            serialized_ = boost::lexical_cast< std::string >( value_ );
            update_();
            return &serialized_;
        }
        
        const char* read( char* buf ) // quick and dirty
        {
            comma::csv::format::traits< comma::uint32 >::to_bin( value_, buf );
            update_();
            return buf;
        }
        
    private:
        options options_;
        comma::uint32 count_;
        comma::uint32 value_;
        std::string serialized_;
        
        void update_()
        {
            ++count_;
            if( count_ < options_.size )
            {
                if( options_.index ) { value_ += options_.reverse ? -1 : 1; }
            }
            else
            {
                value_ = options_.index ? options_.begin : ( value_ + 1 );
                count_ = 0;
            }
        }
};

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        char delimiter = options.value( "--delimiter,-d", ',' );
        std::vector< std::string > unnamed = options.unnamed( "--flush,--index,--reverse", "--delimiter,-d,--begin,--size,--block-size" );
        boost::ptr_vector< source > sources;
        bool is_binary = false;
        for( unsigned int i = 0; i < unnamed.size(); ++i ) // quick and dirty
        {
            if( unnamed[i].substr( 0, 6 ) == "value=" ) { if( value( unnamed[i] ).binary() ) { is_binary = true; } }
            else if( unnamed[i] == "line-number" || unnamed[i].substr( 0, 12 ) == "line-number;" ) { continue; } // quick and dirty
            if( stream( unnamed[i] ).binary() ) { is_binary = true; }
        }
        for( unsigned int i = 0; i < unnamed.size(); ++i )
        {
            source* s;
            if( unnamed[i].substr( 0, 6 ) == "value=" )
            { 
                s = new value( unnamed[i] );
                if( is_binary != s->binary() ) { std::cerr << "csv-paste: one input is ascii, the other binary: " << sources.back().properties() << " vs " << s->properties() << std::endl; return 1; }
            }
            else if( unnamed[i] == "line-number" || unnamed[i].substr( 0, 12 ) == "line-number;" ) // quick and dirty
            {
                s = new line_number( is_binary, line_number::options( unnamed[i], options ) );
            }
            else
            {
                s = new stream( unnamed[i] );
                if( is_binary != s->binary() ) { std::cerr << "csv-paste: one input is ascii, the other binary: " << sources.back().properties() << " vs " << s->properties() << std::endl; return 1; }
            }
            sources.push_back( s );
        }
        if( sources.empty() ) { std::cerr << "csv-paste: expected at least one input, got none" << std::endl; return 1; }
        if( is_binary )
        {
            #ifdef WIN32
                _setmode( _fileno( stdout ), _O_BINARY );
            #endif
            std::size_t size = 0;
            for( unsigned int i = 0; i < sources.size(); ++i ) { size += sources[i].size(); }
            std::vector< char > buffer( size );
            while( true )
            {
                unsigned int streams = 0;
                char* p = &buffer[0];
                for( unsigned int i = 0; i < sources.size(); p += sources[i].size(), ++i )
                {
                    if( sources[i].read( p ) == NULL )
                    {
                        if( streams == 0 ) { return 0; }
                        std::cerr << "csv-paste: unexpected end of file in " << unnamed[i] << std::endl;
                        return 1;
                    }
                    if( sources[i].is_stream() ) { ++streams; }
                }
                std::cout.write( &buffer[0], buffer.size() );
                std::cout.flush();
            }
        }
        else
        {
            while( true )
            {
                std::ostringstream oss;
                unsigned int streams = 0;
                for( unsigned int i = 0; i < sources.size(); ++i )
                {
                    const std::string* s = sources[i].read();
                    if( s == NULL )
                    {
                        if( streams == 0 ) { return 0; }
                        std::cerr << "csv-paste: unexpected end of file in " << unnamed[i] << std::endl; return 1;
                    }
                    if (sources[i].is_stream()) ++streams;
                    if( i > 0 ) { oss << delimiter; }
                    oss << *s;
                }
                std::cout << oss.str() << std::endl;
            }
        }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-paste: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-paste: unknown exception" << std::endl; }
    return 1;
}


// int main( int ac, char** av )
// {
//     bool show_usage = true;
//     try
//     {
//         comma::command_line_options options( ac, av );
//         if( options.exists( "--help,-h" ) ) { usage(); }
//         char delimiter = options.value( "--delimiter,-d", ',' );
//         std::vector< std::string > unnamed = options.unnamed( "", "--delimiter,-d" );
//         boost::ptr_vector< std::istream > files;
//         std::vector< std::pair< std::istream*, std::size_t > > sources;
//         bool binary = false;
//         for( unsigned int i = 0; i < unnamed.size(); ++i )
//         {
//             std::string filename = unnamed[i];
//             std::size_t size = 0;
//             std::vector< std::string > v = comma::split( unnamed[i], ';' );
//             filename = v[0];
//             for( std::size_t j = 1; j < v.size(); ++j )
//             {
//                 std::vector< std::string > w = comma::split( v[j], '=' );
//                 if( w.size() != 2 ) { COMMA_THROW( comma::exception, "expected filename and options, got \"" << unnamed[i] << "\"" ); }
//                 if( w[0] == "binary" )
//                 {
//                     if( i == 0 ) { binary = true; }
//                     else if( !binary ) { COMMA_THROW( comma::exception, unnamed[0] << " is ascii, but " << filename << " is binary" ); }
//                     size = comma::csv::format( w[1] ).size();
//                 }
//                 else if( w[0] == "size" )
//                 {
//                     if( i == 0 ) { binary = true; }
//                     else if( !binary ) { COMMA_THROW( comma::exception, unnamed[0] << " is ascii, but " << filename << " is binary" ); }
//                     size = boost::lexical_cast< std::size_t >( w[1] );
//                 }
//             }
//             if( binary && size == 0 ) { COMMA_THROW( comma::exception, "in binary mode, please specify size or format for \"" << filename << "\"" ); }
//             if( filename == "-" )
//             {
//                 sources.push_back( std::make_pair( &std::cin, size ) );
//             }
//             else
//             {
//                 files.push_back( new std::ifstream( filename.c_str() ) );
//                 if( !files.back().good() || files.back().eof() ) { COMMA_THROW( comma::exception, "failed to open " << unnamed[i] ); }
//                 sources.push_back( std::make_pair( &files.back(), size ) );
//             }
//         }
//         if( sources.empty() ) { usage(); }
//         #ifdef WIN32
//         if( binary ) { _setmode( _fileno( stdin ), _O_BINARY ); }
//         #endif
//         show_usage = false;
//         if( binary )
//         {
//             std::size_t size = 0;
//             for( unsigned int i = 0; i < sources.size(); ++i ) { size += sources[i].second; }
//             while( true )
//             {
//                 for( unsigned int i = 0; i < sources.size(); ++i )
//                 {
//                     std::string s( sources[i].second, 0 );
//                     char* buf = &s[0];
//                     sources[i].first->read( buf, sources[i].second );
//                     int count = sources[i].first->gcount();
//                     if( count != 0 && (unsigned int)count != sources[i].second ) { COMMA_THROW( comma::exception, unnamed[i] << ": expected " << sources[i].second << " bytes, got " << count ); }
//                     if( !sources[i].first->good() || sources[i].first->eof() )
//                     {
//                         bool ok = true;
//                         for( unsigned int j = 0; j < sources.size() && ok; ++j )
//                         {
//                             if( j > i ) { sources[j].first->peek(); }
//                             ok = !sources[j].first->good() || sources[j].first->eof();
//                         }
//                         if( ok ) { return 0; }
//                         else { COMMA_THROW( comma::exception, unnamed[i] << ": unexpected end of file" ); }
//                     }
//                     std::cout << s;
//                 }
//             }
//         }
//         else
//         {
//             while( true )
//             {
//                 bool first = true;
//                 for( unsigned int i = 0; i < sources.size(); ++i )
//                 {
//                     std::string s;
//                     std::getline( *sources[i].first, s );
//                     if( !sources[i].first->good() || sources[i].first->eof() )
//                     {
//                         bool ok = true;
//                         for( unsigned int j = 0; j < sources.size() && ok; ++j )
//                         {
//                             if( j > i ) { sources[j].first->peek(); }
//                             ok = !sources[j].first->good() || sources[j].first->eof();
//                         }
//                         if( ok ) { return 0; }
//                         else { COMMA_THROW( comma::exception, unnamed[i] << ": unexpected end of file" ); }
//                     }
//                     if( !s.empty() && *s.rbegin() == '\r' ) { s = s.substr( 0, s.length() - 1 ); } // windows... sigh...
//                     if( s.empty() ) { continue; }
//                     if( !first ) { std::cout << delimiter; } else { first = false; }
//                     std::cout << s;
//                 }
//                 std::cout << std::endl;
//             }
//         }
//     }
//     catch( std::exception& ex )
//     {
//         std::cerr << "csv-paste: " << ex.what() << std::endl;
//     }
//     catch( ... )
//     {
//         std::cerr << "csv-paste: unknown exception" << std::endl;
//     }
//     if( show_usage ) { usage(); }
// }
