// Copyright (c) 2011 The University of Sydney

/// @author vsevolod vlaskine

#pragma once

#ifdef WIN32
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#endif

#include <fstream>
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>
#include "../base/exception.h"
#include "../csv/ascii.h"
#include "../csv/binary.h"
#include "../csv/options.h"
#include "../string/string.h"

namespace comma { namespace csv {

/// @todo document
namespace detail { void unsynchronize_with_stdio(); }

/// generic input stream
template < typename S > class input_stream;

/// generic output stream
template < typename S > class output_stream;

/// tied input and output streams
template < typename S, typename T > class tied;

/// input stream passed through
template < typename S > class passed;

/// convenience functions: read input stream into a container that has push_back() method
template < typename V > V read_as( std::istream& is, const options& o = options() );
template < typename V > V read_as( const std::string& filename, const options& o = options() );
template < typename V > V read_as( std::istream& is, const options& o, const typename V::value_type& default_value );
template < typename V > V read_as( const std::string& filename, const options& o, const typename V::value_type& default_value );

/// ascii csv input stream
template < typename S >
class ascii_input_stream : public boost::noncopyable
{
    public:
        /// constructor
        ascii_input_stream( std::istream& is, const std::string& column_names = "", char delimiter = ',', bool full_path_as_name = true, const S& sample = S() );

        /// constructor from csv options
        ascii_input_stream( std::istream& is, const options& o, const S& sample = S() );

        /// constructor from csv options
        ascii_input_stream( std::istream& is, const S& sample = S() );

        /// read; return NULL, if end of stream or alike
        const S* read();

        /// read with timeout; return NULL, if insufficient data (e.g. end of stream)
        /// @todo implement
        const S* read( const boost::posix_time::ptime& timeout );

        /// return the last line read
        const std::vector< std::string >& last() const { return line_; }

        /// a helper: return the engine
        const csv::ascii< S > ascii() const { return ascii_; }

        /// return fields
        const std::vector< std::string >& fields() const { return fields_; }

        /// returns true if the stream has data in its buffer to be read
        /// the buffered data may not contain a complete line and a subsequent read (getline) may block
        /// this is under the consideration that the ascii_input_stream is mainly used for
        /// debug purposes only
        bool ready() const;

    private:
        friend class input_stream<S>;
        template < typename W, typename T >
        friend class tied;
        template < typename V, typename T, typename Data >
        friend void append( const input_stream< V >& is, output_stream< T >& os, const Data& data );

        std::istream& is_;
        csv::ascii< S > ascii_;
        const S default_;
        S result_;
        std::vector< std::string > line_;
        std::vector< std::string > fields_;
};

/// ascii csv output stream
template < typename S >
class ascii_output_stream : public boost::noncopyable
{
    public:
        /// constructor
        ascii_output_stream( std::ostream& os, const std::string& column_names = "", char delimiter = ',', bool full_path_as_name = true, const S& sample = S() );

        /// constructor from csv options
        ascii_output_stream( std::ostream& os, const options& o, const S& sample = S() );

        /// constructor from csv options
        ascii_output_stream( std::ostream& os, const S& sample = S() );

        /// write
        void write( const S& s );

        /// substitute corresponding fields in the line and write
        void write( const S& s, const std::string& line );

        /// substitute corresponding fields and write
        void write( const S& s, const std::vector< std::string >& line );

        /// substitute corresponding fields and write
        void write( const S& s, std::vector< std::string >& line );

        /// flush
        /// @todo: to implement
        void flush() {}

        /// set precision
        void precision( unsigned int p ) { ascii_.precision( p ); }

        /// a helper: return the engine
        const csv::ascii< S > ascii() const { return ascii_; }

        /// return fields
        const std::vector< std::string >& fields() const { return fields_; }

    private:
        friend class output_stream<S>;
        template < typename W, typename T>
        friend class tied;
        template < typename V, typename T, typename Data >
        friend void append( const input_stream< V >& is, output_stream< T >& os, const Data& data );

        std::ostream& os_;
        csv::ascii< S > ascii_;
        std::vector< std::string > fields_;
};

/// binary csv input stream
template < typename S >
class binary_input_stream : public boost::noncopyable
{
    public:
        /// constructor
        binary_input_stream( std::istream& is, const std::string& format = "", const std::string& column_names = "", bool full_path_as_name = true, const S& sample = S() );

