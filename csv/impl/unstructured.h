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

#pragma once

#include <string>
#include <vector>
#include <boost/static_assert.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include "../../csv/options.h"
#include "../../math/compare.h"
#include "../../string/string.h"
#include "../../visiting/traits.h"

namespace comma { namespace csv { namespace impl {

/// flat unstructured csv representation for all possible values (quick and dirty)
/// todo: use in csv-calc, csv-join, etc
struct unstructured
{
    /// flat unstructured csv representation
    /// todo: use in csv-calc, csv-join, etc
    template < typename T >
    class values : public std::vector< T >
    {
        public:
            bool operator==( const values& rhs ) const
            {
                for( std::size_t i = 0; i < this->size(); ++i ) { if( !math::equal( this->operator[](i), rhs[i] ) ) { return false; } }
                return true;
            }
            
            bool operator<( const values& rhs ) const
            {
                for( std::size_t i = 0; i < this->size(); ++i ) { if( !math::less( this->operator[](i), rhs[i] ) ) { return false; } }
                return true;
            }
            
            struct hash : public std::unary_function< values, std::size_t >
            {
                std::size_t operator()( values const& p ) const
                {
                    std::size_t seed = 0;
                    for( std::size_t i = 0; i < p.size(); ++i ) { hash_combine_impl_( seed, p[i] ); }
                    return seed;
                }
            };
            
        private:
            static void hash_combine_impl_( std::size_t& s, const boost::posix_time::ptime& t ) // quick and dirty
            {
                BOOST_STATIC_ASSERT( sizeof( boost::posix_time::ptime ) == 8 ); // quick and dirty
                boost::hash_combine( s, reinterpret_cast< const comma::uint64& >( t ) );
            }
            
            template < typename S >
            static void hash_combine_impl_( std::size_t& s, const S& t ) { boost::hash_combine( s, t ); }
    };
    
    values< comma::int64 > longs;
    values< double > doubles;
    values< boost::posix_time::ptime > time;
    values< std::string > strings;
    
    bool empty() const { return longs.empty() && doubles.empty() && time.empty() && strings.empty(); }
    
    std::string append( csv::format::types_enum type )
    {
        switch( type )
        {
            case comma::csv::format::char_t:
            case comma::csv::format::int8:
            case comma::csv::format::uint8:
            case comma::csv::format::int16:
            case comma::csv::format::uint16:
            case comma::csv::format::int32:
            case comma::csv::format::uint32:
            case comma::csv::format::int64:
                longs.resize( longs.size() + 1 );
                longs.back() = 0;
                return "l[" + boost::lexical_cast< std::string >( longs.size() - 1 ) + "]";
            case comma::csv::format::uint64:
                COMMA_THROW( comma::exception, "unsigned 64-bit longs not supported (todo)" );
            case comma::csv::format::float_t:
            case comma::csv::format::double_t:
                doubles.resize( doubles.size() + 1 );
                doubles.back() = 0;
                return "d[" + boost::lexical_cast< std::string >( doubles.size() - 1 ) + "]";
            case comma::csv::format::time:
            case comma::csv::format::long_time:
                time.resize( time.size() + 1 );
                return "t[" + boost::lexical_cast< std::string >( time.size() - 1 ) + "]";
            case comma::csv::format::fixed_string:
                strings.resize( strings.size() + 1 );
                return "s[" + boost::lexical_cast< std::string >( strings.size() - 1 ) + "]";
        }
        COMMA_THROW( comma::exception, "never here" );
    }
    
    std::vector< std::string > resize( const std::string& format, bool append_fields = false )
    {
        return resize( csv::format( format ), append_fields );
    }
    
    std::vector< std::string > resize( const csv::format& f, bool append_fields = false )
    {
        if( !append_fields )
        {
            longs.clear();
            doubles.clear();
            time.clear();
            strings.clear();
        }
        unsigned int count = f.count();
        std::vector< std::string > fields( count );
        for( unsigned int i = 0; i < f.count(); fields[i] = append( f.offset( i ).type ), ++i );
        return fields;
    }
    
