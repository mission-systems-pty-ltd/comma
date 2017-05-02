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


/// @author dmitry mikhin

#ifdef WIN32
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#endif

#include <stdlib.h>
#include <fstream>
#include <numeric>
#include "../../application/command_line_options.h"
#include "../../application/contact_info.h"
#include "../../base/exception.h"
#include "../../csv/format.h"
#include "../../csv/options.h"
#include "../../string/string.h"

using namespace comma;

namespace {

    void usage( bool verbose )
    {
        std::cerr << std::endl;
        std::cerr << "Usage:" << std::endl;
        std::cerr << "    csv-bin-cat file.bin [file2.bin ...] --binary=<format> --fields=<fields> --output-fields=<output-fields> [<options>]" << std::endl;
        std::cerr << "or" << std::endl;
        std::cerr << "    csv-bin-cat file.bin [file2.bin ...] --binary=<format> --fields=<numbers> [<options>]" << std::endl;
        std::cerr << "or" << std::endl;
        std::cerr << "    cat file.bin [file2.bin ...] | csv-bin-cat --binary=... --fields=... [<options>]" << std::endl;
        std::cerr << std::endl;
        std::cerr << "    The first form outputs the specified fields from the binary file(s); produce the same output as the csv-shuffle call" << std::endl;
        std::cerr << "        cat file.bin | csv-shuffle --binary=<format> --fields=<fields> --output-fields=<output-fields>" << std::endl;
        std::cerr << "    but can be MUCH more efficient for large data if only a small sub-set has to be output." << std::endl;
        std::cerr << std::endl;
        std::cerr << "    The second form specifies the output fields as numbers; it works like the UNIX cut (1) utility for \"binary csv\"." << std::endl;
        std::cerr << std::endl;
        std::cerr << "    The third form reads data from standard input and is similar to csv-shuffle. Can use either names or numbers for fields." << std::endl;
        std::cerr << std::endl;
        std::cerr << "    When csv-bin-cat reads files (as opposed to stdin), it seeks to the position of the next data to be output as opposed to" << std::endl;
        std::cerr << "    reading entire records; this improves performance for large record sizes as most of the input is skipped. For small records," << std::endl;
        std::cerr << "    however, the improvement is lost because seek-read-seek-read would repeatedly read the same disk block. Therefore, for small" << std::endl;
        std::cerr << "    records it is advised to turn off the seeking algorithm by the '--read-all' option." << std::endl;
        std::cerr << std::endl;
        std::cerr << "Semantics for input:" << std::endl;
        std::cerr << "    csv-bin-cat a.bin b.bin --binary=<format> ...: data from files, potentially fast" << std::endl;
        std::cerr << "    csv-bin-cat --binary=<format> ...: data from stdin" << std::endl;
        std::cerr << "    csv-bin-cat - --binary=<format> ...: also data from stdin" << std::endl;
        std::cerr << "    csv-bin-cat a.bin - b.bin --binary=<format> ...: data from a.bin (potentially fast), then from stdin (read full records," << std::endl;
        std::cerr << "        may be slow), then data from b.bin (again, potentially fast)" << std::endl;
        std::cerr << "    csv-bin-cat <format> ...: data from stdin" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Semantics for output:" << std::endl;
        std::cerr << "    csv-bin-cat ... --fields=foo,bar,baz --output-fields=foo,baz: input and output fields specified by names" << std::endl;
        std::cerr << "    csv-bin-cat ... --fields=1,3: fields to output are specified by numbers as in cut (1) utility; numbers start with 1" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "    --help,-h: help; --help --verbose: more help" << std::endl;
        std::cerr << "    --fields,-f <fields>: input field names if '--output-fields' are given; output field numbers if not" << std::endl;
        std::cerr << "    --binary,--format <format>: input binary format" << std::endl;
        std::cerr << "    --output-fields,--output,-o <fields>: output fields" << std::endl;
        std::cerr << "    --skip=<N>; skip the first N records (applied once if multiple input files are given); no skip if N = 0" << std::endl;
        std::cerr << "    --count=<N>; output no more than N records; no output if N <= 0" << std::endl;
        std::cerr << "    --read-all,--force-read; do not use the seek form of the algorithm, read entire records at once (see above)" << std::endl;
        std::cerr << "    --flush; flush after every output record; less efficient, more predictable" << std::endl;
        std::cerr << "    --verbose,-v: chat more" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Examples:" << std::endl;
        std::cerr << "    csv-bin-cat input.bin --binary=t,s[1000000] --fields=t,s --output-fields=t" << std::endl;
        std::cerr << "        output only the time field from a binary file; more efficient form of" << std::endl;
        std::cerr << "            cat input.bin | csv-shuffle --binary=t,s[1000000] --fields=t,s --output-fields=t" << std::endl;
        std::cerr << std::endl;
        std::cerr << "    csv-bin-cat input.bin --binary=t,s[1000000] --fields=1" << std::endl;
        std::cerr << "        same but uses positional number to specify output of the 1st field (time)" << std::endl;
        std::cerr << std::endl;
        std::cerr << "    cat input.bin | csv-bin-cat --binary=t,s[1000000] --fields=1" << std::endl;
        std::cerr << "        take input on stdin; same output as above but less efficient because has to read the whole file" << std::endl;
        std::cerr << std::endl;
        std::cerr << "    csv-bin-cat input.bin --binary=t,s[2000] --fields=1 --read-all" << std::endl;
        std::cerr << "        record size is small, force reading the entire record at once; thus, identical to" << std::endl;
        std::cerr << "            cat input.bin | csv-shuffle --binary=t,s[2000] --fields=t,s --output-fields=t" << std::endl;
        std::cerr << std::endl;
        std::cerr << "    csv-bin-cat input.bin --skip=1000 --binary=t,s[1000000] --fields=t,s --output-fields=t | csv-from-bin t" << std::endl;
        std::cerr << "        output only the time field starting with 1000th record; more efficient form of" << std::endl;
        std::cerr << "            cat input.bin | csv-shuffle --binary=t,s[1000000] --fields=t,s --output-fields=t | csv-from-bin t | tail -n+1001" << std::endl;
        std::cerr << std::endl;
        std::cerr << "    csv-bin-cat 1.bin 2.bin 3.bin --skip=1000 --count=2 --binary=t,s[1000000] --fields=t,s ..." << std::endl;
        std::cerr << "        assuming each of the binary files holds 500 records, output data from" << std::endl;
        std::cerr << "        records 0 and 1 of file '3.bin'; records 0-499 in the file '1.bin' skipped, records 0-499 in" << std::endl;
        std::cerr << "        the file '2.bin' skipped (total of 1000 skipped records), then 2 records output" << std::endl;
        std::cerr << std::endl;
        if ( verbose ) {
            std::cerr << "format specifications:" << std::endl;
            std::cerr << csv::format::usage() << std::endl;
        }
        std::cerr << comma::contact_info << std::endl;
        std::cerr << std::endl;
        exit( 0 );
    }

