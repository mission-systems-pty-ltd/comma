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

/// @author dewey nguyen

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <iostream>
#include <deque>
#include <comma/application/command_line_options.h>
#include <comma/application/contact_info.h>
#include <comma/base/types.h>
#include <comma/csv/stream.h>
#include <comma/csv/impl/unstructured.h>
#include <comma/string/string.h>
#include <comma/visiting/traits.h>

static const char* name() { return "csv-blocks: "; }

struct input_t
{
    comma::csv::impl::unstructured key;
    input_t()  {}
    input_t( comma::csv::impl::unstructured key ): key( key ) {}
};

struct input_with_block { comma::uint32 block; };
struct input_with_index { comma::uint32 index; };

struct appended_column
{
    comma::uint32 value;
    
    appended_column() : value(0) {}
    appended_column( comma::uint32 val ) : value(val) {}
};

namespace comma { namespace visiting {

template <> struct traits< input_t >
{
    template < typename K, typename V > static void visit( const K&, const input_t& p, V& v )
    {
        v.apply( "key", p.key );
    }
    template < typename K, typename V > static void visit( const K&, input_t& p, V& v )
    {
        v.apply( "key", p.key );
    }
};

template <> struct traits< input_with_block >
{
    template < typename K, typename V > static void visit( const K&, const input_with_block& p, V& v )
    {
        v.apply( "block", p.block );
    }
    template < typename K, typename V > static void visit( const K&, input_with_block& p, V& v )
    {
        v.apply( "block", p.block );
    }
};

template <> struct traits< input_with_index >
{
    template < typename K, typename V > static void visit( const K&, const input_with_index& p, V& v )
    {
        v.apply( "index", p.index );
    }
    template < typename K, typename V > static void visit( const K&, input_with_index& p, V& v )
    {
        v.apply( "index", p.index );
    }
};

template <> struct traits< appended_column >
{
    template < typename K, typename V > static void visit( const K&, const appended_column& p, V& v )
    {
        v.apply( "value", p.value );
    }
    template < typename K, typename V > static void visit( const K&, appended_column& p, V& v )
    {
        v.apply( "value", p.value );
    }
};

} } // namespace comma { namespace visiting {

static void usage( bool more )
{
    std::cerr << std::endl;
    std::cerr << "operations" << std::endl;
    std::cerr << "    group|make-blocks" << std::endl;
    std::cerr << "        cat something.csv | csv-blocks group --fields=,id, " << std::endl;
    std::cerr << "            appends group's block field base on specified id key or keys" << std::endl;
    std::cerr << "    index" << std::endl;
    std::cerr << "        appends an index field counting the number of records for each block. Use --reverse for count down." << std::endl;
    std::cerr << "            cat something.csv | csv-blocks index --fields=,block " << std::endl;
    std::cerr << "    increment" << std::endl;
    std::cerr << "        increments specified field value, must be uint32 type - any such field can be used as a block:" << std::endl;
    std::cerr << "            cat something.csv | csv-blocks increment --fields=,,block" << std::endl;
    std::cerr << "            cat something.csv | csv-blocks increment --fields=,,block --step 2" << std::endl;
    std::cerr << "            cat something.csv | csv-blocks increment --fields=,,block --step -1" << std::endl;
    std::cerr << "    head" << std::endl;
    std::cerr << "        reads records from first block to stdout, if --num-of-blocks=<num> specified, read more than one blocks" << std::endl;
    std::cerr << "        requires the index from 'index' mode in the inputs" << std::endl;
    std::cerr << "            cat something.csv | csv-blocks index --fields=,index " << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --help,-h: help; --help --verbose: more help" << std::endl;
    std::cerr << "    --reverse; use with 'index' operation, output the indices in descending order instead of ascending" << std::endl;
    std::cerr << "    --from,--starting-block; use with 'group' operation, the starting block number to use, default is 1" << std::endl;
    std::cerr << "    --step; use with 'increment' operation, the number of increment/decrement for specified field, default is 1" << std::endl;
    std::cerr << "    --lines,--num-of-blocks,-n; use with 'head' operation, outputs only the first specified number of blocks, default is 1" << std::endl;
    if( more ) { std::cerr << comma::csv::options::usage() << std::endl << std::endl; }
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    block_csv=block.csv" << std::endl;
    std::cerr << "    ( echo \"a,1,2,3\"; echo \"a,4,2,3\"; echo \"b,5,5,6\"; echo \"c,7,5,6\"; echo \"c,7,8,9\"; echo \"c,7,8,9\" ) >$block_csv" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    group|make-blocks" << std::endl;
    std::cerr << "        cat $block_csv | csv-blocks group --fields=id" << std::endl;
    std::cerr << "            unique ascending block number are assigned based on one id field" << std::endl;
    std::cerr << "        cat $block_csv | csv-blocks group --fields=id,,id" << std::endl;
    std::cerr << "            unique ascending block number are assigned based on two id fields" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    index" << std::endl;
    std::cerr << "        cat $block_csv | csv-blocks append --fields=id | csv-blocks index --fields=,,,,block" << std::endl;
    std::cerr << "            Append will add block field at the end, 'index' will append how many lines/records are left in the block" << std::endl;
    std::cerr << "            See 'head operation' below" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    increment" << std::endl;
    std::cerr << "        cat $block_csv | csv-blocks increment --fields=,block " << std::endl;
    std::cerr << "            Given an integer field (any field), mark it as a block field so the field value is incremented." << std::endl;
    std::cerr << "            This increments the second field" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    head" << std::endl;
    std::cerr << "        cat $block_csv | csv-blocks group --fields=id | csv-blocks index --fields=,,,,index | csv-blocks head --fields=,,,,,index " << std::endl;
    std::cerr << "            After appending the block field, then the block reverse index field, reading a single block from the input is possible" << std::endl;
    std::cerr << std::endl;
    std::cerr << "contact info: " << comma::contact_info <<std::endl;
    exit(0);
}

