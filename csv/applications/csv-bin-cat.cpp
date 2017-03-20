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
        std::cerr << "    csv-bin-cat file.bin [file2.bin ...] --binary=<format> --fields=<fields> --output-fields=<output-fields>" << std::endl;
        std::cerr << std::endl;
        std::cerr << "    output the specified fields from the binary file(s)" << std::endl;
        std::cerr << "    produce the same output as the more universal csv-shuffle call" << std::endl;
        std::cerr << "        cat file.bin | csv-shuffle --binary=<format> --fields=<fields> --output-fields=<output-fields>" << std::endl;
        std::cerr << "    but more efficient; MUCH more efficient for large data is only a small sub-set has to be output" << std::endl;
        std::cerr << "    TODO: support the rules for output of trailing fields as in csv-shuffle" << std::endl;
        std::cerr << std::endl;
        std::cerr << "options:" << std::endl;
        std::cerr << "    --help,-h: help; --help --verbose: more help" << std::endl;
        std::cerr << "    --fields,-f,--input-fields <fields>: input fields" << std::endl;
        std::cerr << "    --binary,--format <format>: input binary format" << std::endl;
        std::cerr << "    --output-fields,--output,-o <fields>: output fields" << std::endl;
        std::cerr << "    --skip=<N>; skip the first N records (applied once, if multiple files are given; no skip if N = 0)" << std::endl;
        std::cerr << "    --count=<N>; output no more than N records; no output if N = 0" << std::endl;
        std::cerr << "    --flush; flush after every output record" << std::endl;
        std::cerr << "    --verbose,-v: more output" << std::endl;
        std::cerr << std::endl;
        std::cerr << "examples:" << std::endl;
        std::cerr << "    csv-bin-cat input.bin --binary=t,s[100000] --fields=t,s --output-fields=t" << std::endl;
        std::cerr << "        output only the time field from a binary file; much more efficient form of" << std::endl;
        std::cerr << "            cat input.bin | csv-shuffle --binary=t,s[100000] --fields=t,s --output-fields=t" << std::endl;
        std::cerr << "    csv-bin-cat input.bin --skip=1000 --binary=t,s[100000] --fields=t,s --output-fields=t | csv-from-bin t" << std::endl;
        std::cerr << "        output only the time field starting with 1000th record; much more efficient form of" << std::endl;
        std::cerr << "            cat input.bin | csv-shuffle --binary=t,s[100000] --fields=t,s --output-fields=t | csv-from-bin t | tail -n+1001" << std::endl;
        std::cerr << "    csv-bin-cat 1.bin 2.bin 3.bin --skip=1000 --head=2 --binary=t,s[100000] --fields=t,s ..." << std::endl;
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

} // anonymous

