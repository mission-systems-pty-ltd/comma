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

#ifndef WIN32
#include <stdlib.h>
#include <sys/ioctl.h>
#endif

#include <vector>
#include <boost/asio/ip/udp.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include "../../application/command_line_options.h"
#include "../../application/signal_flag.h"
#include "../../base/exception.h"
#include "../../base/types.h"
#include "../../io/stream.h"
#include "../../io/select.h"
#include "../../string/string.h"

void usage( bool verbose = false )
{
    std::cerr << std::endl;
    std::cerr << "read from one or a few sources, merge, and output to stdout" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: io-cat <address> [<address>] ... [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "<address>" << std::endl;
    std::cerr << "    local:<path>: local socket" << std::endl;
    std::cerr << "    tcp:<host>:<port>: tcp socket" << std::endl;
    std::cerr << "    udp:<port>: udp socket" << std::endl;
    std::cerr << "    zmp-<protocol>:<address>: zmq (todo)" << std::endl;
    std::cerr << "    <filename>: file" << std::endl;
    std::cerr << "    <fifo>: named pipe" << std::endl;
    std::cerr << "    -: stdin" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --exit-on-first-closed,-e: exit, if one of the streams finishes" << std::endl;
    std::cerr << "    --flush,--unbuffered,-u: flush output" << std::endl;
    std::cerr << "    --verbose,-v: more output" << std::endl;
    std::cerr << std::endl;
    std::cerr << "output order options" << std::endl;
    std::cerr << "    --blocking: blocking read on each source in order sources appear on command line" << std::endl;
    std::cerr << "                output modes" << std::endl;
    std::cerr << "                    default:     output all records from the first source, then all" << std::endl;
    std::cerr << "                                 records from the seconds source, etc" << std::endl;
    std::cerr << "                    round robin: output <n> records from the first source, then <n>" << std::endl;
    std::cerr << "                                 records from the seconds source, etc; note that if" << std::endl;
    std::cerr << "                                 the number of records in a source is not divisible by <n>" << std::endl;
    std::cerr << "                                 then the last records groups may contain fewer than <n>" << std::endl;
    std::cerr << "                                 records" << std::endl;
    std::cerr << "                attention: if you want full control over record ordering, use --blocking" << std::endl;
    std::cerr << "                           when using subshells or sockets as io-cat inputs " << std::endl;
    if( verbose )
    {
        std::cerr << "                           io-cat will open such inputs, but they may not be immediately" << std::endl;
        std::cerr << "                           ready for reading, which may lead to records being read from sources" << std::endl;
        std::cerr << "                           out of order;  use --blocking to avoid this problem" << std::endl;
        std::cerr << "                           e.g. in the following command without --blocking one subshell may" << std::endl;
        std::cerr << "                           start slightly earlier than the other and thus likely to output" << std::endl;
        std::cerr << "                           not what you expect or want - add --blocking to fix that:" << std::endl;
        std::cerr << "                               io-cat --round-robin=1 \\" << std::endl;
        std::cerr << "                                      <( csv-paste line-number value=a | head -n100 ) \\" << std::endl;
        std::cerr << "                                      <( csv-paste line-number value=b | head -n100 )" << std::endl;
    }
    else
    {
        std::cerr << "                           run io-cat --help --verbose for more details" << std::endl;
    }
    std::cerr << "    --head=[<n>]; output first <n> records and exit without waiting for record n+1" << std::endl;
    std::cerr << "                  a workaround for sparse input fed into: io-cat ... | head -n10, which" << std::endl;
    std::cerr << "                  not exit until io-cat receives record 11" << std::endl;
    std::cerr << "                  instead run: io-cat ... --head=10 (use --flush if you don't want buffering" << std::endl;
    std::cerr << "    --repeat=[<n>]; read each stream, output <n> times" << std::endl;
    std::cerr << "                  e.g: run: io-cat my-file-1 my-file-2 --repeat=3" << std::endl;
    std::cerr << "                       instead of: cat my-file-1 my-file-2 my-file-1 my-file-2 my-file-1 my-file-2" << std::endl;
    std::cerr << "                  when using for large source, be aware that the sources get stored in memory first" << std::endl;
    std::cerr << "    --repeat-forever,--forever; same as --repeat, but forever" << std::endl;
    std::cerr << "    --round-robin=[<number of packets>]: only for multiple inputs: read not more" << std::endl;
    std::cerr << "                                         than <number of packets> from an input at once," << std::endl;
    std::cerr << "                                         before checking other inputs" << std::endl;
    std::cerr << "                                         if not specified, read from each input" << std::endl;
    std::cerr << "                                         all available data" << std::endl;
    std::cerr << "                                         ignored for udp streams, where one full udp" << std::endl;
    std::cerr << "                                         packet at a time is always read" << std::endl;
    std::cerr << "    --size=[<bytes>]; on fixed-width binary records, size of the record in bytes, for --round-robin or --head" << std::endl;
    std::cerr << std::endl;
    std::cerr << "connect options" << std::endl;
    std::cerr << "    --connect-max-attempts,--connect-attempts,--attempts,--max-attempts=<n>; default=1; number of attempts to reconnect or 'unlimited'" << std::endl;
    std::cerr << "    --connect-period=<seconds>; default=1; how long to wait before the next connect attempt" << std::endl;
    std::cerr << "    --permissive; run even if connection to some sources fails" << std::endl;
    std::cerr << std::endl;
    std::cerr << "supported address types: tcp, udp, local (unix) sockets, named pipes, files, zmq (todo)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    single stream" << std::endl;
    std::cerr << "        io-cat tcp:localhost:12345" << std::endl;
    std::cerr << "        io-cat udp:12345" << std::endl;
    std::cerr << "        io-cat local:/tmp/socket" << std::endl;
    std::cerr << "        io-cat some/pipe" << std::endl;
    std::cerr << "        io-cat some/file" << std::endl;
    std::cerr << "        io-cat zmq-local:/tmp/socket (not implemented)" << std::endl;
    std::cerr << "        io-cat zmq-tcp:localhost:12345 (not implemented)" << std::endl;
    std::cerr << "        echo hello | io-cat -" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    multiple streams" << std::endl;
    std::cerr << "        merge line-based input" << std::endl;
    std::cerr << "            io-cat tcp:localhost:55555 tcp:localhost:88888" << std::endl;
    std::cerr << "        merge binary input with packet size 100 bytes" << std::endl;
    std::cerr << "            io-cat tcp:localhost:55555 tcp:localhost:88888 --size 100" << std::endl;
    std::cerr << "        merge line-based input with stdin" << std::endl;
    std::cerr << "            echo hello | io-cat tcp:localhost:55555 -" << std::endl;
    std::cerr << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

class stream
{
    public:
        stream( const std::string& address ): address_( address ) {}
        virtual ~stream() {}
        virtual unsigned int read_available( std::vector< char >& buffer, unsigned int max_count, bool blocking ) = 0;
        virtual comma::io::file_descriptor fd() const = 0;
        virtual bool eof() const = 0;
        virtual bool empty() const = 0;
        virtual void close() = 0;
        virtual bool closed() const = 0;
        virtual bool connected() const = 0;
        virtual void connect() = 0;
        const std::string& address() const { return address_; }
        
    protected:
        std::string address_;
};

class udp_stream : public stream
{
    public:
        udp_stream( const std::string& address ) : stream( address )
        {
            const std::vector< std::string >& v = comma::split( address, ':' );
            if( v.size() != 2 ) { COMMA_THROW( comma::exception, "io-cat: expected udp:<port>, e.g. udp:12345, got '" << address << "'" ); }
            port_ = boost::lexical_cast< unsigned short >( v[1] );
        }
        
        bool eof() const { return false; }
        
        bool empty() const { return false; }
        
        void close() {}
        
        bool closed() const { return false; }
        
        comma::io::file_descriptor fd() const { return socket_->native_handle(); }
        
        unsigned int read_available( std::vector< char >& buffer, unsigned int, bool )
        {
            boost::system::error_code error;
            std::size_t size = socket_->receive( boost::asio::buffer( buffer ), 0, error );
            return error ? 0 : size;
        }
        
        bool connected() const { return bool( socket_ ); }
        
        void connect()
        {
            if( socket_ ) { return; }
            socket_.reset( new boost::asio::ip::udp::socket( service_ ) );
            socket_->open( boost::asio::ip::udp::v4() );
            boost::system::error_code error;
            socket_->set_option( boost::asio::ip::udp::socket::broadcast( true ), error );
            if( error ) { socket_.reset(); COMMA_THROW( comma::exception, "io-cat: udp failed to set broadcast option on port " << port_ ); }
            socket_->bind( boost::asio::ip::udp::endpoint( boost::asio::ip::udp::v4(), port_ ), error );
            if( error ) { socket_.reset(); COMMA_THROW( comma::exception, "io-cat: udp failed to bind port " << port_ ); }
        }
        
    private:
        unsigned short port_;
#if (BOOST_VERSION >= 106600)
        boost::asio::io_context service_;
#else
        boost::asio::io_service service_;
#endif
        mutable boost::scoped_ptr< boost::asio::ip::udp::socket > socket_; // boost::asio::ip::udp::socket::fd() is non-const for some reason
};

class any_stream : public stream
{
    public:
        any_stream( const std::string& address, unsigned int size, bool binary ): stream( address ), size_( size ), binary_( binary ), closed_( false ) {}
        
        comma::io::file_descriptor fd() const { return ( *istream_ ).fd(); }
        
        unsigned int read_available( std::vector< char >& buffer, unsigned int max_count, bool blocking )
        {
            std::size_t available = available_();
            if( !blocking && available == 0 ) { return 0; }
            if( binary_ )
            {
                unsigned int count = size_ ? available / size_ : 0;
                if( max_count && count > max_count ) { count = max_count; }
                if( count == 0 ) { count = 1; } // read at least one packet
                unsigned int size = size_ ? count * size_ : std::min( available, buffer.size() );
                ( *istream_ )->read( &buffer[0], size );
                return ( *istream_ )->gcount() <= 0 ? 0 : ( *istream_ )->gcount();
            }
            else
            {
                std::string line; // quick and dirty, no-one expects ascii to be fast
                std::getline( *( *istream_ ), line );
                if( line.empty() ) { return 0; }
                if( line.size() >= buffer.size() ) { buffer.resize( line.size() + 1 ); }
                ::memcpy( &buffer[0], &line[0], line.size() );
                buffer[ line.size() ] = '\n';
                return line.size() + 1;
            }
        }
        
        bool empty() const { return !connected() || closed_ || available_() == 0; }
        
        bool eof() const { return bool( istream_ ) && ( !( *istream_ )->good() || ( *istream_ )->eof() ); }
        
        void close() { closed_ = true; ( *istream_ ).close(); }
        
        bool closed() const { return closed_; }
        
        bool connected() const { return bool( istream_ ); }
        
        void connect()
        {
            if( istream_ ) { return; }
            auto blocking_mode = false ? comma::io::mode::non_blocking : comma::io::mode::blocking; // todo? expose on command line?
            istream_.reset( new comma::io::istream( address_, comma::io::mode::binary, blocking_mode ) );
            if( ( *istream_ )() != &std::cin ) { return; }
            std::ios_base::sync_with_stdio( false ); // unsync to make rdbuf()->in_avail() working
            std::cin.tie( NULL ); // std::cin is tied to std::cout by default
        }
        
    private:
        boost::scoped_ptr< comma::io::istream > istream_;
        unsigned int size_;
        bool binary_;
        bool closed_;
        
        std::size_t available_() const // seriously quick and dirty
        {
            if( ( *istream_ )() == NULL ) { return ( *istream_ ).available_on_file_descriptor(); } // quick and dirty
            std::streamsize s = ( *istream_ )->rdbuf()->in_avail();
            if( s < 0 ) { return 0; }
            // todo: it should be s + available_on_file_descriptor(), but it won't work for std::cin (and potentially for std::ifstream (we have not checked)
            //       if performance becomes a problem e.g. for tcp, check whether the stream is not std::cin and use sum instead of max
            return std::max( static_cast< std::size_t >( s ), ( *istream_ ).available_on_file_descriptor() );
        }
};

static stream* make_stream( const std::string& address, unsigned int size, bool binary )
{
    const std::vector< std::string >& v = comma::split( address, ':' );
    if( v[0] == "udp" ) { return new udp_stream( address ); }
    if( v[0] == "zmq-local" || v[0] == "zero-local" || v[0] == "zmq-tcp" || v[0] == "zero-tcp" ) { COMMA_THROW( comma::exception, "io-cat: zmq support not implemented" ); }
    return new any_stream( address, size, binary );
}

static bool verbose;
static unsigned int connect_max_attempts;
static boost::posix_time::time_duration connect_period;
static bool permissive;

static bool ready( const boost::ptr_vector< stream >& streams, comma::io::select& select, bool connected_all_we_could, bool blocking )
{
    if( blocking )
    {
        select.check();
        bool r{connected_all_we_could};
        for( unsigned int i = 0; i < streams.size() && r; ++i ) { r = streams[i].closed() || select.read().ready( streams[i].fd() ); }
        if( !r ) { boost::this_thread::sleep( boost::posix_time::milliseconds( 10 ) ); } // quick and dirty
        return r;
    }
    for( unsigned int i = 0; i < streams.size(); ++i ) { if( !streams[i].empty() ) { select.check(); return true; } }
    if( !select.read()().empty() ) { return select.wait( boost::posix_time::seconds( 1 ) ) > 0; }
    if( connected_all_we_could ) { return true; }
    boost::this_thread::sleep( connect_period );
    return false;
}

static bool try_connect( boost::ptr_vector< stream >& streams, comma::io::select& select )
{
    static boost::posix_time::ptime next_connect_attempt_time;
    static unsigned int attempts = 0;
    static bool connected_all_we_could = false;
    static unsigned int unconnected_count = streams.size();
    if( connected_all_we_could ) { return connected_all_we_could; }
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
    if( !next_connect_attempt_time.is_not_a_date_time() && now <= next_connect_attempt_time ) { return connected_all_we_could; }
    next_connect_attempt_time = now + connect_period;
    std::string what;
    for( unsigned int i = 0; i < streams.size(); ++i )
    {
        if( streams[i].connected() ) { continue; }
        try
        {
            if( verbose ) { std::cerr << "io-cat: stream " << i << " (" << streams[i].address() << "): connecting, attempt " << ( attempts + 1 ) << " of " << ( connect_max_attempts == 0 ? std::string( "unlimited" ) : boost::lexical_cast< std::string >( connect_max_attempts ) ) << "..." << std::endl; }
            streams[i].connect();
            if( verbose ) { std::cerr << "io-cat: stream " << i << " (" << streams[i].address() << "): connected" << std::endl; }
            select.read().add( streams[i] );
            --unconnected_count;
            continue;
        }
        catch( std::exception& ex ) { what = ex.what(); }
        catch( ... ) { what = "unknown exception"; }
        if( verbose ) { std::cerr << "io-cat: stream " << i << " (" << streams[i].address() << "): failed to connect" << std::endl; }
    }
    ++attempts;
    connected_all_we_could = unconnected_count == 0 || ( permissive && connect_max_attempts > 0 && attempts >= connect_max_attempts );
    if( connected_all_we_could ) { return connected_all_we_could; }
    if( connect_max_attempts == 0 || attempts < connect_max_attempts ) { return connected_all_we_could; }
    std::cerr << "io-cat: fatal: after " << attempts << " attempt(s): " << what << std::endl;
    exit( 1 );
}

struct output_t
{
    unsigned int size{0};
    bool forever{false};
    std::vector< std::vector< char > > buffers; // todo: quick and dirty, watch performance on push back of large inputs
    operator bool() const { return !buffers.empty(); }

    output_t() = default;
    output_t( const comma::command_line_options& options, unsigned int n )
        : size( options.value( "--repeat", 0 ) )
        , forever( options.exists( "--repeat-forever,--forever" ) )
        , buffers( size > 0 || forever ? n : 0 )
    {
    }

    void write( unsigned int i, const std::vector< char >& buffer, unsigned int bytes_read )
    {
        if( buffers.empty() ) { std::cout.write( &buffer[0], bytes_read ); return; }
        unsigned int s = buffers[i].size();
        buffers[i].resize( s + bytes_read );
        std::memcpy( &buffers[i][s], &buffer[0], bytes_read );
    }

    void finalise( const comma::signal_flag& is_shutdown ) const
    {
        for( unsigned int i = 0; !is_shutdown && ( i < size || forever ); ++i )
        {
            for( unsigned int j = 0; !is_shutdown && j < buffers.size() && std::cout.good(); ++j )
            {
                std::cout.write( &buffers[j][0], buffers[j].size() );
            }
        }
    }
};

output_t output;

static bool _write( unsigned int i, const comma::command_line_options& options, const std::vector< char >& buffer, unsigned int bytes_read )
{
    static unsigned int head = options.value( "--head", 0 );
    static unsigned int size = options.value( "--size,-s", 0 );
    static unsigned int count = 0;
    if( head == 0 ) { output.write( i, buffer, bytes_read ); return true; }
    if( size == 0 )
    {
        output.write( i, buffer, bytes_read );
        ++count;
    }
    else
    {
        unsigned int n = std::min( bytes_read / size, head - count );
        output.write( i, buffer, n * size );
        count += n;
    }
    return count < head;
}

int main( int argc, char** argv )
{
    #ifdef WIN32
    std::cerr << "io-cat: not implemented on windows" << std::endl;
    return 1;
    #endif
    
    try
    {
        if( argc < 2 ) { usage(); }
        comma::command_line_options options( argc, argv, usage );
        comma::signal_flag is_shutdown;
        verbose = options.exists( "--verbose,-v" );
        unsigned int size = options.value( "--size,-s", 0 );
        bool blocking = options.exists( "--blocking" );
        bool unbuffered = options.exists( "--flush,--unbuffered,-u" );
        bool exit_on_first_closed = options.exists( "--exit-on-first-closed,-e" );
        options.assert_mutually_exclusive( "--blocking", "--permissive" );
        std::string connect_max_attempts_string = options.value< std::string >( "--connect-max-attempts,--connect-attempts,--attempts,--max-attempts", "1" );
        connect_max_attempts = connect_max_attempts_string == "unlimited" ? 0 : boost::lexical_cast< unsigned int >( connect_max_attempts_string );
        double connect_period_seconds = options.value( "--connect-period", 1.0 );
        connect_period = boost::posix_time::milliseconds( static_cast<unsigned int>(std::floor( connect_period_seconds * 1000 ) ));
        permissive = options.exists( "--permissive" );
        bool has_head = options.exists( "--head" );
        const std::vector< std::string >& unnamed = options.unnamed( "--repeat-forever,--forever,--blocking,--permissive,--exit-on-first-closed,-e,--flush,--unbuffered,-u,--verbose,-v", "-.+" );
        options.assert_mutually_exclusive( "--round-robin", "--repeat,--repeat-forever,--forever" );
        #ifdef WIN32
        if( size || ( unnamed.size() == 1 && !has_head ) ) { _setmode( _fileno( stdout ), _O_BINARY ); }
        //if( size ) { _setmode( _fileno( stdout ), _O_BINARY ); }
        #endif
        if( unnamed.empty() ) { std::cerr << "io-cat: please specify at least one source" << std::endl; return 1; }
        output = output_t( options, unnamed.size() );
        boost::ptr_vector< stream > streams;
        comma::io::select select;
        for( unsigned int i = 0; i < unnamed.size(); ++i ) { streams.push_back( make_stream( unnamed[i], size, size > 0 || ( unnamed.size() == 1 && !has_head ) ) ); }
        //for( unsigned int i = 0; i < unnamed.size(); ++i ) { streams.push_back( make_stream( unnamed[i], size, size > 0 ) ); }
        const unsigned int max_count = size ? ( size > 65536u ? 1 : 65536u / size ) : 0;
        std::vector< char > buffer( size ? size * max_count : 65536u );        
        unsigned int round_robin_count = unnamed.size() > 1 ? options.value( "--round-robin", 0 ) : 0;
        for( bool done = false; !done; )
        {
            if( is_shutdown ) { if( verbose ) { std::cerr << "io-cat: received signal" << std::endl; }; break; }
            bool connected_all_we_could = try_connect( streams, select );
            if( !ready( streams, select, connected_all_we_could, blocking ) ) { continue; }
            done = true;
            for( unsigned int i = 0; i < streams.size(); ++i )
            {
                if( !streams[i].connected() ) { done = connected_all_we_could; continue; }
                if( streams[i].closed() ) { continue; }
                bool ready = select.read().ready( streams[i].fd() );
                bool empty = streams[i].empty();
                if( empty && ( ready || streams[i].eof() ) )
                { 
                    if( verbose ) { std::cerr << "io-cat: stream " << i << " (" << unnamed[i] << "): closed" << std::endl; }
                    select.read().remove( streams[i].fd() );
                    streams[i].close();
                    if( exit_on_first_closed || ( connected_all_we_could && select.read()().empty() ) ) { done = true; break; }
                    continue;
                }
                if( !ready && empty ) { done = false; continue; }
                unsigned int countdown = round_robin_count;
                while( !streams[i].eof() ) // todo? check is_shutdown here as well?
                {
                    unsigned int bytes_read = streams[i].read_available( buffer, countdown ? countdown : max_count, blocking );
                    if( bytes_read == 0 ) { break; }
                    done = false;
                    if( size && bytes_read % size != 0 ) { std::cerr << "io-cat: stream " << i << " (" << streams[i].address() << "): expected " << size << " byte(s), got only " << ( bytes_read % size ) << std::endl; return 1; }
                    if( !_write( i, options, buffer, bytes_read ) ) { done = true; break; }
                    if( !std::cout.good() ) { done = true; break; }
                    if( unbuffered ) { std::cout.flush(); }
                    if( round_robin_count )
                    {
                        countdown -= ( size ? bytes_read / size : 1 );
                        if( countdown == 0 ) { break; }
                    }
                }
            }
        }
        output.finalise( is_shutdown );
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "io-cat: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "io-cat: unknown exception" << std::endl; }
    return 1;
}
