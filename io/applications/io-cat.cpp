// This file is part of comma, a generic and flexible library
// Copyright (c) 2011 The University of Sydney
// Copyright (c) 2024 Vsevolod Vlaskine
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
#include "../../io/select.h"
#include "../../io/server.h"
#include "../../io/stream.h"
#include "../../string/string.h"

void usage( bool verbose = false )
{
    std::cerr << R"(
read from one or a few sources, merge, and output to stdout

usage: io-cat <address> [<address>] ... [<options>]

<address>
    local:<path>: local socket
    tcp:<host>:<port>: tcp socket
    tcp:<port>: tcp server socket (only partly implemented)
    udp:<port>: udp socket
    zmp-<protocol>:<address>: zmq (todo)
    <filename>: file
    <fifo>: named pipe
    -: stdin

options
    --exit-on-first-closed,-e: exit, if one of the streams finishes
    --flush,--unbuffered,-u: flush output
    --verbose,-v: more output

output order options
    --blocking: blocking read on each source in order sources appear on command line
                output modes
                    default:     output all records from the first source, then all
                                 records from the seconds source, etc
                    round robin: output <n> records from the first source, then <n>
                                 records from the seconds source, etc; note that if
                                 the number of records in a source is not divisible by <n>
                                 then the last records groups may contain fewer than <n>
                                 records
                attention: if you want full control over record ordering, use --blocking
                           when using subshells or sockets as io-cat inputs)" << std::endl;
    if( verbose )
    {
        std::cerr << R"(                           io-cat will open such inputs, but they may not be immediately
                           ready for reading, which may lead to records being read from sources
                           out of order;  use --blocking to avoid this problem
                           e.g. in the following command without --blocking one subshell may
                           start slightly earlier than the other and thus likely to output
                           not what you expect or want - add --blocking to fix that:
                               io-cat --round-robin=1 \\
                                      <( csv-paste line-number value=a | head -n100 ) \\
                                      <( csv-paste line-number value=b | head -n100 ))" << std::endl;
    }
    else
    {
        std::cerr << "                           run io-cat --help --verbose for more details..." << std::endl;
    }
    std::cerr << R"(    --head=[<n>]; output first <n> records and exit without waiting for record n+1
                  a workaround for sparse input fed into: io-cat ... | head -n10, which
                  not exit until io-cat receives record 11
                  instead run: io-cat ... --head=10 (use --flush if you don't want buffering
    --repeat=[<n>]; read each stream, output <n> times
                  e.g: run: io-cat my-file-1 my-file-2 --repeat=3
                       instead of: cat my-file-1 my-file-2 my-file-1 my-file-2 my-file-1 my-file-2
                  when using for large source, be aware that the sources get stored in memory first
    --repeat-forever,--forever; same as --repeat, but forever
    --round-robin=[<number of packets>]: only for multiple inputs: read not more
                                         than <number of packets> from an input at once,
                                         before checking other inputs
                                         if not specified, read from each input
                                         all available data
                                         ignored for udp streams, where one full udp
                                         packet at a time is always read
    --size=[<bytes>]; on fixed-width binary records, size of the record in bytes, for --round-robin or --head
    
connect options
    --connect-max-attempts,--connect-attempts,--attempts,--max-attempts=<n>; default=1; number of attempts to reconnect or 'unlimited'
    --connect-period=<seconds>; default=1; how long to wait before the next connect attempt
    --permissive; run even if connection to some sources fails
    
supported address types: tcp, udp, local (unix) sockets, named pipes, files, zmq (todo)
    
examples
    single stream
        io-cat tcp:localhost:12345
        io-cat udp:12345
        io-cat local:/tmp/socket
        io-cat some/pipe
        io-cat some/file
        io-cat zmq-local:/tmp/socket (not implemented)
        io-cat zmq-tcp:localhost:12345 (not implemented)
        echo hello | io-cat -
    multiple streams
        merge line-based input
            io-cat tcp:localhost:55555 tcp:localhost:88888
        merge binary input with packet size 100 bytes
            io-cat tcp:localhost:55555 tcp:localhost:88888 --size 100
        merge line-based input with stdin
            echo hello | io-cat tcp:localhost:55555 -
)" << std::endl;
    exit( 0 );
}

