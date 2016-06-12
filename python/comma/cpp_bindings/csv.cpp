#include <boost/python.hpp>
#include <comma/csv/format.h>

BOOST_PYTHON_MODULE(csv)
{
    boost::python::class_< comma::csv::format >( "format", boost::python::init< std::string >() )
        .def( "size", &comma::csv::format::size );
}

