#include <boost/python.hpp>
#include <comma/csv/format.h>

struct blah
{
    blah() : value( 5 ) { std::cerr << "default constructor" << std::endl; }
    blah( const blah& rhs ) : value( 6 ) { std::cerr << "copy constructor" << std::endl; }
    const blah& operator=( const blah& rhs ) { std::cerr << "assignment" << std::endl; return *this; }
    int value;
};

struct bound
{
    bound( const blah& b ) : value_( b.value ) {}
    int value() const { return value_; }
    int value_;
};

BOOST_PYTHON_MODULE( csv )
{
    boost::python::class_< comma::csv::format >( "format", boost::python::init< const std::string& >() )
        .def( "size", &comma::csv::format::size );

    boost::python::class_< blah >( "blah" );

    boost::python::class_< bound >( "bound", boost::python::init< blah >() )
        .def( "value", &bound::value );
}

