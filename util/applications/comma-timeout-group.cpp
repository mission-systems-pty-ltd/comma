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

#include <string.h>
#include <signal.h>
#include <time.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include "../../application/contact_info.h"
#include "../../application/command_line_options.h"
#include "../../base/exception.h"
#include "../../base/types.h"
#include "../../csv/stream.h"
#include "../../csv/impl/epoch.h"
#include "../../string/string.h"
#include "../../visiting/traits.h"

namespace {

void usage( bool )
{
    static char const * const msg_general =
        "\nRun the given command with its arguments. Kill the command if it is still running after the given"
        "\ntime duration."
        "\n"
        "\nUsage:"
        "\n    comma-timeout-group <options> duration command <args>"
        "\n"
        "\nThis program aims to be a drop-in replacement of the standard timeout(1) utility for most common"
        "\nusage patterns. The capability to wait for all processes in a process group added."
        "\nSome options of timeout (1) are not supported, and, if given, this utility exits in error."
        "\n"
        "\nOptions:"
        "\n    -h,--help, print this help and exit"
        "\n    --verbose, chat more"
        "\n    --preserve-status, not supported"
        "\n    --foreground, not supported"
        "\n    -k, --kill-after=duration, if the command is still running this long after the initial"
        "\n        signal was sent, send the KILL signal to finish it off"
        "\n    --wait-for-process-group=duration, after the initial signal, wait this time for all the processes"
        "\n        in the current process group to finish; if some processes are still left, send the KILL signal"
        "\n        to finish them off (same as -k duration); if both this option and '-k' is given, the duration"
        "\n        specified last on the command line takes precedence"
        "\n    -s, --signal=signal, the signal to be sent on timeout, given as a name (HUP) or number;"
        "\n        only a sub-set of all available signal names is supported, use '--list-known-signals' to list;"
        "\n        if signal specified as a number, arbitrary signal can be used, see 'kill -l' for the values;"
        "\n        by default, use SIGTERM"
        "\n    --list-known-signals, list the supported signals and exit"
        "\n"
        "\nDuration for the '-k' and '--wait-for-process-group' options is a floating point number with"
        "\nan optional suffix 's' for seconds (default), 'm' for minutes, 'h' for hours, and 'd' for days."
        "\nThe '--wait-for-process-group' option also accepts special duration 'forever' (equal to DBL_MAX)"
        "\ngiven as a literal string (no quotes)."
        "\n"
        "\nIf both '--kill-after and --wait-for-process-group' are specified, the former takes precedence."
        "\n"
        "\nReturn value:"
        "\n    - if the command times out, exit with status 124"
        "\n    - if the command does not exit on the first signal, and the KILL signal is sent, exit with"
        "\n      status 128+9"
        "\n    - otherwise, exit with the status of command"
        "\n"
        "\nSimple examples:"
        "\n    comma-timeout-group 10 sleep 1  # exits after 1 second with status 0"
        "\n    comma-timeout-group 1 sleep 10  # exits after 1 second with status 124"
        "\n"
        "\nMore complex examples:"
        "\n    comma-timeout-group -k 5 --signal=USR1 10 bash -c \"bash_function 1 2\""
        "\n        run the given bash function with arguments '1 2'; the function must be 'export -f'-ed"
        "\n        send the bash process the USR1 signal if it is still running in 10 s after start"
        "\n        send the entire process group the KILL signal if it is still running after another 5 s"
        "\n"
        "\n";
    std::cerr << msg_general << comma::contact_info << std::endl << std::endl;
    exit( 0 );
}

double seconds_from_string( const std::string& s, bool allow_forever = false )
{
    struct impl_ {
        static double from_string( const std::string & s )
        {
            const char & w = *s.rbegin();
            try {
                switch( w )
                {
                    case 'd':
                        return 60 * 60 * 24 * boost::lexical_cast< double >( static_cast< const char * >( &s[0] ), s.size() - 1 );
                    case 'h':
                        return 60 * 60 * boost::lexical_cast< double >( static_cast< const char * >( &s[0] ), s.size() - 1 );
                    case 'm':
                        return 60 * boost::lexical_cast< double >( static_cast< const char * >( &s[0] ), s.size() - 1 );
                    case 's':
                        return boost::lexical_cast< double >( static_cast< const char * >( &s[0] ), s.size() - 1 );
                    default:
                        return boost::lexical_cast< double >( s );
                }
            } catch ( boost::bad_lexical_cast & e ) {
                COMMA_THROW( comma::exception, "cannot convert '" << s << "' to seconds, " << e.what() );
            }
        }
    };
    if ( s.empty() )
    {
        COMMA_THROW( comma::exception, "expected non-empty string" );
    }
    if ( s == "forever" ) {
        if ( !allow_forever ) { COMMA_THROW( comma::exception, "the 'forever' duration is not allowed in this context" ); }
        return DBL_MAX;
    }
    double result = impl_::from_string( s );
    if ( result <= 0.0 )
    {
        COMMA_THROW( comma::exception, "negative time interval '" << s << "'" );
    }
    return result;
}

struct sig2str {
    // use vector and not map to keep things sorted by signal value (int)
    // size is tiny, map probably gives little speed-up in lookup
    typedef std::vector< std::pair< std::string, int > > V;