static bool verbose;
static comma::csv::options csv;
static input_t default_input;
static bool reverse_index = false;
// All the data for this block
static std::deque< std::string > block_records;
static comma::csv::impl::unstructured keys;
static comma::uint32 current_block = 1;
static comma::int32 increment_step = 1;

void flush_indexing( std::deque< std::string >& block_records, bool is_binary, char delimiter )
{
    std::size_t size = block_records.size();
    for( std::size_t i=0; i<size; ++i ) 
    {
        const std::string& input = block_records[i];
        comma::uint32 index = reverse_index ? size - (i+1) : i ;
        
        if( is_binary ) 
        { 
            std::cout.write( input.c_str(), input.size() );
            std::cout.write( (const char *) &index, sizeof(comma::uint32)); 
        }
        else 
        { 
            std::cout << input << delimiter << index << std::endl; 
        }
    }
    block_records.clear();
}

// This is to load the fields marked 'id' into input_t key structure
void make_blocks_setup( const comma::command_line_options& options, std::string& first_line )
{
    std::vector< std::string > v = comma::split( csv.fields, ',' );
    comma::csv::format f;
    if( csv.binary() ) { f = csv.format(); }
    else if( options.exists( "--format,--binary" ) ) { f = comma::csv::format( options.value< std::string >( "--format,--binary" ) ); }
    else
    {
        while( std::cin.good() && first_line.empty() ) { std::getline( std::cin, first_line ); }
        if( first_line.empty() ) { std::cerr << name() << "--format= is missing however the first line is empty" ; exit( 1 ); }
        f = comma::csv::impl::unstructured::guess_format( first_line, csv.delimiter );
        if( verbose ) { std::cerr << name() << "guessed format: " << f.string() << std::endl; }
    }
    // This is to load the keys into input_t structure
    unsigned int size = f.count();
    for( std::size_t i = 0; i < size; ++i )
    {
        if( i < v.size() )
        {
            if( v[i] == "id" ) { v[i] = "key/" + default_input.key.append( f.offset( i ).type ); continue; }
        }
    }
    csv.fields = comma::join( v, ',' );
}

struct memory_buffer
{
    char* buffer;
    size_t size;
    memory_buffer();
    memory_buffer( size_t size ) { allocate( size ); }
    ~memory_buffer();
    