class stream
{
    public:
        stream( const std::string& address ): address_( address ) {}
        virtual ~stream() {}
        virtual unsigned int read_available( std::vector< char >& buffer, unsigned int max_count, bool blocking ) = 0;
        virtual comma::io::file_descriptor fd() const { return comma::io::invalid_file_descriptor; }
        virtual bool eof() const = 0;
        virtual bool empty() const = 0;
        virtual void close() = 0;
        virtual bool closed() const = 0;
        virtual bool connected() const = 0;
        virtual void connect() = 0;
        virtual void add_to( comma::io::select& select ) const { select.read().add( fd() ); }
        virtual void remove_from( comma::io::select& select ) const { select.read().remove( fd() ); }
        virtual bool ready( comma::io::select& select ) const { return select.read().ready( fd() ); }
        virtual void update( comma::io::select& select ) const {}
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

class client_stream : public stream
{
    public:
        client_stream( const std::string& address, unsigned int size, bool binary ): stream( address ), size_( size ), binary_( binary ), closed_( false ) {}
        
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

class server_stream : public stream // todo! super-quick and dirty! get streams from the server instead and add/remove them to/from read methods
{
    public:
        server_stream( const std::string& address, unsigned int size, bool binary, bool blocking )
            : stream( address )
            , _size( size )
            , _binary( binary )
            , _blocking( blocking )
            , _server( address, binary ? comma::io::mode::binary : comma::io::mode::ascii, blocking )
        {
        }
        
        unsigned int read_available( std::vector< char >& buffer, unsigned int max_count, bool blocking )
        {
            COMMA_ASSERT_BRIEF( blocking == _blocking, "server stream is " << ( _blocking ? "blocking" : "non-blocking" ) << ", but asked to do " << ( blocking ? "blocking" : "non-blocking" ) << " read" );
            unsigned int count{0};
            char* p = &buffer[0];
            while( true )
            {
                std::size_t available_at_least = _server.available_at_least();
                if( !blocking && available_at_least == 0 ) { return 0; }
                if( _binary )
                {
                    if( _server.read( p, _size ) != _size ) { return 0; } // todo? more checks?
                    ++count;
                    p += _size;
                    if( count >= max_count || count * _size >= available_at_least ) { return count * _size; }
                }
                else
                {
                    std::string line = _server.getline();
                    if( line.empty() ) { return 0; }
                    if( line.size() >= buffer.size() ) { buffer.resize( line.size() + 1 ); }
                    ::memcpy( &buffer[0], &line[0], line.size() );
                    buffer[ line.size() ] = '\n';
                    return line.size() + 1;
                }
            }
            return 0;
        }
        
        bool empty() const { return _closed || _server.available_at_least() == 0; }
        
        bool eof() const { return closed(); } // { return bool( istream_ ) && ( !( *istream_ )->good() || ( *istream_ )->eof() ); }
        
        void close() { _closed = true; _server.close(); }
        
        bool closed() const { return _closed; }
        
        bool connected() const { return true; }
        
        void connect() {}

        void add_to( comma::io::select& select ) const
        {
            for( auto d: _server.select().read()() ) { select.read().add( d ); }
            select.read().add( _server.acceptor_file_descriptor() ); // uber-quick and dirty
        }

        void remove_from( comma::io::select& select ) const
        {
            for( auto d: _server.select().read()() ) {  select.read().remove( d ); }
            select.read().add( _server.acceptor_file_descriptor() ); // uber-quick and dirty
        }

        void update( comma::io::select& select ) const
        {
            // todo
        }

        // todo!? use io::impl::receive()?
        // todo! get streams from the server instead and add/remove them to/from read methods
        // todo! test connecting/disconnecting clients
        // todo! test multiple clients
        // todo! cpu performance when there are no connections
        // todo? for now, if server, don't allow multiple input streams
        // todo! examples

        bool ready( comma::io::select& select ) const
        {
            for( auto d: _server.select().read()() ) { if( select.read().ready( d ) ) { return true; } }
            return false;
        }
        
