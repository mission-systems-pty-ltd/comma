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

/// @author cedric wohlleber

#include <fcntl.h>
#include <stdio.h>
#ifdef WIN32
#include <io.h>
#else
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#endif
#include <termios.h>

#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include "../../application/command_line_options.h"
#include "../../application/signal_flag.h"
#include "../../base/exception.h"
#include "../../csv/options.h"
#include "../../csv/traits.h"
#include "../../name_value/parser.h"
#include "../../csv/applications/play/multiplay.h"

static void bash_completion( unsigned const ac, char const* const* av )
{
    static const char* completion_options =
        " --help -h"
        " --speed --slowdown --slow"
        " --quiet"
        " --fields --binary"
        " --clients"
        " --interactive -i"
        " --no-flush "
        " --paused-at-start --paused"
        " --resolution"
        " --from --to"
        ;
    std::cout << completion_options << std::endl;
    exit( 0 );
}

static void interactive_help( std::string prefix )
{
    prefix.assign( prefix.size(), ' ' );
    std::cerr << prefix << "<space>: pause or resume" << std::endl;
    std::cerr << prefix << "right or down arrow key: output one record at a time" << std::endl;
    std::cerr << prefix << "<t>: output current timestamp to stderr" << std::endl;
    std::cerr << prefix << "<q>: quit" << std::endl;
}

static void usage( bool )
{
    std::cerr << std::endl;
    std::cerr << "play back timestamped data from standard input in a real time manner" << std::endl;
    std::cerr << "to standard output or optionally into given files/pipes" << std::endl;
    std::cerr << std::endl;
    std::cerr << "usage: csv-play [<options>]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "options" << std::endl;
    std::cerr << "    --speed: speed-up playback by a factor, default is 1 (inverse to --slowdown)" << std::endl;
    std::cerr << "    --slowdown,--slow: slow-down playback by a factor, default is 1 (inverse to --speed)" << std::endl;
    std::cerr << "    --quiet: don't print warnings when lagging behind" << std::endl;
    std::cerr << "    --fields <fields> : specify where timestamp is" << std::endl;
    std::cerr << "                        e.g., if timestamp is the 4th field: --fields=\",,,t\"" << std::endl;
    std::cerr << "                        default: the timestamp is the first field" << std::endl;
    std::cerr << "    --binary <format> : use binary format" << std::endl;
    std::cerr << "    --clients: minimum number of clients to connect to each stream" << std::endl;
    std::cerr << "               before playback starts; default 0" << std::endl;
    std::cerr << "               can be specified individually for each client, e.g." << std::endl;
    std::cerr << "               csv-play file1;pipe;clients=1 file2;tcp:1234;clients=3" << std::endl;
    std::cerr << "    --interactive,-i: react to key presses:" << std::endl;
    interactive_help( "    --interactive,-i: " );
    std::cerr << "    --no-flush : if present, do not flush the output stream ( use on high bandwidth sources )" << std::endl;
    std::cerr << "    --paused-at-start,--paused: start playback as paused, implies --interactive" << std::endl;
    std::cerr << "    --pause-at=[<timestamp>]; pause when timestamp reached, implies --interactive" << std::endl;
    std::cerr << "    --resolution=<second>: timestamp resolution; timestamps closer than this value will be" << std::endl;
    std::cerr << "                           played without delay; the rationale is that microsleep used in csv-play" << std::endl;
    std::cerr << "                           (boost::this_thread::sleep()) is essentially imprecise and may create" << std::endl;
    std::cerr << "                           unnecessary delays in the data" << std::endl;
    std::cerr << "                           default 0.01" << std::endl;
    std::cerr << "    --from <timestamp> : play back data starting at <timestamp> ( iso format )" << std::endl;
    std::cerr << "    --to <timestamp> : play back data up to <timestamp> ( iso format )" << std::endl;
    std::cerr << comma::csv::format::usage();
    std::cerr << std::endl;
    std::cerr << "output" << std::endl;
    std::cerr << "    -: write to stdout (default)" << std::endl;
    std::cerr << "    offset=<offset>: add <offset> seconds to the timestamp of this source" << std::endl;
    std::cerr << "    <filename>: write to file or named pipe, e.g. csv-play \"points.csv;pipe\"" << std::endl;
    std::cerr << "    tcp:<port>: open tcp server socket on given port and write to the tcp clients" << std::endl;
    std::cerr << "    local:<name>: same as tcp, but use unix/linux domain sockets" << std::endl;
    std::cerr << std::endl;
    std::cerr << "examples" << std::endl;
    std::cerr << "    output timestamped 3d points in real time manner to stdout (e.g. for visualisation)" << std::endl;
    std::cerr << "        cat points.csv | csv-play | view-points --fields=,x,y,z" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    play back several files and output to, say, named pipes:" << std::endl;
    std::cerr << "        mkfifo file1.pipe file2.pipe" << std::endl;
    std::cerr << "        csv-play \"file1.csv;pipe1\" \"file2.csv;pipe2\" &" << std::endl;
    std::cerr << "        view-points pipe1 pipe2 --fields=,x,y,z" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    same as above, but block, until all the pipes are connected:" << std::endl;
    std::cerr << "        csv-play \"file1.csv;pipe1\" \"file2.csv;pipe2\" --clients=1 &" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    output multiple inputs of the same format to stdout:" << std::endl;
    std::cerr << "        csv-play \"file1.csv;-\" \"file2.csv;-\" &" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    use binary data (try it)" << std::endl;
    std::cerr << "        > csv-play <( csv-paste line-number | csv-repeat --pace --period 1 | csv-time-amp | csv-to-bin t,ui --flush )';-;binary=t,ui' \\" << std::endl;
    std::cerr << "                 <( csv-paste line-number value=0 | csv-repeat --pace --period 1 | csv-time-stamp | csv-to-bin t,2ui --flush )';tcp:8888;binary=t,2ui' \\" << std::endl;
    std::cerr << "            | csv-from-bin t,ui" << std::endl;
    std::cerr << "        > #in another shell, run" << std::endl;
    std::cerr << "        > socat tcp:localhost:8888 - | csv-from-bin t,2ui" << std::endl;
    std::cerr << std::endl;
    std::cerr << "    pause and step through output:" << std::endl;
    std::cerr << "        echo 0 | csv-repeat --period 0.1 --yes | csv-paste - line-number \\" << std::endl;
    std::cerr << "            | csv-time-stamp | csv-play --interactive" << std::endl;
    std::cerr << std::endl;
    exit( 0 );
}