    struct field
    {
        std::string name;
        unsigned int index;
        unsigned int offset;
        boost::optional< unsigned int > input_index;
        unsigned int input_offset;
        unsigned int size;
        field( const std::string& name, unsigned int index ) : name( name ), index( index ) {}
    };

    std::vector< field > setup_fields( const comma::command_line_options & options, const comma::csv::options & csv )
    {
        std::vector< std::string > input_fields = comma::split( csv.fields, ',' );
        std::vector< field > fields;
        if ( options.exists( "--output-fields,--output,-o" ) )
        {
            // input and output fields must be names
            std::vector< std::string > output_fields = comma::split( options.value< std::string >( "--output-fields,--output,-o" ), ',' );
            for( unsigned int i = 0; i < output_fields.size(); ++i )
            {
                if( output_fields[i].empty() ) { continue; }
                fields.push_back( field( output_fields[i], i ) );
            }
            for( unsigned int i = 0; i < input_fields.size(); ++i )
            {
                for( unsigned int j = 0; j < fields.size(); ++j )
                {
                    if( fields[j].name != input_fields[i] ) { continue; }
                    fields[j].input_index = i;
                    fields[j].input_offset = csv.format().offset( i ).offset;
                    fields[j].size = csv.format().offset( i ).size;
                }
            }
        } else {
            // input fields must be numbers
            for( unsigned int i = 0; i < input_fields.size(); ++i )
            {
                if ( input_fields[i].empty() ) { continue; }
                int pos = boost::lexical_cast< int >( input_fields[i] );
                if ( pos < 1 || pos > csv.format().count() ) { COMMA_THROW( comma::exception, "field number " << input_fields[i] << " out of [0," << csv.format().count() << "] bounds" ); }
                fields.push_back( field( boost::lexical_cast< std::string >( pos ), fields.size() ) );
                fields.back().input_index = --pos;
                fields.back().input_offset = csv.format().offset( pos ).offset;
                fields.back().size = csv.format().offset( pos ).size;
            }
        }
        if( fields.empty() ) { COMMA_THROW( comma::exception, "csv-bin-cat: please define at least one output field" ); }
        for( unsigned int i = 0; i < fields.size(); ++i )
        {
            if( !fields[i].input_index ) { COMMA_THROW( comma::exception, "csv-bin-cat: field '" << fields[i].name << "' not found in input fields '" << csv.fields << "'" ); }
            fields[i].offset = ( i == 0 ? 0 : fields[i - 1].offset + fields[i - 1].size );
        }
        return fields;
    }

