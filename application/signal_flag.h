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

#ifndef COMMA_APPLICATION_SIGNALFLAG_H_
#define COMMA_APPLICATION_SIGNALFLAG_H_

#include <csignal>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <comma/base/exception.h>

namespace comma {

/// a simplistic singleton signal handler (e.g. for a shutdown flag)
struct signal_flag : public boost::noncopyable
{
    public:
        /// signals
        #ifdef WIN32
            enum signals { sigint = SIGINT, sigterm = SIGBREAK };
        #else
            enum signals { sigint = SIGINT, sigterm = SIGTERM, sigpipe = SIGPIPE /* etc */ };
        #endif
        
        /// constructor
        signal_flag()
        {
            #ifdef WIN32
            boost::array< signals, 2 > signals = { { sigint, sigterm } };
            #else
            boost::array< signals, 3 > signals = { { sigint, sigterm, sigpipe } };
            #endif 
            init_( signals );
        }

        /// constructor
        template < typename T >
        signal_flag( const T& signals ) { init_( signals ); }

        /// return true, if set
        bool is_set() const { return is_set_; }

        /// for those who does not like to type
        operator bool() const { return is_set_; }

        /// reset to false
        void reset() { is_set_ = false; }

    private:
        static bool is_set_;
        static void handle( int ) { is_set_ = true; }
        template < typename T >
        static bool init_( const T& signals ) { static bool r = register_once_< T >( signals ); return r; }
        template < typename T >
        static bool register_once_( const T& signals )
        {
            #ifndef WIN32
                struct sigaction sa;
                sa.sa_handler = handle;
                sigemptyset( &sa.sa_mask );
                sa.sa_flags = 0;
            #endif
            for( unsigned int i = 0; i < T::static_size; ++i )
            {
                #ifdef WIN32
                    // quick and dirty, use ::signal (but it won't handle Ctrl+C properly - fix it, when we know how)
                    // todo: restore from svn
                    if( ::signal( signals[i], handle_signal_ ) == SIG_ERR ) { COMMA_THROW( comma::exception, "failed to set handler for signal " << signals[i] ); }
                #else
                    if( ::sigaction( signals[i], &sa, NULL ) != 0 ) { COMMA_THROW( comma::exception, "failed to set handler for signal " << signals[i] ); }
                #endif
            }
            return true;
        }

    #ifdef WIN32
        static void handle_signal_( int ) { is_set_ = true; }
    #endif
};

} // namespace comma {

#endif // COMMA_APPLICATION_SIGNALFLAG_H_