static boost::scoped_ptr< comma::Multiplay > multiplay;
static bool quit = false;

class playback_state_t
{
public:
    playback_state_t() : state_( state::running ) {}

    bool is_running() const { return state_ == state::running; }
    bool is_paused() const { return state_ == state::paused; }

    void pause( const boost::posix_time::ptime& t = boost::posix_time::not_a_date_time )
    {
        if( state_ != state::paused )
        {
            state_ = state::paused;
            paused_time_ = boost::posix_time::microsec_clock::universal_time();
            if( ! t.is_not_a_date_time() ) { std::cerr << "csv-play: paused at " << boost::posix_time::to_iso_string( t ) << std::endl; }
        }
    }

    void unpause()
    {
        multiplay->paused_for( boost::posix_time::microsec_clock::universal_time() - paused_time_ );
    }

    void run()
    {
        if( state_ != state::running )
        {
            if( state_ == state::paused ) { unpause(); }
            state_ = state::running;
            std::cerr << "csv-play: resumed" << std::endl;
        }
    }

    void read_once()
    {
        if( state_ == state::paused ) { unpause(); }
        state_ = state::read_once;
    }

    void has_read_once()
    {
        if( state_ == state::read_once ) { pause(); }
    }

private:
    enum class state { running, paused, read_once, read_block };

    state state_;
    boost::posix_time::ptime paused_time_;
};

static playback_state_t playback;

class key_press_handler_t
{
public:
    key_press_handler_t( bool interactive ) : key_press_( interactive ) {}

    void update( boost::posix_time::ptime t )
    {
        key k = get_key();
        switch( k )
        {
            case key::space:
                if( playback.is_running() ) { playback.pause( t ); }
                else { playback.run(); }
                break;
            case key::down_arrow:
            case key::right_arrow:
                playback.read_once();
                break;
            case key::q:
                quit = true;
                break;
            case key::t:
                std::cerr << boost::posix_time::to_iso_string( t ) << std::endl;
                break;
            case key::none:
            case key::other:
                break;
        }
    }
    
private:
    enum class key { none, space, right_arrow, down_arrow, q, t, other };

    key get_key()
    {
        boost::optional< char > c = key_press_.read();
        if( !c ) { return key::none; }
        switch( *c )
        {
            case ' ': return key::space;
            case 'q': return key::q;
            case 't': return key::t;
            case 27:                    // escape sequence for arrows: ESC-[
                c = key_press_.read();
                if( !c || *c != 91 ) { break; }
                c = key_press_.read();
                if( !c ) { break; }
                if( *c == 66 ) { return key::down_arrow; }
                if( *c == 67 ) { return key::right_arrow; }
                break;
        }
        return key::other;
    }

    class key_press_t_
    {
    public:
        key_press_t_( bool interactive = false, const std::string& tty = "/dev/tty" ) : interactive_( interactive )
        {
            if( !interactive_ ) { return; }
            fd_ = ::open( &tty[0], O_RDONLY | O_NONBLOCK | O_NOCTTY );
            if( !isatty( fd_ ) ) { COMMA_THROW( comma::exception, "'" << tty << "' is not tty" ); }
            if( fd_ == -1 ) { COMMA_THROW( comma::exception, "failed to open '" << tty << "'" ); }
            struct termios new_termios;
            ::tcgetattr( fd_, &old_termios_ );
            new_termios = old_termios_;
            new_termios.c_lflag &= ~( ICANON | ECHO );
            new_termios.c_iflag &= ~( BRKINT | ICRNL | INPCK | ISTRIP | IXON );
            if( ::tcsetattr( fd_, TCSANOW, &new_termios ) < 0 ) { COMMA_THROW( comma::exception, "failed to set '" << tty << "'" ); }
            std::cerr << "csv-play: running in interactive mode" << std::endl;
            interactive_help( "csv-play: " );
        }
        