        /// constructor from options
        binary_input_stream( std::istream& is, const options& o, const S& sample = S() );

        /// read; return NULL, if insufficient data (e.g. end of stream)
        const S* read();

        /// read with timeout; return NULL, if insufficient data (e.g. end of stream)
        /// @todo implement
        const S* read( const boost::posix_time::ptime& timeout );

        /// return the last line read
        const char* last() const { return &buf_[0]; }

        /// a helper: return the engine
        const csv::binary< S > binary() const { return binary_; }

        /// return size
        std::size_t size() const { return size_; }

        /// return fields
        const std::vector< std::string >& fields() const { return fields_; }

        /// return true, if read will not block
        bool ready() const;

    private:
        friend class input_stream<S>;
        template < typename W, typename T>
        friend class tied;
        template < typename V, typename T, typename Data >
        friend void append( const input_stream< V >& is, output_stream< T >& os, const Data& data );

        std::istream& is_;
        csv::binary< S > binary_;
        const S default_;
        S result_;
        const std::size_t size_;
        std::vector< char > buf_;
        std::vector< std::string > fields_;
};

/// binary csv output stream
template < typename S >
class binary_output_stream : public boost::noncopyable
{
    public:
        /// constructor
        binary_output_stream( std::ostream& os, const std::string& format = "", const std::string& column_names = "", bool full_path_as_name = true, bool flush = false, const S& sample = S() );

        /// constructor from options
        binary_output_stream( std::ostream& os, const options& o, const S& sample = S() );

        /// destructor
        ~binary_output_stream() { flush(); }

        /// write
        void write( const S& s );

        /// substitute corresponding fields in the buffer and write
        void write( const S& s, const char* buf );

        /// flush
        void flush();

        /// a helper: return the engine
        const csv::binary< S > binary() const { return binary_; }

        /// return fields
        const std::vector< std::string >& fields() const { return fields_; }

    private:
        template < typename W, typename T>
        friend class tied;
        friend class output_stream< S >;
        template < typename V, typename T, typename Data >
        friend void append( const input_stream< V >& is, output_stream< T >& os, const Data& data );

        std::ostream& os_;
        csv::binary< S > binary_;
        //const std::size_t size_;
        std::vector< char > buf_;
        //char* begin_;
        //const char* end_;
        //char* cur_;
        std::vector< std::string > fields_;
        bool flush_;
        /// bool is_stdout;
};

/// trivial generic csv input stream wrapper, less optimized, but more convenient
template < typename S >
class input_stream : public boost::noncopyable
{
    public:
        /// construct from ascii stream
        input_stream( ascii_input_stream< S >* is ) : ascii_( is ) {}

        /// construct from binary stream
        input_stream( binary_input_stream< S >* is ) : binary_( is ) {}

        /// construct from csv options
        input_stream( std::istream& is, const csv::options& o, const S& sample = S() );

        /// construct ascii stream from default csv options
        input_stream( std::istream& is, const S& sample = S() );

        /// read; return NULL, if insufficient data (e.g. end of stream)
        const S* read() { return ascii_ ? ascii_->read() : binary_->read(); }

        /// read with timeout; return NULL, if insufficient data (e.g. end of stream)
        const S* read( const boost::posix_time::ptime& timeout ) { return ascii_ ? ascii_->read( timeout ) : binary_->read( timeout ); }

        /// return fields
        const std::vector< std::string >& fields() const { return ascii_ ? ascii_->fields() : binary_->fields(); }

        /// a convenience function; return last read record as a string containg the original string for ascii and the original binary buffer for binary, may be slow
        std::string last() const;

        const ascii_input_stream< S >& ascii() const { return *ascii_; }

        const binary_input_stream< S >& binary() const { return *binary_; }

        ascii_input_stream< S >& ascii() { return *ascii_; }

        binary_input_stream< S >& binary() { return *binary_; }

        bool is_binary() const { return bool( binary_ ); }

        bool ready() const { return binary_ ? binary_->ready() : ascii_->ready(); }