    class seeker
    {
        public:
            seeker( const std::vector< field > & fields, const comma::csv::options & csv, unsigned int skip, long int count_max, bool flush, bool force_read )
                : fields_( fields )
                , orecord_size_( std::accumulate( fields.begin(), fields.end(), (size_t)0, []( size_t i, const field & f ){ return f.size + i; } ) )
                , obuf_( orecord_size_ )
                , ibuf_( 0 )
                , irecord_size_( csv.format().size() )
                , skip_( skip )
                , count_( 0 )
                , count_max_( count_max )
                , flush_( flush )
                , force_read_( force_read )
                {}

            int process( const std::vector< std::string > & files );

        private:
            int read_fields( std::ifstream & ifs, const std::string & fname );
            int read_all( std::istream & is );

            const std::vector< field > & fields_;
            size_t orecord_size_;
            std::vector< char > obuf_;
            std::vector< char > ibuf_;
            size_t irecord_size_;
            unsigned int skip_;
            long int count_;
            long int count_max_;
            bool flush_;
            bool force_read_;
    };

    int seeker::read_all( std::istream & is )
    {
        if ( count_max_ >= 0 && count_ >= count_max_ ) { return 0; }
        ibuf_.resize( irecord_size_ );
        while( is.good() && !is.eof() )
        {
            is.read( &ibuf_[0], irecord_size_ );
            if( is.gcount() == 0 ) { continue; }
            if( is.gcount() < int( irecord_size_ ) ) { COMMA_THROW( comma::exception, "csv-bin-cat: expected " << irecord_size_ << " bytes, got only " << is.gcount() ); }
            if ( skip_ ) { --skip_; continue; }
            for( unsigned int i = 0; i < fields_.size(); ++i ) { std::cout.write( &ibuf_[0] + fields_[i].input_offset, fields_[i].size ); }
            if( flush_ ) { std::cout.flush(); }
            if ( count_max_ >= 0 && ++count_ >= count_max_ ) { return 0; }
        }
        return 0;
    }