        ~key_press_t_()
        {
            if( !interactive_ ) { return; }
            ::tcsetattr( STDIN_FILENO, TCSANOW, &old_termios_ ); // restore the console
            ::close( fd_ );
        }
        
        boost::optional< char > read()
        {
            if( !interactive_ ) { return boost::optional< char >(); }
            char c;
            int count = ::read( fd_, &c, 1 );
            if( count == 1 ) { return c; }
            return boost::optional< char >();
        }
        
    private:
        bool interactive_;
        int fd_;
        struct termios old_termios_;
    };

    key_press_t_ key_press_;
};

int main( int argc, char** argv )
{
    try
    {
        const boost::array< comma::signal_flag::signals, 2 > signals = { { comma::signal_flag::sigint, comma::signal_flag::sigterm } };
        comma::signal_flag shutdown_flag( signals );
        comma::command_line_options options( argc, argv, usage );
        if( options.exists( "--bash-completion" ) ) bash_completion( argc, argv );
        options.assert_mutually_exclusive( "--speed,--slow,--slowdown" );
        double speed = options.value( "--speed", 1.0 / options.value< double >( "--slow,--slowdown", 1.0 ) );
        double resolution = options.value< double >( "--resolution", 0.01 );
        std::string from = options.value< std::string>( "--from", "" );
        std::string to = options.value< std::string>( "--to", "" );
        bool quiet =  options.exists( "--quiet" );
        bool flush =  !options.exists( "--no-flush" );
        std::vector< std::string > configstrings = options.unnamed( "--verbose,-v,--interactive,-i,--paused,--paused-at-start,--quiet,--flush,--no-flush","--pause-at,--slow,--slowdown,--speed,--resolution,--binary,--fields,--clients,--from,--to" );
        if( configstrings.empty() ) { configstrings.push_back( "-;-" ); }
        comma::csv::options csv( argc, argv );
        csv.full_xpath = false;
        comma::name_value::parser name_value("filename,output", ';', '=', false );
        std::vector< comma::Multiplay::SourceConfig > sourceConfigs( configstrings.size() );
        comma::Multiplay::SourceConfig defaultConfig( "-", options.value( "--clients", 0 ), csv );
        for( unsigned int i = 0U; i < configstrings.size(); ++i ) { sourceConfigs[i] = name_value.get< comma::Multiplay::SourceConfig >( configstrings[i], defaultConfig ); }
        boost::posix_time::ptime fromtime;
        if( !from.empty() ) { fromtime = boost::posix_time::from_iso_string( from ); }
        boost::posix_time::ptime totime;
        if( !to.empty() ) { totime = boost::posix_time::from_iso_string( to ); }
        multiplay.reset( new comma::Multiplay( sourceConfigs, speed, quiet, boost::posix_time::microseconds( static_cast<unsigned int>( resolution * 1000000 )), fromtime, totime, flush ));
        if( options.exists( "--paused,--paused-at-start" )) { playback.pause(); }
        boost::optional< boost::posix_time::ptime > pause_at_timestamp = boost::make_optional( options.exists( "--pause-at" ), boost::posix_time::from_iso_string( options.value< std::string >( "--pause-at" )));
        key_press_handler_t key_press_handler(  options.exists( "--interactive,-i" )
                                             || options.exists( "--paused,--paused-at-start" )
                                             || options.exists( "--pause-at" ));
        while( !shutdown_flag && !quit && std::cout.good() )
        {
            boost::posix_time::ptime now = multiplay->now();
            key_press_handler.update( now );
            if( pause_at_timestamp && !now.is_not_a_date_time() && *pause_at_timestamp < now )
            {
                playback.pause( now );
                pause_at_timestamp = boost::none;
            }
            if( playback.is_paused() )
            {
                boost::this_thread::sleep( boost::posix_time::millisec( 200 ) );
                continue;
            }
            if( !multiplay->read() ) { break; }
            playback.has_read_once();
        }
        multiplay->close();
        multiplay.reset();
        if( shutdown_flag ) { std::cerr << "csv-play: interrupted by signal" << std::endl; return -1; }
        return 0;
    }
    catch( std::exception& ex ) { std::cerr << "csv-play: " << ex.what() << std::endl; }
    catch( ... ) { std::cerr << "csv-play: unknown exception" << std::endl; }
    try { if( multiplay ) { multiplay->close(); } } catch ( ... ) {} // windows thing
    return 1;
}
