MACRO( add_cpp_module module )
    ADD_LIBRARY( ${module} SHARED ${module}.cpp )
    TARGET_LINK_LIBRARIES( ${module} ${Boost_LIBRARIES} ${PYTHON_LIBRARY} comma_${module} )
    STRING( REPLACE ${comma_PYTHON_SOURCE_DIR}/ "" comma_PYTHON_PACKAGE_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
    SET_TARGET_PROPERTIES( ${module} PROPERTIES PREFIX "" LIBRARY_OUTPUT_DIRECTORY ${BUILD_BASE}/${PYTHON_BUILD_LIB_DIR}/${comma_PYTHON_PACKAGE_DIR} )
ENDMACRO( add_cpp_module )

add_cpp_module( csv )