    int seeker::read_fields( std::ifstream & ifs, const std::string & fname )
    {
        // algorithm summary:
        // - seekg to the beginning of record
        // - iterate over fields by doing seekg to the beginning of each field, and read into buffer
        // - check for read errors / end of input
        // - write the buffer to stdout
        std::streampos record_start = 0;
        if ( skip_ ) {
            ifs.seekg( 0, std::ios_base::end );
            std::streampos fsize = ifs.tellg();
            unsigned int nrecords = fsize / irecord_size_;
            if ( nrecords * irecord_size_ != fsize ) { COMMA_THROW( comma::exception, "csv-bin-cat: size of file '" << fname << "' is not a multiple of the record size" ); }
            if ( nrecords < skip_ ) { skip_ -= nrecords; return 0; }
            record_start = irecord_size_ * skip_;
            ifs.seekg( record_start, std::ios_base::beg );
            if ( ifs.fail() ) { COMMA_THROW( comma::exception, "csv-bin-cat: cannot skip " << skip_ << " records in the file '" << fname << "'" ); }
            skip_ = 0;
        }
        while( ifs.good() && !ifs.eof() )
        {
            for( unsigned int i = 0; i < fields_.size(); ++i )
            {
                std::streamoff off = fields_[i].input_offset - ( i == 0 ? 0 : fields_[i - 1].input_offset + fields_[i - 1].size );
                ifs.seekg( off, std::ios_base::cur );
                ifs.read( &obuf_[ fields_[i].offset ], fields_[i].size );
                if ( ifs.eof() ) {
                    if ( i == 0 ) { break; }
                    COMMA_THROW( comma::exception, "csv-bin-cat: encountered eof mid-record in '" << fname << "'" );
                }
                if ( ifs.fail() ) { COMMA_THROW( comma::exception, "csv-bin-cat: reading field '" << fields_[i].name << "' at position " << record_start + off << " failed" ); }
            }
            if ( !ifs.eof() )
            {
                std::cout.write( &obuf_[0], orecord_size_ );
                if ( flush_ ) { std::cout.flush(); }
                if ( std::cout.fail() ) { COMMA_THROW( comma::exception, "csv-bin-cat: std::cout output failed" ); }
                if ( count_max_ >= 0 && ++count_ >= count_max_ ) { return 0; }  // count not incremented if no limit imposed, do not care
                // go to the start of the next record
                record_start += irecord_size_;
                unsigned int i = fields_.size() - 1;
                std::streamoff off = irecord_size_ - ( fields_[i].input_offset + fields_[i].size );
                ifs.seekg( off, std::ios_base::cur );
            }
        }
        return 0;
    }

    int seeker::process( const std::vector< std::string > & files )
    {
        if ( files.empty() ) { return read_all( std::cin ); }
        for ( std::vector< std::string >::const_iterator ifile = files.begin(); ifile < files.end(); ++ifile ) {
            if ( count_max_ >= 0 && count_ >= count_max_ ) { return 0; }
            if ( *ifile == "-" ) {
                int rv = read_all( std::cin );
                if ( rv != 0 ) { return rv; }
            } else {
                std::ifstream ifs( ifile->c_str(), std::ifstream::binary );
                if ( !ifs.is_open() ) { COMMA_THROW( comma::exception, "csv-bin-cat: cannot open '" << *ifile << "' for reading" ); }
                int rv = ( force_read_ ? read_all( ifs ) : read_fields( ifs, *ifile ) );
                if ( rv != 0 ) { return rv; }
            }
        }
        return 0;
    }

} // anonymous

int main( int ac, char** av )
{
    #ifdef WIN32
    _setmode( _fileno( stdin ), _O_BINARY ); /// @todo move to a library
    _setmode( _fileno( stdout ), _O_BINARY );
    #endif
    try
    {
        command_line_options options( ac, av );

        bool verbose = options.exists( "--verbose,-v" );
        if( options.exists( "--help,-h" ) ) { usage( verbose ); }

        comma::csv::options csv( options );
        {
            std::string b = options.value< std::string >( "--format", "" );
            if( !b.empty() ) { csv.format( b ); }
            std::string f = options.value< std::string >( "--input-fields", "" );
            if( !f.empty() ) { csv.fields = f; }
        }

        std::vector< std::string > files = options.unnamed( "--help,-h,--verbose,-v,--flush,--read-all,--force-read", "--fields,-f,--input-fields,--output-fields,-o,--binary,--format,--skip,--count" );
        if ( files.size() == 1 && files[0] != "-" ) {
            // could be format string, not a file
            try
            {
                comma::csv::format f( files[0] );
                csv.format( f );
                files.pop_back();
            }
            catch ( comma::exception & )
            {
                // OK, not a string
            }
        }
        if( !csv.binary() ) { std::cerr << "csv-bin-cat: must provide '--binary=..' format" << std::endl; return 1; }

        std::vector< field > fields = setup_fields( options, csv );

        unsigned int skip = options.value< unsigned int >( "--skip", 0 );
        long int count_max = options.value< long int >( "--count", -1 );
        bool flush = options.exists( "--flush" );
        bool force_read = options.exists( "--read-all,--force-read" );

        seeker seek( fields, csv, skip, count_max, flush, force_read );
        return seek.process( files );
    }
    catch( std::exception& ex ) { std::cerr << "csv-from-bin: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-from-bin: unknown exception" << std::endl; }
    return 1;
}