    void allocate( size_t size );
};
memory_buffer::memory_buffer() : buffer(NULL), size(0) {}
memory_buffer::~memory_buffer()
{
    if( buffer != NULL ) {  
//         std::cerr  << "freeing head ascii memory, size: " << size << std::endl;
        free( (void*) buffer ); 
    }
}

void memory_buffer::allocate(size_t size)
{
    this->size = size;
    buffer = (char*) ::malloc( size );
    if( buffer == NULL ) { std::cerr << name() << "failed to allocate memory buffer of size: " << size << std::endl; exit(1); }
}

/// Read a record and  fills out param 'record', the binary data is immediately send to stdout
static comma::uint32 read_and_write_binary_record()
{
    static memory_buffer memory( csv.format().size() );
    static comma::csv::binary< input_with_index > binary( csv );
    // Reads from stdin, the exact number of bytes to be memory.size
    //  It should blocks and keep reading until we have the entire message,
    //  This is to reassemble the message if it is broken into pieces (e.g. TCP input piped into stdin)
    input_with_index record;
    size_t bytes_read = 0;
    while ( bytes_read < memory.size && !::feof( stdin ) && !::ferror(stdin) )
    {
        bytes_read +=  ::fread( &memory.buffer[bytes_read], 1, memory.size - bytes_read, stdin );
    }
    if ( bytes_read == 0 ) { exit( 0 ); } 
    if ( bytes_read < memory.size ) { std::cerr << "csv-blocks: expected " << memory.size << " bytes; got only: " << bytes_read << std::endl; exit( 1 ); } 
    
    // fill 'record' param
    binary.get( record, memory.buffer );
    // send data to stdout
    std::cout.write( memory.buffer, memory.size );
    return record.index;
}

/// Read a record and  fills out param 'record', the binary data is immediately send to stdout
// when return value is false, 'record' is not filled because line read is empty
static comma::uint32 read_and_write_ascii_record()
{
    static memory_buffer memory( 1024 );
    static comma::csv::ascii< input_with_index > ascii( csv );
    while( true )
    {
        input_with_index record;
        std::stringstream sstream;
        bool has_end_of_line = false;
        while( !has_end_of_line && !::feof(stdin) )
        {
            // getline may actually changes the buffer with realloc and extend the size
            ssize_t bytes_read = ::getline( &memory.buffer, &memory.size, stdin );
            if ( bytes_read <= 0 ) { exit( 0 ); }  // We are done
#ifndef WIN32
            has_end_of_line = ( memory.buffer[bytes_read-1] == '\n' );
            sstream.write( memory.buffer, has_end_of_line ? bytes_read-1 : bytes_read );
#else
            has_end_of_line = ( bytes_read > 1 
                                && memory.buffer[bytes_read-2] == '\r' 
                                && memory.buffer[bytes_read-1] == '\n' );
            sstream.write( memory.buffer, has_end_of_line ? bytes_read-2 : bytes_read );
#endif
        }        
        std::string line = sstream.str();
        if( line.empty() ) { continue; }
        // filling 'record' param
        ascii.get( record, line );
        // send line to stdout
        std::cout << line << std::endl;
        return record.index; // 'record' filled
    }
}