    static std::pair< unstructured, comma::csv::options > make( const comma::csv::options& csv, const std::string& sample = "" )
    {
        std::pair< unstructured, comma::csv::options > p;
        p.second = csv;
        p.second.full_xpath = false;
        const comma::csv::format& format = csv.binary() ? csv.format() : guess_format( sample, csv.delimiter );
        std::vector< std::string > v = comma::split( p.second.fields, csv.delimiter );
        for( std::size_t i = 0; i < v.size(); ++i )
        {
            if( !v[i].empty() ) { v[i] = p.first.append( format.offset( i ).type ); }
        } 
        p.second.fields = comma::join( v, ',' );
        return p;
    }
        
    static comma::csv::format guess_format( const std::string& sample, char delimiter = ',' )
    {
        comma::csv::format f;
        std::vector< std::string > v = comma::split( sample, delimiter );
        for( std::size_t i = 0; i < v.size(); ++i )
        {
            try
            { 
                if( v[i] != "not-a-date-time" ) { boost::posix_time::from_iso_string( v[i] ); }
                f += "t";
            }
            catch( ... )
            { 
                try
                {
                    boost::lexical_cast< comma::int64 >( v[i] );
                    f += "l";
                }
                catch( ... )
                {
                    try
                    {
                        boost::lexical_cast< double >( v[i] );
                        f += "d";
                    }
                    catch ( ... ) // way quick and dirty
                    {
                        f += "s[1024]";
                    }
                }
            }
        }
        return f;
    }
    
    bool operator==( const unstructured& rhs ) const
    {
        return longs == rhs.longs && doubles == rhs.doubles && time == rhs.time && strings == rhs.strings;        
    }
    
    bool operator<( const unstructured& rhs ) const
    {
        return doubles < rhs.doubles && time < rhs.time && strings < rhs.strings;        
    }
    
    template < typename T > values< T >& get();
    
    template < typename T > const values< T >& get() const;
        
    struct hash : public std::unary_function< unstructured, std::size_t >
    {
        std::size_t operator()( unstructured const& p ) const
        {
            if( !p.doubles.empty() ) { COMMA_THROW( comma::exception, "doubles are not supported as keys, got non-empty doubles" ); }
            std::size_t seed = 0;
            boost::hash_combine( seed, values< comma::int64 >::hash()( p.longs ) );
            //boost::hash_combine( seed, values< double >::hash()( p.doubles ) );
            boost::hash_combine( seed, values< boost::posix_time::ptime >::hash()( p.time ) );
            boost::hash_combine( seed, values< std::string >::hash()( p.strings ) );
            return seed;
        }
    };    
};

template <> inline unstructured::values< comma::int64 >& unstructured::get< comma::int64 >() { return longs; }
template <> inline const unstructured::values< comma::int64 >& unstructured::get< comma::int64 >() const { return longs; }
template <> inline unstructured::values< double >& unstructured::get< double >() { return doubles; }
template <> inline const unstructured::values< double >& unstructured::get< double >() const { return doubles; }
template <> inline unstructured::values< std::string >& unstructured::get< std::string >() { return strings; }
template <> inline const unstructured::values< std::string >& unstructured::get< std::string >() const { return strings; }
template <> inline unstructured::values< boost::posix_time::ptime >& unstructured::get< boost::posix_time::ptime >() { return time; }
template <> inline const unstructured::values< boost::posix_time::ptime >& unstructured::get< boost::posix_time::ptime >() const { return time; }

} } } // namespace comma { namespace csv { namespace impl {
    
namespace comma { namespace visiting {

template <> struct traits< comma::csv::impl::unstructured >
{
    template < typename K, typename V > static void visit( const K&, const comma::csv::impl::unstructured& p, V& v )
    { 
        v.apply( "l", static_cast< const std::vector< comma::int64 >& >( p.longs ) );
        v.apply( "d", static_cast< const std::vector< double >& >( p.doubles ) );
        v.apply( "t", static_cast< const std::vector< boost::posix_time::ptime >& >( p.time ) );
        v.apply( "s", static_cast< const std::vector< std::string >& >( p.strings ) );
    }

    template < typename K, typename V > static void visit( const K&, comma::csv::impl::unstructured& p, V& v )
    { 
        v.apply( "l", static_cast< std::vector< comma::int64 >& >( p.longs ) );
        v.apply( "d", static_cast< std::vector< double >& >( p.doubles ) );
        v.apply( "t", static_cast< std::vector< boost::posix_time::ptime >& >( p.time ) );
        v.apply( "s", static_cast< std::vector< std::string >& >( p.strings ) );
    }
};

} } // namespace comma { namespace visiting {
