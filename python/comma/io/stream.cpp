#include <boost/python.hpp>
#include <comma/io/stream.h>

BOOST_PYTHON_MODULE(stream)
{
    boost::python::def( "hello_python", comma::io::hello_python );
}