int main( int ac, char** av )
{
    try
    {
        comma::command_line_options options( ac, av, usage );
        verbose = options.exists( "--verbose,-v" );
        csv = comma::csv::options( options );
        csv.full_xpath = true;
        csv.quote.reset();
        
        comma::csv::options csv_out;
        if( csv.binary() ) { csv_out.format( comma::csv::format("ui") ); }
        
        std::vector< std::string > unnamed = options.unnamed( "--help,-h,--reverse,--verbose,-v", "-.*" );
        if( unnamed.size() < 1 ) { std::cerr << name() << "expected one operation, got " << comma::join( unnamed, ' ' ) << std::endl; return 1; }
        const std::string  operation = unnamed.front();
        
        if( verbose ) { std::cerr << name() << "csv fields: " << csv.fields << std::endl; }
        
        
        if( operation == "group" || operation == "make-blocks" )
        {
            current_block = options.value< comma::uint32 >( "--starting-block,--from", 1 ); // default is 1
            
            std::string first_line;
            make_blocks_setup( options, first_line );
            if( verbose ) { std::cerr << name() << "csv fields: " << csv.fields << std::endl; }
            if ( default_input.key.empty() ) { std::cerr << name() << "please specify at least one id field" << std::endl; return 1; }
            
            comma::csv::input_stream< input_t > istream( std::cin, csv, default_input );
            comma::csv::output_stream< appended_column > ostream( std::cout, csv_out );
            comma::csv::tied< input_t, appended_column > tied( istream, ostream );
            
            if( !first_line.empty() ) 
            { 
                input_t p = comma::csv::ascii< input_t >( csv, default_input ).get( first_line ); 
                if( !(keys == p.key) ) { ++current_block; }
                keys = p.key;
                // This is needed because the record wasnt read in by istream
                // Write it out
                if( istream.is_binary() ) { std::cout.write( (char*)&p, istream.binary().size() ); }
                else { std::cout << first_line << istream.ascii().ascii().delimiter(); }
                ostream.write( appended_column( current_block ) );
            }
            while( istream.ready() || ( std::cin.good() && !std::cin.eof() ) )
            {
                const input_t* p = istream.read();
                if( !p ) { break; }
                if( !(keys == p->key) ) { ++current_block; }
                keys = p->key;
                tied.append( appended_column( current_block ) );
            }
            
            return 0;
        }
        else if( operation == "head" )
        {
            ::setvbuf( stdin, (char *)NULL, _IONBF, 0 );
            #ifdef WIN32
                if( csv.binary() ) { _setmode( _fileno( stdin ), _O_BINARY ); }
            #endif
            comma::uint32 num_of_blocks = options.value< comma::uint32 >( "--lines,--num-of-blocks,-n", 1 );
            while( num_of_blocks > 0 )
            {
                comma::uint32 index = csv.binary() ? read_and_write_binary_record() : read_and_write_ascii_record();
                if( index == 0 ) { --num_of_blocks; }
            }
            return 0;
        }
        else if( operation == "index" )
        {
            reverse_index = options.exists("--reverse");
            
            comma::csv::input_stream< input_with_block > istream( std::cin, csv );
            
            char delimiter = istream.is_binary() ? ',' : istream.ascii().ascii().delimiter();
            comma::uint32 block = 0;
            std::string buffer;
            if( istream.is_binary() ) { buffer.resize( istream.binary().size() ); }
            
            while( istream.ready() || ( std::cin.good() && !std::cin.eof() ) )
            {
                const input_with_block* p = istream.read();
                if( !p ) { break; }
                
                if( block != p->block && !block_records.empty() ) { flush_indexing( block_records, istream.is_binary(), delimiter  ); }
                block = p->block;
                
                // Put the input into the buffer
                if( istream.is_binary() )  
                { 
                    memcpy( &buffer[0], istream.binary().last(),  istream.binary().size() ); 
                    block_records.push_back( buffer );
                }
                else { block_records.push_back( comma::join( istream.ascii().last(), delimiter ) ); }
            }
            
            // flushes the last block
            flush_indexing( block_records, istream.is_binary(), delimiter  );
            
            return 0;
        }
        else if( operation == "increment" )    // operation is head
        {
            increment_step = options.value< comma::int32 >( "--step", 1 );
            
            comma::csv::input_stream< input_with_block > istream( std::cin, csv );
            comma::csv::output_stream< appended_column > ostream( std::cout, csv_out );
            comma::csv::tied< input_with_block, appended_column > tied( istream, ostream );
            
            appended_column incremented;
            while( istream.ready() || ( std::cin.good() && !std::cin.eof() ) )
            {
                const input_with_block* p = istream.read();
                if( !p ) { break; }
                
                incremented.value = p->block + increment_step;
                tied.append( incremented );
            }
            
            return 0;
        }
        else { std::cerr << name() << "unrecognised operation '" << operation << "'" << std::endl; }
        
        return 1;
    }
    catch( std::exception& ex ) { std::cerr << name() << ex.what() << std::endl; }
    catch( ... ) { std::cerr << name() << "unknown exception" << std::endl; }
    return 1;
}