int main( int ac, char** av )
{
    #ifdef WIN32
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
        if( !csv.binary() ) { std::cerr << "csv-bin-cat: must provide '--binary=..' format" << std::endl; return 1; }

        std::vector< std::string > files = options.unnamed( "--help,-h,--verbose,-v,--flush", "--fields,-f,--input-fields,--output-fields,-o,--binary,--format,--skip,--count" );
        if( files.size() < 1 ) { std::cerr << "csv-bin-cat: expected at least one file name" << std::endl; return 1; }

        std::vector< std::string > input_fields = comma::split( csv.fields, ',' );
        std::vector< std::string > output_fields = comma::split( options.value< std::string >( "--output-fields,--output,-o" ), ',' );
        // TODO:
        // bool output_trailing_fields = output_fields.back() == "...";
        // if( output_fields.back() == "..." ) { output_fields.erase( output_fields.end() - 1 ); }
        std::vector< field > fields;
        for( unsigned int i = 0; i < output_fields.size(); ++i )
        {
            if( output_fields[i].empty() ) { continue; }
            fields.push_back( field( output_fields[i], i ) );
        }
        if( fields.empty() ) { std::cerr << "csv-bin-cat: please define at least one output field" << std::endl; return 1; }
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
        size_t orecord_size = 0;
        for( unsigned int i = 0; i < fields.size(); ++i )
        {
            if( !fields[i].input_index ) { std::cerr << "csv-bin-cat: \"" << fields[i].name << "\" not found in input fields " << csv.fields << std::endl; return 1; }
            fields[i].offset = ( i == 0 ? 0 : fields[i - 1].offset + fields[i - 1].size );
            orecord_size += fields[i].size;
        }
        unsigned int skip = options.value< unsigned int >( "--skip", 0 );
        long int count_max = options.value< long int >( "--count", -1 );
        bool flush = options.exists( "--flush" );

        // algorithm summary:
        // - seekg to the beginning of record
        // - iterate over fields by doing seekg to the beginning of each field, and read into buffer
        // - check for read errors / end of input
        // - write the buffer to stdout
        std::vector< char > buf( orecord_size );
        long int count = 0;
        for ( std::vector< std::string >::const_iterator ifile = files.begin(); ifile < files.end(); ++ifile ) {
            std::ifstream ifs( ifile->c_str(), std::ifstream::binary );
            if ( !ifs.is_open() ) { std::cerr << "csv-bin-cat: cannot open '" << *ifile << "' for reading" << std::endl; return 1; }
            std::streampos record_start = 0;
            if ( skip ) {
                ifs.seekg( 0, std::ios_base::end );
                std::streampos fsize = ifs.tellg();
                unsigned int nrecords = fsize / csv.format().size();
                if ( nrecords * csv.format().size() != fsize ) { std::cerr << "csv-bin-cat: size of file '" << *ifile << "' is not a multiple of the record size" << std::endl; return 1; }
                if ( nrecords < skip ) { skip -= nrecords; continue; }
                record_start = csv.format().size() * skip;
                ifs.seekg( record_start, std::ios_base::beg );
                if ( ifs.fail() ) { std::cerr << "csv-bin-cat: cannot skip " << skip << " records in the file '" << *ifile << "'" << std::endl; return 1; }
            }
            while( ifs.good() && !ifs.eof() )
            {
                for( unsigned int i = 0; i < fields.size(); ++i )
                {
                    std::streamoff off = fields[i].input_offset - ( i == 0 ? 0 : fields[i - 1].input_offset + fields[i - 1].size );
                    ifs.seekg( off, std::ios_base::cur );
                    ifs.read( &buf[ fields[i].offset ], fields[i].size );
                    if ( ifs.eof() ) {
                        if ( i == 0 ) { break; }
                        std::cerr << "csv-bin-cat: encountered eof mid-record in '" << *ifile << "'" << std::endl; return 1;
                    }
                    if ( ifs.fail() ) { std::cerr << "csv-bin-cat: reading '" << fields[i].name << "' at position " << record_start + off << " failed" << std::endl; return 1; }
                }
                if ( !ifs.eof() )
                {
                    std::cout.write( &buf[0], orecord_size );
                    if (flush) { std::cout.flush(); }
                    if ( std::cout.fail() ) { std::cerr << "csv-bin-cat: std::cout output failed" << std::endl; return 1; }
                    ++count;
                    if ( count_max >= 0 && count >= count_max ) { return 0; }
                    // go to the start of the next record
                    record_start += csv.format().size();
                    ifs.seekg( record_start, std::ios_base::beg );
                }
            }
        }
#if 0
            std::vector< char > buf( csv.format().size() );
            std::vector< comma::csv::format::element > elements;
            elements.reserve( csv.format().count() ); // quick and dirty, can be really wasteful on large things like images
            for( unsigned int i = 0; i < elements.capacity(); ++i ) { elements.push_back( csv.format().offset( i ) ); }
            while( std::cin.good() && !std::cin.eof() )
            {
                // todo: quick and dirty; if performance is an issue, you could read more than
                // one record every time see comma::csv::binary_input_stream::read() for reference
                std::cin.read( &buf[0], csv.format().size() );
                if( std::cin.gcount() == 0 ) { continue; }
                if( std::cin.gcount() < int( csv.format().size() ) ) { std::cerr << "csv-shuffle: expected " << csv.format().size() << " bytes, got only " << std::cin.gcount() << std::endl; return 1; }
                unsigned int previous_index = 0;
                for( unsigned int i = 0; i < fields.size(); ++i ) // quick and dirty
                {
                    for( unsigned int k = previous_index; k < fields[i].index && k < elements.size(); ++k )
                    {
                        std::cout.write( &buf[ elements[k].offset ], elements[k].size );
                    }
                    std::cout.write( &buf[ fields[i].input_offset ], fields[i].size );
                    previous_index = fields[i].index + 1;
                }
                //std::cerr << "--> previous_index: " << previous_index << " elements.size(): " << elements.size() << std::endl;
                for( unsigned int k = previous_index; output_trailing_fields && k < elements.size(); ++k )
                {
                    std::cout.write( &buf[ elements[k].offset ], elements[k].size );
                }
                std::cout.flush(); // todo: flushing too often?
            }
#endif
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-from-bin: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-from-bin: unknown exception" << std::endl; }
    return 1;
}

