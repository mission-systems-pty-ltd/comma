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

#ifndef COMMA_SYNC_SYNCHRONIZED_HEADER_GUARD_
#define COMMA_SYNC_SYNCHRONIZED_HEADER_GUARD_

#include <boost/scoped_ptr.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace comma {

/// wrapper synchronizing access to the whole class, rather
/// than to some of its methods, as otherwise we would need
/// to write a wrapper exposing all the class's methods
/// protected by a mutex; owns the class;
///
/// see unit test for examples
template < typename T >
class synchronized
{
    public:
        /// constructors
        synchronized() : t_( new T ) {}
        synchronized( T* t ) : t_( t ) {}
        template < typename A1 > synchronized( A1 a1 ) : t_( new T( a1 ) ) {}
        template < typename A1, typename A2 > synchronized( A1 a1, A2 a2 ) : t_( new T( a1, a2 ) ) {}
        template < typename A1, typename A2, typename A3 > synchronized( A1 a1, A2 a2, A3 a3 ) : t_( new T( a1, a2, a3 ) ) {}
        template < typename A1, typename A2, typename A3, typename A4 > synchronized( A1 a1, A2 a2, A3 a3, A4 a4 ) : t_( new T( a1, a2, a3, a4 ) ) {}
        // etc as needed

        /// lock
        void lock() const { mutex_.lock(); }

        /// unlock
        void unlock() const { mutex_.unlock(); }

        /// accessor class
        class scoped_transaction
        {
            public:
                /// constructor, locks mutex
                scoped_transaction( synchronized& s ) : synchronized_( s ) { synchronized_.lock(); }

                /// destructor, unlocks mutex
                ~scoped_transaction() { synchronized_.unlock(); }

                /// access operators
                T& operator*() { return *synchronized_.t_; }
                const T& operator*() const { return *synchronized_.t_; }
                T* operator->() { return synchronized_.t_.get(); }
                const T* operator->() const { return synchronized_.t_.get(); }

            private:
                synchronized& synchronized_;

        };

        /// accessor class
        class const_scoped_transaction
        {
            public:
                /// constructor, locks mutex
                const_scoped_transaction( const synchronized& s ) : synchronized_( s ) { synchronized_.lock(); }

                /// destructor, unlocks mutex
                ~const_scoped_transaction() { synchronized_.unlock(); }

                /// access operators
                const T& operator*() const { return *synchronized_.t_; }
                const T* operator->() const { return synchronized_.t_.get(); }

            private:
                const synchronized& synchronized_;

        };

    private:
        friend class scoped_transaction;
        friend class const_scoped_transaction;
        boost::scoped_ptr< T > t_;
        mutable boost::recursive_mutex mutex_;
};

} // namespace comma {

#endif // COMMA_SYNC_SYNCHRONIZED_HEADER_GUARD_
