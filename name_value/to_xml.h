#ifndef COMMA_XML_TO_XML_
#define COMMA_XML_TO_XML_

#include <iostream>
#include <vector>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/type_traits.hpp>
#include <comma/visiting/visit.h>
#include <comma/visiting/while.h>

namespace comma {

/// a simple visitor for outputting an xml fragment (which is not possible with boost::property_tree)
class to_xml
{
    public:
        to_xml( std::ostream& os, unsigned int indent = 4, unsigned int indent_depth = 0 ) : os_( os ), indent_( indent_depth * indent, ' ' ), indent_step_( indent, ' ' ), final_( false ), closed_( true ) {}

        /// apply_next on boost optional
        template < typename K, typename T >
        void apply_next( const K& name, const boost::optional< T >& value )
        {
            if( !value ) { return; }
            visiting::do_while<    !boost::is_fundamental< T >::value
                                && !boost::is_same< T, boost::posix_time::ptime >::value
                                && !boost::is_same< T, std::string >::value >::visit( name, *value, *this );
        }

        /// apply
        template < typename K, typename T, typename A >
        void apply( const K& name, const std::vector< T, A >& value )
        {
            for( unsigned int i = 0; i < value.size(); ++i )
            {
                open_( name );
                visiting::do_while<    !boost::is_fundamental< T >::value
                                    && !boost::is_same< T, boost::posix_time::ptime >::value
                                    && !boost::is_same< T, std::string >::value >::visit( i, value[i], *this );
                close_( name );
            }
        }

        /// apply
        template < typename K, typename T >
        void apply( const K& name, const T& value )
        {
            std::string s = boost::lexical_cast< std::string >( name );
            open_( &s[0] );
            visiting::do_while<    !boost::is_fundamental< T >::value
                                && !boost::is_same< T, boost::posix_time::ptime >::value
                                && !boost::is_same< T, std::string >::value >::visit( name, value, *this );
            close_( &s[0] );
        }

        /// apply to non-leaf elements
        template < typename K, typename T >
        void apply_next( const K& name, const T& value ) { comma::visiting::visit( name, value, *this ); }

        /// apply to leaf elements
        template < typename K, typename T >
        void apply_final( const K& name, const T& value )
        {
            os_ << value_( value );
            final_ = true;
        }

    private:
        std::ostream& os_;
        std::string indent_;
        std::string indent_step_;
        bool final_;
        bool closed_;
        void open_( const char* name )
        {
            if( name[0] == '\0' ) { return; }
            if( !indent_step_.empty() && !closed_ ) { os_ << std::endl; }
            os_ << indent_ << "<" << name << ">";
            if( indent_step_.empty() ) { return; }
            indent_ += indent_step_;
            closed_ = false;
        }
        void close_( const char* name )
        {
            if( name[0] == '\0' ) { return; }
            indent_ = indent_.substr( 0, indent_.size() - indent_step_.size() );
            if( !final_ ) { os_ << indent_; }
            final_ = false;
            os_ << "</" << name << ">";
            if( !indent_step_.empty() ) { os_ << std::endl; }
            closed_ = true;
        }
        static std::string value_( const boost::posix_time::ptime& t ) { return boost::posix_time::to_iso_string( t ); }
        template < typename T > static T value_( T v ) { return v; }
};

} // namespace comma {

#endif
