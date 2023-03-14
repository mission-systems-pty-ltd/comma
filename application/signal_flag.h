// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#ifndef COMMA_APPLICATION_SIGNALFLAG_H_
#define COMMA_APPLICATION_SIGNALFLAG_H_

#include <csignal>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include "../base/exception.h"

namespace comma {

/// a simplistic singleton signal handler (e.g. for a shutdown flag)
struct signal_flag : public boost::noncopyable
{
    public:
        /// signals
        #ifdef WIN32
            enum signals { sigint = SIGINT, sigterm = SIGBREAK };
        #else
            enum signals { sigint = SIGINT, sigterm = SIGTERM, sigpipe = SIGPIPE, sighup = SIGHUP /* etc */ };
        #endif

        enum sigtype { soft = 0, hard = 1 };
        
        /// constructor
        signal_flag(sigtype stype = soft);

        /// constructor
        template < typename T >
        signal_flag( const T& signals, sigtype stype=soft ) { sigtype_ = stype; init_( signals ); }

        /// return true, if set
        bool is_set() const { return is_set_; }

        /// for those who does not like to type
        operator bool() const { return is_set_; }

        /// reset to a given value
        void reset( bool value = false ) { is_set_ = value; }

    private:
        static bool is_set_;
        static void handle( int sig);
        static sigtype sigtype_;
        
        template < typename T >
        static bool init_( const T& signals ) { static bool r = register_once_< T >( signals ); return r; }
        
        template < typename T >
        static bool register_once_( const T& signals )
        {
            #ifndef WIN32
                struct sigaction sa;
                sa.sa_handler = handle;
                sigemptyset( &sa.sa_mask );
                sa.sa_flags = (sigtype_ == hard)? SA_RESETHAND : 0;
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