    private:
        unsigned int _size{0};
        bool _binary{false};
        bool _blocking{false};
        bool _closed{false};
        comma::io::iserver _server;
};

static stream* make_stream( const std::string& address, unsigned int size, bool binary, bool blocking )
{
    const std::vector< std::string >& v = comma::split( address, ':' );
    if( v[0] == "udp" ) { return new udp_stream( address ); }
    if( v[0] == "tcp" && v.size() == 2 ) { return new server_stream( address, size, binary, blocking ); } // todo: quick and dirty for now; a better check if tcp:<port>-like
    COMMA_ASSERT_BRIEF( v[0] != "zmq-local" && v[0] != "zero-local" && v[0] != "zmq-tcp" && v[0] != "zero-tcp", "zmq support not implemented" );
    return new client_stream( address, size, binary );
}

static bool verbose;
static unsigned int connect_max_attempts;
static boost::posix_time::time_duration connect_period;
static bool permissive;

static bool ready( const boost::ptr_vector< stream >& streams, comma::io::select& select, bool connected_all_we_could, bool blocking )
{
    for( const auto& s: streams ) { s.update( select ); } // quick and dirty
    if( blocking )
    {
        select.check();
        bool r{connected_all_we_could};
        for( unsigned int i = 0; i < streams.size() && r; ++i ) { r = streams[i].closed() || streams[i].ready( select ); }
        if( !r ) { boost::this_thread::sleep( boost::posix_time::milliseconds( 10 ) ); } // quick and dirty
        return r;
    }
    for( unsigned int i = 0; i < streams.size(); ++i ) { if( !streams[i].empty() ) { select.check(); return true; } }
    if( !select.read()().empty() ) { return select.wait( boost::posix_time::milliseconds( 100 ) ) > 0; }
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
            comma::saymore() << "stream " << i << " (" << streams[i].address() << "): connecting, attempt " << ( attempts + 1 ) << " of " << ( connect_max_attempts == 0 ? std::string( "unlimited" ) : boost::lexical_cast< std::string >( connect_max_attempts ) ) << "..." << std::endl;
            streams[i].connect();
            comma::saymore() << "stream " << i << " (" << streams[i].address() << "): connected" << std::endl;
            streams[i].add_to( select );
            --unconnected_count;
            continue;
        }
        catch( std::exception& ex ) { what = ex.what(); }
        catch( ... ) { what = "unknown exception"; }
        comma::saymore() << "stream " << i << " (" << streams[i].address() << "): failed to connect" << std::endl;
    }
    ++attempts;
    if( unconnected_count == 0 ) { return true; }
    if( connect_max_attempts == 0 ) { return false; }
    if( attempts < connect_max_attempts ) { return false; }
    if( permissive ) { return true; }
    comma::say() << "fatal: after " << attempts << " attempt(s): " << what << std::endl;
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
    comma::say() << "not implemented on windows" << std::endl;
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
        COMMA_ASSERT_BRIEF( !unnamed.empty(), "please specify at least one source" );
        output = output_t( options, unnamed.size() );
        boost::ptr_vector< stream > streams;
        comma::io::select select;
        for( unsigned int i = 0; i < unnamed.size(); ++i ) { streams.push_back( make_stream( unnamed[i], size, size > 0 || ( unnamed.size() == 1 && !has_head ), blocking ) ); }
        //for( unsigned int i = 0; i < unnamed.size(); ++i ) { streams.push_back( make_stream( unnamed[i], size, size > 0 ) ); }
        comma::saymore() << "created " << unnamed.size() << " stream" << ( unnamed.size() == 1 ? "" : "s" ) << std::endl;
        const unsigned int max_count = size ? ( size > 65536u ? 1 : 65536u / size ) : 0;
        std::vector< char > buffer( size ? size * max_count : 65536u );        
        unsigned int round_robin_count = unnamed.size() > 1 ? options.value( "--round-robin", 0 ) : 0;
        for( bool done = false; !done; )
        {
            if( is_shutdown ) { comma::saymore() << "received signal" << std::endl; break; }
            bool connected_all_we_could = try_connect( streams, select );
            if( !ready( streams, select, connected_all_we_could, blocking ) ) { continue; }
            done = true;
            for( unsigned int i = 0; i < streams.size(); ++i )
            {
                if( !streams[i].connected() ) { done = connected_all_we_could; continue; }
                if( streams[i].closed() ) { continue; }
                bool ready = streams[i].ready( select );
                bool empty = streams[i].empty();
                if( empty && ( ready || streams[i].eof() ) )
                {
                    comma::saymore() << "stream " << i << " (" << unnamed[i] << "): closed" << std::endl;
                    streams[i].remove_from( select );
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
                    COMMA_ASSERT_BRIEF( !( size && bytes_read % size != 0 ), "stream " << i << " (" << streams[i].address() << "): expected " << size << " byte(s), got only " << ( bytes_read % size ) );
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
    catch( std::exception& ex ) { comma::say() << ex.what() << std::endl; }
    catch( ... ) { comma::say() << "unknown exception" << std::endl; }
    return 1;
}
