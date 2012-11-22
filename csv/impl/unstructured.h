// This file is part of comma, a generic and flexible library 
// for robotics research.
//
// Copyright (C) 2011 The University of Sydney
//
// comma is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// comma is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License 
// for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with comma. If not, see <http://www.gnu.org/licenses/>.

/// @author vsevolod vlaskine

#ifndef COMMA_CSV_IMPL_UNSTRUCTURED_H_
#define COMMA_CSV_IMPL_UNSTRUCTURED_H_

#include <string>
#include <vector>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/functional/hash.hpp>
#include <comma/csv/options.h>
#include <comma/string/string.h>
#include <comma/visiting/traits.h>

namespace comma { namespace csv { namespace impl {

/// flat unstructured csv representation (see csv-join and alike for usage)
template < typename T >
struct unstructured_values : public std::vector< T >
{
    struct hash : public std::unary_function< input, std::size_t >
    {
        std::size_t operator()( unstructured_values const& p ) const
        {
            static std::size_t seed = 0;
            for( std::size_t i = 0; i < p.size(); ++i ) { boost::hash_combine( seed, p[i] ); }
            return seed;
        }
    };
    
    bool operator==( const unstructured_values& rhs ) const
    {
        for( std::size_t i = 0; i < this->size(); ++i ) { if( this->operator[](i) != rhs[i] ) { return false; } }
        return true;
    }
    
    bool operator<( const unstructured_values& rhs ) const
    {
        for( std::size_t i = 0; i < this->size(); ++i ) { if( !( this->operator[](i) < rhs[i] ) ) { return false; } }
        return true;
    }
};

/// flat unstructured csv representation for all possible values (quick and dirty)
/// (see csv-join and alike for usage)
struct unstructured
{
    unstructured_values< comma::uint64 > integers;
    unstructured_values< double > doubles;
    unstructured_values< boost::posix_time::ptime > timestamps;
    unstructured_values< std::string > strings;
    
    static std::pair< unstructured, comma::csv::options > make( const comma::csv::options& csv, const std::string& sample = "" )
    {
        std::pair< unstructured, comma::csv::options > p;
        p.second.full_xpath = false;
        p.second.format( guess_format( csv, sample ).string() );
        std::vector< std::string > v = comma::split( p.second.fields );
        for( std::size_t i = 0; i < v.size(); ++i )
        {
            if( v[i].empty() ) { continue; }
            switch( o.format().offset( i ).type )
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
                    // todo: set v[i]
                    // todo: reserve
                    break;
                case comma::csv::format::float_t:
                case comma::csv::format::double_t:
                    // todo: set v[i]
                    // todo: reserve
                    break;
                case comma::csv::format::time:
                case comma::csv::format::long_time:
                    // todo: set v[i]
                    // todo: reserve
                    break;
                case comma::csv::format::fixed_string:
                    // todo: set v[i]
                    // todo: reserve
                    break;
            }
        } 
        p.second.fields = comma::join( v[i], ',' );
    }
        
    static comma::csv::format guess_format( const comma::csv::options& options, const std::string& sample )
    {
        if( csv.binary() ) { return csv.format(); }
        
        // todo
        
        return comma::csv::format();
    }
    
    bool operator==( const unstructured& rhs ) const
    {
        return integers == rhs.integers && doubles == rhs.doubles && timestamps == rhs.timestamps && strings == rhs.strings;        
    }
    
    bool operator<( const unstructured& rhs ) const
    {
        return integers < rhs.integers && doubles < rhs.doubles && timestamps < rhs.timestamps && strings < rhs.strings;        
    }
    
    struct hash : public std::unary_function< input, std::size_t >
    {
        std::size_t operator()( unstructured const& p ) const
        {
            static std::size_t seed = 0;
            boost::hash_combine( seed, unstructured_values< comma::uint64 >::hash( p.integers ) );
            boost::hash_combine( seed, unstructured_values< double >::hash( p.doubles ) );
            boost::hash_combine( seed, unstructured_values< boost::posix_time::ptime >::hash( p.timestamps ) );
            boost::hash_combine( seed, unstructured_values< string >::hash( p.strings ) );
            return seed;
        }
    };    
};

} } } // namespace comma { namespace csv { namespace impl {
    
namespace comma { namespace visiting {

template <> struct traits< comma::csv::impl::unstructured >
{
    template < typename K, typename V > static void visit( const K&, const comma::csv::impl::unstructured& p, V& v )
    { 
        v.apply( "i", static_cast< const std::vector< comma::uint64 >& >( p.integers ) );
        v.apply( "d", static_cast< const std::vector< double >& >( p.doubles ) );
        v.apply( "t", static_cast< const std::vector< boost::posix_time::ptime >& >( p.timestamps ) );
        v.apply( "s", static_cast< const std::vector< std::string >& >( p.strings ) );
    }

    template < typename K, typename V > static void visit( const K&, comma::csv::impl::unstructured& p, V& v )
    { 
        v.apply( "i", static_cast< std::vector< comma::uint64 >& >( p.integers ) );
        v.apply( "d", static_cast< std::vector< double >& >( p.doubles ) );
        v.apply( "t", static_cast< std::vector< boost::posix_time::ptime >& >( p.timestamps ) );
        v.apply( "s", static_cast< std::vector< std::string >& >( p.strings ) );
    }
};

} } // namespace comma { namespace visiting {

#endif // COMMA_CSV_IMPL_UNSTRUCTURED_H_
