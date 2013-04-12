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

#ifndef COMMA_CSV_SPLIT_H
#define COMMA_CSV_SPLIT_H

#include <fstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <comma/base/types.h>
#include <comma/csv/ascii.h>
#include <comma/csv/binary.h>
#include <comma/visiting/traits.h>

namespace comma { namespace csv { namespace applications {

struct input
{
    boost::posix_time::ptime timestamp;
    unsigned int block;
    unsigned int id;
}; 

} } } // namespace comma { namespace csv { namespace applications {

namespace comma { namespace visiting {

template <> struct traits< comma::csv::applications::input >
{
    template < typename K, typename V > static void visit( const K&, const comma::csv::applications::input& p, V& v )
    { 
        v.apply( "t", p.timestamp );
        v.apply( "block", p.block );
        v.apply( "id", p.id );
    }
    
    template < typename K, typename V > static void visit( const K&, comma::csv::applications::input& p, V& v )
    { 
        v.apply( "t", p.timestamp );
        v.apply( "block", p.block );
        v.apply( "id", p.id );
    }
};

} } // namespace comma { namespace visiting {    

namespace comma { namespace csv { namespace applications {
    
/// split data to files by time
/// files are named by timestamp, cut down to seconds
class split
{
    public:
        split( boost::optional< boost::posix_time::time_duration > period
             , const std::string& suffix
             , const comma::csv::options& csv );
        void write( const char* data, unsigned int size );
        void write( const std::string& line );

    private:
        std::ofstream& ofstream_by_time_();
        std::ofstream& ofstream_by_block_();
        std::ofstream& ofstream_by_id_();
        void update_( const char* data, unsigned int size );
        void update_( const std::string& line );
        
        boost::function< std::ofstream&() > ofstream_;
        boost::scoped_ptr< comma::csv::ascii< input > > ascii_;
        boost::scoped_ptr< comma::csv::binary< input > > binary_;
        boost::optional< boost::posix_time::time_duration > period_;
        std::string suffix_;
        input current_;
        boost::optional< input > last_;
        std::ios_base::openmode mode_;
        std::ofstream file_;
        typedef boost::unordered_map< comma::uint32, boost::shared_ptr< std::ofstream > > Files;
        typedef boost::unordered_set< comma::uint32 > ids_type_;
        Files files_;
        ids_type_ seen_ids_;
};

} } } // namespace comma { namespace csv { namespace applications {
    
#endif // COMMA_CSV_SPLIT_H