    static std::string list_all()
    {
        std::vector< std::string > names;
        names.reserve( known_signals.size() );
        for ( V::const_iterator i = known_signals.begin(); i != known_signals.end(); ++i ) { names.push_back( i->first ); }
        return comma::join( names, ',' );
    }

    static int from_string( const std::string & s ) {
        try {
            return boost::lexical_cast< int >( s );
        }
        catch ( boost::bad_lexical_cast & ) {
            // assume is a name
            return from_named_string( s, true );
        }
    }

    private:
        static V known_signals;

        static int from_named_string( const std::string & s, bool recurse ) {
            if ( recurse && 0 == s.compare( 0, 3, "SIG" ) ) {
                // no recursion to disable SIGSIGTERM
                return from_named_string( s.substr( 3 ), false );
            }
            for ( V::const_iterator i = known_signals.begin(); i != known_signals.end(); ++i ) { if ( i->first == s ) return i->second; }
            COMMA_THROW( comma::exception, "unknown signal '" << ( recurse ? "" : "SIG" ) << s << "'" );
        }
};

sig2str::V sig2str::known_signals = boost::assign::list_of< std::pair< std::string, int > > \
           (  "HUP",  SIGHUP ) \
           (  "INT",  SIGINT ) \
           ( "KILL", SIGKILL ) \
           ( "USR1", SIGUSR1 ) \
           ( "USR2", SIGUSR2 ) \
           ( "ALRM", SIGALRM ) \
           ( "TERM", SIGTERM ) \
           ( "CONT", SIGCONT ) \
           ( "STOP", SIGSTOP ) ;

} // anonymous

int main( int ac, char** av ) try
{
    bool timed_out;
    int signal_to_use = SIGTERM;  // same default as kill and timeout commands
    int child_pid;
    double kill_after = 0.0;
    bool wait_for_process_group = false;

    comma::command_line_options options( ac, av, usage );
    if ( options.exists( "-h,--help" ) ) { usage( true ); return 0; }
    if ( options.exists( "--list-known-signals" ) ) { std::cout << sig2str::list_all() << std::endl; return 0; }
    if ( options.exists( "--preserve-status,--foreground" ) ) { usage( true ); COMMA_THROW( comma::exception, "unsupported option of the original timeout" ); }

    bool verbose = options.exists( "--verbose" );

    if ( options.exists( "-s,--signal" ) ) {
        signal_to_use = sig2str::from_string( options.value< std::string >( "--signal" ) );
        if ( verbose ) { std::cerr << "comma-timeout-group: will use signal " << signal_to_use << std::endl; }
    if ( options.exists( "--wait-for-process-group" ) ) {
        wait_for_process_group = true;
        kill_after = seconds_from_string( options.value< std::string >( "--wait-for-process-group" ), true );
    }

    if ( options.exists( "-k,--kill-after" ) ) { kill_after = seconds_from_string( options.value< std::string >( "--kill-after" ) ); }

    }

    return 0;
}
catch( std::exception& ex ) { std::cerr << "comma-timeout-group: " << ex.what() << std::endl; }
catch( ... ) { std::cerr << "comma-timeout-group: unknown exception" << std::endl; }
