MACRO( enable_python_testing )
    EXECUTE_PROCESS( COMMAND rm -f ${CMAKE_CURRENT_SOURCE_DIR}/disabled )
    MESSAGE( STATUS "${CMAKE_CURRENT_SOURCE_DIR}/disabled has been removed: testing python modules and applications is enabled" )
ENDMACRO( enable_python_testing )

MACRO( disable_python_testing )
    EXECUTE_PROCESS( COMMAND touch ${CMAKE_CURRENT_SOURCE_DIR}/disabled )
    MESSAGE( STATUS "disabled file has been dropped in ${CMAKE_CURRENT_SOURCE_DIR}: testing python modules and applications is disabled" )
ENDMACRO( disable_python_testing )

MACRO( check_python_module module )
    EXECUTE_PROCESS( COMMAND ${PYTHON} -c "import ${module}; print( ${module}.__version__ );"
        RESULT_VARIABLE MODULE_EXIT_CODE
        OUTPUT_VARIABLE MODULE_VERSION_STRING
        ERROR_VARIABLE MODULE_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE )
    IF( MODULE_EXIT_CODE MATCHES 0 )
        MESSAGE( STATUS "found python ${module} module version ${MODULE_VERSION_STRING}" )
    ELSE( MODULE_EXIT_CODE MATCHES 0 )
        MESSAGE( WARNING "python ${module} module not found; install it or disable BUILD_PYTHON_PACKAGES" )
        MESSAGE( STATUS "comma/python modules and scripts will not be installed" )
        disable_python_testing()
        RETURN()
    ENDIF( MODULE_EXIT_CODE MATCHES 0 )
ENDMACRO( check_python_module )

MACRO( set_python_build_lib_directory )
    EXECUTE_PROCESS( COMMAND ${PYTHON} -c "from distutils.dist import Distribution as D; from distutils.command.build import build as B; b = B(D()); b.finalize_options(); print( b.build_lib.split('/',1)[-1] );"
        RESULT_VARIABLE MODULE_EXIT_CODE
        OUTPUT_VARIABLE PYTHON_BUILD_LIB_DIR
        ERROR_VARIABLE MODULE_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE )
    IF( MODULE_EXIT_CODE MATCHES 0 )
        MESSAGE( STATUS "python build lib directory: ${PYTHON_BUILD_LIB_DIR}" )
    ELSE( MODULE_EXIT_CODE MATCHES 0 )
        MESSAGE( WARNING "failed to get python build lib directory" )
        MESSAGE( STATUS "comma/python modules and scripts will not be installed" )
        disable_python_testing()
        RETURN()
    ENDIF( MODULE_EXIT_CODE MATCHES 0 )
ENDMACRO( set_python_build_lib_directory )

IF( NOT BUILD_PYTHON_PACKAGES )
    disable_python_testing()
    RETURN()
ENDIF( NOT BUILD_PYTHON_PACKAGES )

FIND_PROGRAM( PYTHON "python3" )

IF( NOT PYTHON )
    MESSAGE( WARNING "python3 not found" )
    MESSAGE( STATUS "comma/python modules and scripts will not be installed" )
    disable_python_testing()
    RETURN()
ENDIF( NOT PYTHON )

check_python_module( numpy )
enable_python_testing()

SET( PYTHON_PACKAGE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE STRING "install prefix for python packages, leave empty for python default prefix" )

SET( INSTALL_PREFIX_OPTION "" )
IF( PYTHON_PACKAGE_INSTALL_PREFIX )
    SET( INSTALL_PREFIX_OPTION "--prefix=${PYTHON_PACKAGE_INSTALL_PREFIX}" )
    IF( NOT WIN32 )
        SET( INSTALL_PREFIX_OPTION "${INSTALL_PREFIX_OPTION} --exec-prefix=${PYTHON_PACKAGE_INSTALL_PREFIX}" )
    ENDIF( NOT WIN32 )
ENDIF( PYTHON_PACKAGE_INSTALL_PREFIX )

SET( SETUP_PY "${CMAKE_CURRENT_SOURCE_DIR}/setup.py" ) # todo: uses pip instead

SET( BUILD_BASE "${CMAKE_CURRENT_BINARY_DIR}/build" )

SET( comma_PYTHON_SOURCE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} )

set_python_build_lib_directory()

add_subdirectory( comma )
add_subdirectory( bindings )

ADD_CUSTOM_TARGET( python_build ALL
                   ${PYTHON} ${SETUP_PY} build --build-base ${BUILD_BASE}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

INSTALL( CODE "EXECUTE_PROCESS(
                    COMMAND ${PYTHON} ${SETUP_PY} build --build-base ${BUILD_BASE} install ${INSTALL_PREFIX_OPTION} --force
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )" )

# setup.py creates a couple of working directories in the current directory: comma_py.egg_info and dist
# Because it's run in the install stage, and therefore typically as root, these
# directories are owned by root, which is a bit annoying as they are in the
# source tree. So set them to being owned by the user.
INSTALL( CODE "EXECUTE_PROCESS(
                    COMMAND chown -R $ENV{USER} comma_py.egg-info dist
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )" )

#install( CODE "EXECUTE_PROCESS(
#                    COMMAND ${PYTHON} -m pip install ${CMAKE_CURRENT_SOURCE_DIR} --ignore-installed
#                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )" )

SET_DIRECTORY_PROPERTIES( PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES build )

IF( ADD_PYTHON_PACKAGES_TO_RPM )
    SET( PYTHON_TMP "/tmp/comma/python_modules_for_rpm_install" )
    INSTALL( DIRECTORY ${BUILD_BASE}/${PYTHON_BUILD_LIB_DIR}/ DESTINATION ${PYTHON_TMP} FILES_MATCHING PATTERN "*" )
    INSTALL( FILES ${SETUP_PY} DESTINATION ${PYTHON_TMP} )
ENDIF( ADD_PYTHON_PACKAGES_TO_RPM )
