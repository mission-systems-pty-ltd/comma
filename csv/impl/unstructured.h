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
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by the The University of Sydney.
// 4. Neither the name of the The University of Sydney nor the
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

#ifndef COMMA_CSV_IMPL_UNSTRUCTURED_H_
#define COMMA_CSV_IMPL_UNSTRUCTURED_H_

#include <string>
#include <vector>
#include <boost/static_assert.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <comma/csv/options.h>
#include <comma/math/compare.h>
#include <comma/string/string.h>
#include <comma/visiting/traits.h>

namespace comma { namespace csv { namespace impl {

/// flat unstructured csv representation
/// todo: use in csv-calc, csv-join, etc
template < typename T >
class unstructured_values : public std::vector< T >
{
    public:
        bool operator==( const unstructured_values& rhs ) const
        {
            for( std::size_t i = 0; i < this->size(); ++i ) { if( !math::equal( this->operator[](i), rhs[i] ) ) { return false; } }
            return true;
        }
        
        bool operator<( const unstructured_values& rhs ) const
        {
            for( std::size_t i = 0; i < this->size(); ++i ) { if( !math::less( this->operator[](i), rhs[i] ) ) { return false; } }
            return true;
        }
        
        struct hash : public std::unary_function< unstructured_values, std::size_t >
        {
            std::size_t operator()( unstructured_values const& p ) const
            {
                static std::size_t seed = 0;
                for( std::size_t i = 0; i < p.size(); ++i ) { hash_combine_impl_( seed, p[i] ); }
                return seed;
            }
        };
        
    private:
        static void hash_combine_impl_( std::size_t& s, const boost::posix_time::ptime& t )
        {
            BOOST_STATIC_ASSERT( sizeof( boost::posix_time::ptime ) == 8 ); // quick and dirty
            boost::hash_combine( s, reinterpret_cast< const comma::uint64& >( t ) );
        }
        
        template < typename S >
        static void hash_combine_impl_( std::size_t& s, const S& t ) { boost::hash_combine( s, t ); }
};

/// flat unstructured csv representation for all possible values (quick and dirty)
/// todo: use in csv-calc, csv-join, etc
struct unstructured
{
    //unstructured_values< comma::uint64 > integers;
    unstructured_values< double > doubles;
    unstructured_values< boost::posix_time::ptime > timestamps;
    unstructured_values< std::string > strings;
    
    static std::pair< unstructured, comma::csv::options > make( const comma::csv::options& csv, const std::string& sample = "" )
    {
        std::pair< unstructured, comma::csv::options > p;
        p.second = csv;
        p.second.full_xpath = false;
        const comma::csv::format& format = csv.binary() ? csv.format() : guess_format( sample, csv.delimiter );
        std::vector< std::string > v = comma::split( p.second.fields, csv.delimiter );
        for( std::size_t i = 0; i < v.size(); ++i )
        {
            if( v[i].empty() ) { continue; }
            switch( format.offset( i ).type )
            {
                case comma::csv::format::char_t:
                case comma::csv::format::int8:
                case comma::csv::format::uint8:
                case comma::csv::format::int16:
                case comma::csv::format::uint16:
                case comma::csv::format::int32:
                case comma::csv::format::uint32:
                case comma::csv::format::int64:
                case comma::csv::format::uint64:
                    //v[i] = "i[" + boost::lexical_cast< std::string >( integers.size() ) + "]";
                    //p.first.integers.resize( integers.size() + 1 );
                    //break;
                case comma::csv::format::float_t:
                case comma::csv::format::double_t:
                    v[i] = "d[" + boost::lexical_cast< std::string >( p.first.doubles.size() ) + "]";
                    p.first.doubles.resize( p.first.doubles.size() + 1 );
                    p.first.doubles.back() = 0;
                    break;
                case comma::csv::format::time:
                case comma::csv::format::long_time:
                    v[i] = "t[" + boost::lexical_cast< std::string >( p.first.timestamps.size() ) + "]";
                    p.first.timestamps.resize( p.first.timestamps.size() + 1 );
                    break;
                case comma::csv::format::fixed_string:
                    v[i] = "s[" + boost::lexical_cast< std::string >( p.first.strings.size() ) + "]";
                    p.first.strings.resize( p.first.strings.size() + 1 );
                    break;
            }
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
                boost::posix_time::from_iso_string( v[i] );
                f += "t";
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
        return f;
    }
    
    bool operator==( const unstructured& rhs ) const
    {
        //return integers == rhs.integers && doubles == rhs.doubles && timestamps == rhs.timestamps && strings == rhs.strings;
        return doubles == rhs.doubles && timestamps == rhs.timestamps && strings == rhs.strings;        
    }
    
    bool operator<( const unstructured& rhs ) const
    {
        //return integers < rhs.integers && doubles < rhs.doubles && timestamps < rhs.timestamps && strings < rhs.strings;
        return doubles < rhs.doubles && timestamps < rhs.timestamps && strings < rhs.strings;        
    }
    
    template < typename T > unstructured_values< T >& get();
    
    template < typename T > const unstructured_values< T >& get() const;
        
    struct hash : public std::unary_function< unstructured, std::size_t >
    {
        std::size_t operator()( unstructured const& p ) const
        {
            static std::size_t seed = 0;
            //boost::hash_combine( seed, unstructured_values< comma::uint64 >::hash( p.integers ) );
            boost::hash_combine( seed, unstructured_values< double >::hash()( p.doubles ) );
            boost::hash_combine( seed, unstructured_values< boost::posix_time::ptime >::hash()( p.timestamps ) );
            boost::hash_combine( seed, unstructured_values< std::string >::hash()( p.strings ) );
            return seed;
        }
    };    
};

template <> inline unstructured_values< double >& unstructured::get< double >() { return doubles; }
template <> inline const unstructured_values< double >& unstructured::get< double >() const { return doubles; }
template <> inline unstructured_values< std::string >& unstructured::get< std::string >() { return strings; }
template <> inline const unstructured_values< std::string >& unstructured::get< std::string >() const { return strings; }
template <> inline unstructured_values< boost::posix_time::ptime >& unstructured::get< boost::posix_time::ptime >() { return timestamps; }
template <> inline const unstructured_values< boost::posix_time::ptime >& unstructured::get< boost::posix_time::ptime >() const { return timestamps; }

} } } // namespace comma { namespace csv { namespace impl {
    
namespace comma { namespace visiting {

template <> struct traits< comma::csv::impl::unstructured >
{
    template < typename K, typename V > static void visit( const K&, const comma::csv::impl::unstructured& p, V& v )
    { 
        //v.apply( "i", static_cast< const std::vector< comma::uint64 >& >( p.integers ) );
        v.apply( "d", static_cast< const std::vector< double >& >( p.doubles ) );
        v.apply( "t", static_cast< const std::vector< boost::posix_time::ptime >& >( p.timestamps ) );
        v.apply( "s", static_cast< const std::vector< std::string >& >( p.strings ) );
    }

    template < typename K, typename V > static void visit( const K&, comma::csv::impl::unstructured& p, V& v )
    { 
        //v.apply( "i", static_cast< std::vector< comma::uint64 >& >( p.integers ) );
        v.apply( "d", static_cast< std::vector< double >& >( p.doubles ) );
        v.apply( "t", static_cast< std::vector< boost::posix_time::ptime >& >( p.timestamps ) );
        v.apply( "s", static_cast< std::vector< std::string >& >( p.strings ) );
    }
};

} } // namespace comma { namespace visiting {

#endif // COMMA_CSV_IMPL_UNSTRUCTURED_H_