    private:
        boost::scoped_ptr< ascii_input_stream< S > > ascii_;
        boost::scoped_ptr< binary_input_stream< S > > binary_;
};

/// trivial generic csv output stream wrapper, less optimized, but more convenient
template < typename S >
class output_stream : public boost::noncopyable
{
    public:
        /// construct from ascii stream
        output_stream( ascii_output_stream< S >* os ) : ascii_( os ) {}

        /// construct from binary stream
        output_stream( binary_output_stream< S >* os ) : binary_( os ) {}

        /// construct ascii stream from default csv options
        output_stream( std::ostream& os, const S& sample = S() );

        /// construct from csv options
        output_stream( std::ostream& os, const csv::options& o, const S& sample = S() );

        output_stream( std::ostream& os, bool binary, bool full_xpath = true, bool flush = false, const S& sample = S() );

        /// write
        void write( const S& s ) { if( ascii_ ) { ascii_->write( s ); } else { binary_->write( s ); } }

        /// write, substituting corresponding fields in given line
        void write( const S& s, const char* line ) { if( ascii_ ) { ascii_->write( s, line ); } else { binary_->write( s, line ); } }

        /// write, substituting corresponding fields in given line
        void write( const S& s, const std::string& line ) { if( ascii_ ) { ascii_->write( s, line ); } else { binary_->write( s, &line[0] ); } }

        /// write, substituting corresponding fields in given line
        void write( const S& s, const std::vector< std::string >& line ) { ascii_->write( s, line ); }

        /// write, substituting corresponding fields in the last record read from the input
        void write( const S& s, const input_stream< S >& istream ) { if( binary_ ) { binary_->write( s, istream.binary().last() ); } else { ascii_->write( s, istream.ascii().last() ); } }

        /// append record s to line and write them to output stream
        /// for ascii stream, line should not have end of line character at the end
        void append(const std::string& line, const S& s);

        /// flush
        void flush() { if( ascii_ ) { ascii_->flush(); } else { binary_->flush(); } }

        /// return fields
        const std::vector< std::string >& fields() const { return ascii_ ? ascii_->fields() : binary_->fields(); }

        const ascii_output_stream< S >& ascii() const { return *ascii_; }

        const binary_output_stream< S >& binary() const { return *binary_; }

        ascii_output_stream< S >& ascii() { return *ascii_; }

        binary_output_stream< S >& binary() { return *binary_; }

        bool is_binary() const { return bool( binary_ ); }

        std::ostream& os() { return binary_ ? binary_->os_ : ascii_->os_; }

    private:
        boost::scoped_ptr< ascii_output_stream< S > > ascii_;
        boost::scoped_ptr< binary_output_stream< S > > binary_;
};

template < typename S >
inline void output_stream< S >::append( const std::string& line, const S& s ) 
{
    if( !is_binary() )
    {
        ascii().os_.write( &line[0], line.size() );
        ascii().os_ << ascii().ascii().delimiter();
    }
    else
    {
        /// do not do it unless a properly scalable solution devised; see generic backlog; also see the notes inside the passed<> implementation
        /// auto& bos = binary();
        /// if ( bos.is_stdout ) {
        ///     ::write( 1, &line[0], line.size() );
        /// } else {
        ///    bos.os_.write(&line[0], line.size());
        /// }
        binary().os_.write( &line[0], line.size() );
    }
    write( s );
}

/// append record s to last record from input stream and and write them to output
template < typename S, typename T, typename Data >
inline void append( const input_stream< S >& is, output_stream< T >& os, const Data& data )
{
    if( is.is_binary() )
    {
        /// do not do it unless a properly scalable solution devised; see generic backlog; also see the notes inside the passed<> implementation
        /// auto& bos = os.binary();
        /// if ( bos.is_stdout ) {
        ///     ::write( 1, is.binary().last(), is.binary().size() );
        /// } else {
        ///     bos.os_.write( is.binary().last(), is.binary().size() );
        /// }
        os.binary().os_.write( is.binary().last(), is.binary().size() );
        os.write( data );  // todo: low-hanging fruit for append() only: add writing to stdout as a private method or alike (i.e. hide from the user) and still use ::write() for stdout
    }
    else
    {
        std::string sbuf;
        os.ascii().ascii().put( data, sbuf );
        os.ascii().os_ << comma::join( is.ascii().last(), os.ascii().ascii().delimiter() ) << os.ascii().ascii().delimiter() << sbuf << std::endl;
    }
}

/// append a columns of output to last record of input to write to output stream
template < typename S, typename T >
class tied
{
    public:
        tied( const input_stream< S >& is, output_stream< T >& os ) : is_( is ), os_( os ) { }

        /// append record s to last record from input stream and and write them to output
        void append( const T& data ) { os_.append(is_.last(),data); }

    private:
        const input_stream< S >& is_;
        output_stream< T >& os_;

};

template < typename S, typename T >
inline tied< S, T > make_tied( const input_stream< S >& is, output_stream< T >& os ) { return tied< S, T >( is, os ); }

template < typename S, typename T >
inline tied< S, T > tie( const input_stream< S >& is, output_stream< T >& os ) { return make_tied( is, os ); }

template < typename S >
class passed
{
    public:
        passed( const input_stream< S >& is, std::ostream& os, bool flush = false );

        void write();
        
        void write( const S& s );

    private:
        const input_stream< S >& is_;
        std::ostream& os_;
        std::string buffer_;
        bool flush;
        bool is_stdout_;
};

template < typename S >
inline passed< S >::passed( const input_stream< S >& is, std::ostream& os, bool flush ) : is_( is ), os_( os ), flush( flush )
{
    #ifdef WIN32
    if( is_.is_binary() && os == std::cout ) { _setmode( _fileno( stdout ), _O_BINARY ); }
    #endif // #ifdef WIN32
// In using view-points in 'pass' mode there were issues with the write method.
// How to reproduce: take some nav data in t,6d format. Store as in.bin and other.bin.
//
// cat in.bin | view-points "-;fields=,x,y,z;binary=t,6d;pass" "other.bin;fields=,x,y,z;binary=t,6d;colour=grey" > out.bin
//
// The files in.bin and out.bin are randomly different; out.bin misses some of the input records.
//
// The same problem was earlier seen in basler-cat utility from the snark library.
// Quote:
//
// In testing basler-cat with the Pika XC2 camera we saw issues with the write() method.
//
// Specifically, using a command line like:
//     basler-cat "log=log.bin" > stdout.bin
// we would witness stdout.bin as corrupted, log.bin would be fine.
//
// Note that on the Pika XC2 the frame rate is high (150fps) and the image size large
// (2354176 bytes) which might contribute to the problem. The target was an SSD drive.
//
// In detail:
// * sometimes (every few seconds), the header (20 bytes) would be written but the body would not
// * os.write( body) was called but os.tellp() would indicate that no frames were written
// * os.fail() and os.bad() were always false
// * strangely, writing a large (1M) amount of random data to another diskfile stopped the problem
// * we tried various combinations of flushing data with no success
// * switching from std::cout.write() to write( 1, ... ) was successful
//
// We don't understand why std::cout would exhibit this behaviour so for now we
// will have a second method (below) that uses the c-style write() function.
// This is used by pipeline::write_()
//
// End of quote.
//
// We attempted a modified implementation that handles std::cout explicitly by deferring to C-level write.
// However this approaches exhibits a very nasty side effect that renders it almost worse then the original
// problem:
//  - the user application may write to std::cout on its own, directly, not using csv::stream facilities
//  - if so, there would be writes to file descriptor 1 (C-level write) and std::cout
//  - these two streams are not synchronized; as the result, the output is randomly garbled and interleaved
//  - there is no way to prohibit the application from using std::cout explicitly, and potentially
//    there could be dozens of applications affected with with subtle effect
//  - according to git grep, only view-points was using this class template at the moment; therefore,
//    the change is very localized and we preserve it in this class
//  - however, all the other similar modifications have been commented out using /// symbol
    is_stdout_ = os.rdbuf() == std::cout.rdbuf();
}

template < typename S >
inline void passed< S >::write()
{
    if( is_.is_binary() )
    {
        if( is_stdout_ )
        {
            auto r = ::write( 1, is_.binary().last(), is_.binary().size() );
            ( void ) r;
            if( flush ) { ::fflush( stdout ); }
        }
        else
        {
            os_.write( is_.binary().last(), is_.binary().size() );
            if( flush ) { os_.flush(); }
        }
    }
    else
    {
        os_ << comma::join( is_.ascii().last(), is_.ascii().ascii().delimiter() ) << std::endl;
    }
}

template < typename S >
inline void passed< S >::write( const S& s )
{
    if( is_.is_binary() )
    {
        buffer_.resize( is_.binary().size() );
        ::memcpy( &buffer_[0], is_.binary().last(), is_.binary().size() ); // quick and dirty
        is_.binary().binary().put( s, &buffer_[0] );
        if( is_stdout_ )
        {
            auto r = ::write( 1, &buffer_[0], is_.binary().size() );
            ( void ) r;
            if( flush ) { ::fflush( stdout ); }
        }
        else
        {
            os_.write( &buffer_[0], is_.binary().size() );
            if( flush ) { os_.flush(); }
        }
    }
    else
    {
        std::vector< std::string > v = is_.ascii().last();
        is_.ascii().ascii().put( s, v );
        os_ << comma::join( v, is_.ascii().ascii().delimiter() ) << std::endl;
    }
}

template < typename S >
inline ascii_input_stream< S >::ascii_input_stream( std::istream& is, const std::string& column_names, char delimiter, bool full_path_as_name, const S& sample )
    : is_( is )
    , ascii_( column_names, delimiter, full_path_as_name, sample )
    , default_( sample )
    , result_( sample )
    , fields_( split( column_names, ',' ) )
{
    detail::unsynchronize_with_stdio();
}

template < typename S >
inline ascii_input_stream< S >::ascii_input_stream(std::istream& is, const options& o, const S& sample )
    : is_( is )
    , ascii_( o, sample )
    , default_( sample )
    , result_( sample )
    , fields_( split( o.fields, ',' ) )
{
    detail::unsynchronize_with_stdio();
}

template < typename S >
inline ascii_input_stream< S >::ascii_input_stream(std::istream& is, const S& sample )
    : is_( is )
    , ascii_( options().fields, options().delimiter, true, sample ) // , ascii_( options().fields, options().delimiter, o.full_xpath, sample )
    , default_( sample )
    , result_( sample )
    , fields_( split( options().fields, ',' ) )
{
    detail::unsynchronize_with_stdio();
}

template < typename S >
inline bool ascii_input_stream< S >::ready() const
{
    return is_.rdbuf()->in_avail() > 0;
}

template < typename S >
inline const S* ascii_input_stream< S >::read()
{
    while( is_.good() && !is_.eof() )
    {
        /// @todo implement reassembly
        std::string s;
        std::getline( is_, s );
        if( !s.empty() && *s.rbegin() == '\r' ) { s = s.substr( 0, s.length() - 1 ); } // windows... sigh...
        if( s.empty() ) { continue; }
        result_ = default_;
        line_ = split( s, ascii_.delimiter() );
        ascii_.get( result_, line_ );
        return &result_;
    }
    return NULL;
}

template < typename S >
inline ascii_output_stream< S >::ascii_output_stream( std::ostream& os, const std::string& column_names, char delimiter, bool full_path_as_name, const S& sample )
    : os_( os )
    , ascii_( column_names, delimiter, full_path_as_name, sample )
    , fields_( split( column_names, ',' ) )
{
}

template < typename S >
inline ascii_output_stream< S >::ascii_output_stream( std::ostream& os, const comma::csv::options& o, const S& sample )
    : os_( os )
    , ascii_( o, sample )
    , fields_( split( o.fields, ',' ) )
{
}

template < typename S >
inline ascii_output_stream< S >::ascii_output_stream( std::ostream& os, const S& sample )
    : os_( os )
    , ascii_( options().fields, options().delimiter, true, sample ) // , ascii_( options().fields, options().delimiter, o.full_xpath, sample )
    , fields_( split( options().fields, ',' ) )
{
}

template < typename S >
inline void ascii_output_stream< S >::write( const S& s )
{
    std::vector< std::string > v;
    write( s, v );
}

template < typename S >
inline void ascii_output_stream< S >::write( const S& s, const std::string& line )
{
    write( s, split
( line, ascii_.delimiter() ) );
}

template < typename S >
inline void ascii_output_stream< S >::write( const S& s, const std::vector< std::string >& line )
{
    std::vector< std::string > v( line );
    write( s, v );
}

template < typename S >
inline void ascii_output_stream< S >::write( const S& s, std::vector< std::string >& v )
{
    ascii_.put( s, v );
    if( v.empty() ) { return; } // never here, though
    os_ << v[0];
    for( std::size_t i = 1; i < v.size(); ++i ) { os_ << ascii_.delimiter() << v[i]; }
    os_ << std::endl;
}

template < typename S >
inline binary_input_stream< S >::binary_input_stream( std::istream& is, const std::string& format, const std::string& column_names, bool full_path_as_name, const S& sample )
    : is_( is )
    , binary_( format, column_names, full_path_as_name, sample )
    , default_( sample )
    , result_( sample )
    , size_( binary_.format().size() )
    , buf_( size_ )
    , fields_( split( column_names, ',' ) )
{
    #ifdef WIN32
    if( &is == &std::cin ) { _setmode( _fileno( stdin ), _O_BINARY ); }
    #endif
    detail::unsynchronize_with_stdio();
}

template < typename S >
inline binary_input_stream< S >::binary_input_stream( std::istream& is, const options& o, const S& sample )
    : is_( is )
    , binary_( o.format().string(), o.fields, o.full_xpath, sample )
    , default_( sample )
    , result_( sample )
    , size_( binary_.format().size() )
    , buf_( size_ )
    , fields_( split( o.fields, ',' ) )
{
    #ifdef WIN32
    if( &is == &std::cin ) { _setmode( _fileno( stdin ), _O_BINARY ); }
    #endif
    detail::unsynchronize_with_stdio();
}

template < typename S >
inline bool binary_input_stream< S >::ready() const
{
    return is_.rdbuf()->in_avail() >= int( size_ );
}

template < typename S >
inline const S* binary_input_stream< S >::read()
{
    is_.read( &buf_[0], size_ );
    if( is_.gcount() == 0 ) { return NULL; }
    if( is_.gcount() != int( size_ ) ) { COMMA_THROW( comma::exception, "expected " << size_ << " bytes; got " << is_.gcount() ); }
    result_ = default_;
    binary_.get( result_, &buf_[0] );
    return &result_;
}

template < typename S >
inline binary_output_stream< S >::binary_output_stream( std::ostream& os, const std::string& format, const std::string& column_names, bool full_path_as_name, bool flush, const S& sample )
    : os_( os )
    , binary_( format, column_names, full_path_as_name, sample )
    //, size_( binary_.format().size() * ( 4098 / binary_.format().size() ) ) // quick and dirty
    , buf_( binary_.format().size() ) //, buf_( size_ )
    //, begin_( &buf_[0] )
    //, end_( begin_ + size_ )
    //, cur_( begin_ )
    , fields_( split( column_names, ',' ) )
    , flush_( flush )
    /// , is_stdout( os_.rdbuf() == std::cout.rdbuf() )
{
    #ifdef WIN32
    if( &os == &std::cout ) { _setmode( _fileno( stdout ), _O_BINARY ); }
    else if( &os == &std::cerr ) { _setmode( _fileno( stderr ), _O_BINARY ); }
    #endif
}

template < typename S >
inline binary_output_stream< S >::binary_output_stream( std::ostream& os, const options& o, const S& sample )
    : os_( os )
    , binary_( o.format().string(), o.fields, o.full_xpath, sample )
//     , size_( binary_.format().size() ) //, size_( binary_.format().size() * ( 4098 / binary_.format().size() ) ) // quick and dirty
    , buf_( binary_.format().size() ) //, buf_( size_ )
//     , begin_( &buf_[0] )
//     , end_( begin_ + size_ )
//     , cur_( begin_ )
    , fields_( split( o.fields, ',' ) )
    , flush_( o.flush )
    /// , is_stdout( os_.rdbuf() == std::cout.rdbuf() )
{
    #ifdef WIN32
    if( &os == &std::cout ) { _setmode( _fileno( stdout ), _O_BINARY ); }
    else if( &os == &std::cerr ) { _setmode( _fileno( stderr ), _O_BINARY ); }
    #endif
}

template < typename S >
inline void binary_output_stream< S >::flush()
{
    os_.flush();
//     if( cur_ == begin_ ) { return; }
//     os_.write( begin_, cur_ - begin_ );
//     os_.flush();
//     cur_ = begin_;
}

template < typename S >
inline void binary_output_stream< S >::write( const S& s )
{
    binary_.put( s, &buf_[0] );
    /// if ( is_stdout ) {
        /// do not do it! see the notes inside the passed<> implementation
        /// ::write( 1, &buf_[0], binary_.format().size() );
        /// if( flush_ ) { ::fflush( stdout ); }
    /// } else {
        os_.write( &buf_[0], binary_.format().size() );
        if( flush_ ) { os_.flush(); }
    /// }
//     binary_.put( s, cur_ );
//     cur_ += binary_.format().size();
//     if( cur_ == end_ ) { flush(); }
}

template < typename S >
inline void binary_output_stream< S >::write( const S& s, const char* buf )
{
    ::memcpy( &buf_[0], buf, binary_.format().size() );
    write( s );
//     ::memcpy( cur_, buf, binary_.format().size() );
//     write( s );
//    if( flush_ ) { os_.flush(); }
}

template < typename S >
inline input_stream< S >::input_stream( std::istream& is, const csv::options& o, const S& sample )
{
    if( o.binary() ) { binary_.reset( new binary_input_stream< S >( is, o, sample ) ); }
    else { ascii_.reset( new ascii_input_stream< S >( is, o, sample ) ); }
}

template < typename S >
inline input_stream< S >::input_stream( std::istream& is, const S& sample )
    : ascii_( new ascii_input_stream< S >( is, sample ) )
{
}

template < typename S >
std::string inline input_stream< S >::last() const
{
    if( !binary_ ) { return comma::join( ascii_->last(), ascii_->ascii().delimiter() ); }
    std::string s( binary_->size(), 0 );
    ::memcpy( &s[0], binary_->last(), binary_->size() );
    return s;
}

template < typename S >
inline output_stream< S >::output_stream( std::ostream& os, const csv::options& o, const S& sample )
{
    if( o.binary() ) { binary_.reset( new binary_output_stream< S >( os, o, sample ) ); }
    else { ascii_.reset( new ascii_output_stream< S >( os, o, sample ) ); }
}

template < typename S >
inline output_stream< S >::output_stream( std::ostream& os, bool binary, bool full_xpath, bool flush, const S& sample )
{
    if( binary ) { binary_.reset( new binary_output_stream< S >( os, "", "", full_xpath, flush, sample ) ); }
    else { ascii_.reset( new ascii_output_stream< S >( os, sample ) ); }
}


template < typename S >
inline output_stream< S >::output_stream( std::ostream& os, const S& sample )
    : ascii_( new ascii_output_stream< S >( os, sample ) )
{
}

/*template< typename S > template<typename T >
inline void output_stream< S >::append_output( input_stream< T >& is, const S& s )
{
    if( binary_)
    {
        binary_->os().write( is.binary().last(), is.binary().size() );
        write(s);
    }
    else
    {
        std::string sbuf;
        ascii_->ascii().put( s, sbuf );
        ascii_->os()<< comma::join( is.ascii().last(), ascii_->ascii().delimiter() ) << ascii_->ascii().delimiter() << sbuf << std::endl;
    }
}*/

template < typename V > inline V read_as( std::istream& is, const options& o, const typename V::value_type& default_value )
{
    input_stream< typename V::value_type > istream( is, o, default_value );
    V v;
    while( istream.ready() || is.good() )
    {
        auto p = istream.read();
        if( !p ) { break; }
        v.push_back( *p );
    }
    return v;
}

template < typename V > inline V read_as( const std::string& filename, const options& o, const typename V::value_type& default_value )
{
    std::ifstream ifs;
    ifs.open( &filename[0], o.binary() ? std::ios_base::in | std::ios_base::binary : std::ios_base::in );
    if( ifs.is_open() ) { return read_as< V >( ifs, o, default_value ); }
    COMMA_THROW( comma::exception, "failed to open '" << filename << "'" );
}

template < typename V > inline V read_as( const std::string& filename, const options& o ) { return read_as< V >( filename, o, typename V::value_type() ); }

template < typename V > inline V read_as( std::istream& is, const options& o ) { return read_as< V >( is, o, typename V::value_type() ); }

} } // namespace comma { namespace csv {
