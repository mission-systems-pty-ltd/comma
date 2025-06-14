# to use toolchain, run ccmake/cmake first time as:
#  ccmake -DUSE_ARM_TOOLCHAIN=ON ~/src/comma/
# for now, if you want to build applications, you will need to point pthread
# and boost libraries to the libraries built for arm
if( USE_ARM_TOOLCHAIN )
    set( CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/arm.toolchain.cmake )
    # for info: look at opencv-2.4.11/platforms/linux/arm-gnueabi.toolchain.cmake
    include( ${CMAKE_TOOLCHAIN_FILE} REQUIRED )
endif( USE_ARM_TOOLCHAIN )

cmake_minimum_required( VERSION 3.1 )
cmake_policy( SET CMP0048 NEW )
project( comma VERSION 1.1.0 LANGUAGES CXX )
if( NOT DEFINED CMAKE_PROJECT_VERSION )
    set( CMAKE_PROJECT_VERSION ${PROJECT_VERSION} )
    set( CMAKE_PROJECT_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} )
    set( CMAKE_PROJECT_VERSION_MINOR ${PROJECT_VERSION_MINOR} )
    set( CMAKE_PROJECT_VERSION_PATCH ${PROJECT_VERSION_PATCH} )
endif( NOT DEFINED CMAKE_PROJECT_VERSION )

set( comma_version_major "${CMAKE_PROJECT_VERSION_MAJOR}" ) # quick and dirty
set( comma_version_minor "${CMAKE_PROJECT_VERSION_MINOR}" ) # quick and dirty
set( comma_version_patch "${CMAKE_PROJECT_VERSION_PATCH}" ) # quick and dirty
set( comma_version "${CMAKE_PROJECT_VERSION}" ) # quick and dirty

# option( USE_ARM_TOOLCHAIN "Cross compile using arm toolchain" OFF )
# if( USE_ARM_TOOLCHAIN )
#     set(CMAKE_TOOLCHAIN_FILE ${SOURCE_CODE_BASE_DIR}/CMakeFiles/arm.toolchain.cmake)
# endif( USE_ARM_TOOLCHAIN )
#
# option(my_test_option "whatever" OFF)
# if(my_test_option)
#     set(my_other_option "is set")
# endif(my_test_option)
# set(my_first_option "is also set")

ENABLE_TESTING()

IF (UNIX AND NOT APPLE)
  # May also fail on BSD as it assumes GNU date rather than BSD date command
	# Check if Git is installed
	find_package(Git)
	IF(GIT_FOUND)
		# Get the date of the last commit (for use by CPack)
		EXECUTE_PROCESS(
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMAND git log -n1 --format=%ci
		COMMAND xargs -0 date +%Y%m%dT%H%M -d
		OUTPUT_VARIABLE COMMITDATE
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
		# And also the hash.
		EXECUTE_PROCESS(
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMAND git rev-parse HEAD
		OUTPUT_VARIABLE COMMITHASH
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	ELSE()
		SET ( COMMITDATE 1 )
		SET ( COMMITHASH "abc" )
	ENDIF()
ELSE() # Probably windows; gets hardcoded patch version
	SET ( COMMITDATE 1 )
	SET ( COMMITHASH "abc" )
ENDIF()

if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release CACHE STRING "options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE )
endif( NOT CMAKE_BUILD_TYPE )

if( WIN32 )
  SET(CMAKE_CXX_WARNING_LEVEL 4)
  IF(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    STRING(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  ELSE(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  ENDIF(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
  ADD_DEFINITIONS(/wd4512)
  ADD_DEFINITIONS(/wd4127)
  ADD_DEFINITIONS(/wd4996)
  ADD_DEFINITIONS(/wd4800)
  ADD_DEFINITIONS(/wd4244)
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D \"NOMINMAX\"")
  ADD_DEFINITIONS(-D_USE_MATH_DEFINES)
  ADD_DEFINITIONS(-DWIN32_LEAN_AND_MEAN) #used to fix winsock redefinition error
else( WIN32 )
    option( ENABLE_FAST_MATH "enable compiler fast-math; experimental, some tests fail on nan and precision" OFF )
    if( ENABLE_FAST_MATH )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffast-math" )
    endif( ENABLE_FAST_MATH )
    option( ENABLE_OPTIMIZATION "enable compiler optimisation" ON )
    if( ENABLE_OPTIMIZATION )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3" ) # quick and dirty; make optimisation level configurable
    endif( ENABLE_OPTIMIZATION )
endif( WIN32 )

IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  SET( IS_64_BIT TRUE )
ELSE( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  SET( IS_64_BIT FALSE )
ENDIF( CMAKE_SIZEOF_VOID_P EQUAL 8 )

IF(CMAKE_BUILD_TOOL MATCHES "make")
  IF(NOT CMAKE_CXX_FLAGS MATCHES "-Wall")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  ENDIF(NOT CMAKE_CXX_FLAGS MATCHES "-Wall")
  IF(NOT CMAKE_C_FLAGS MATCHES "-Wall")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
  ENDIF(NOT CMAKE_C_FLAGS MATCHES "-Wall")

  IF( IS_64_BIT )
    IF(NOT CMAKE_CXX_FLAGS MATCHES "-fPIC")
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    ENDIF(NOT CMAKE_CXX_FLAGS MATCHES "-fPIC")
    IF(NOT CMAKE_C_FLAGS MATCHES "-fPIC")
      SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    ENDIF(NOT CMAKE_C_FLAGS MATCHES "-fPIC")
  ENDIF( IS_64_BIT )

ENDIF(CMAKE_BUILD_TOOL MATCHES "make")

if( UNIX )
    include( CMakeFiles/check.c++.standard.cmake REQUIRED )
endif( UNIX )

SET( SOURCE_CODE_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
SET( LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib )

set( comma_INSTALL_BIN_DIR bin CACHE PATH "installation directory for binaries" )
set( comma_INSTALL_INCLUDE_DIR include/comma CACHE PATH "installation directory for include directories" )
set( comma_INSTALL_LIB_DIR lib CACHE PATH "installation directory for libraries" )
set( comma_INSTALL_PACKAGE_DIR CMakeFiles CACHE PATH "installation directory for cmake files" )

SET( comma_LIBRARY_PROPERTIES ${comma_LIBRARY_PROPERTIES} DEBUG_POSTFIX "-d" SOVERSION ${CMAKE_PROJECT_VERSION} LINK_FLAGS_RELEASE -s )

#INCLUDE_DIRECTORIES( ${comma_INSTALL_INCLUDE_DIR}/../ )

IF( APPLE )
    SET( CMAKE_INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/${comma_INSTALL_LIB_DIR} )
ENDIF( APPLE )

OPTION( BUILD_TESTS "build unit tests" OFF )
SET( comma_BUILD_TESTS ${BUILD_TESTS} )

option( BUILD_PYTHON_PACKAGES "install comma python packages" ON )

INCLUDE( CMakeDependentOption )
CMAKE_DEPENDENT_OPTION( ADD_PYTHON_PACKAGES_TO_RPM "add comma python packages to rpm" OFF "BUILD_PYTHON_PACKAGES" ON )
CMAKE_DEPENDENT_OPTION( BUILD_CPP_PYTHON_BINDINGS "build C++/Python bindings" OFF "BUILD_PYTHON_PACKAGES" ON )

OPTION( BUILD_APPLICATIONS "build applications" ON )
SET( comma_BUILD_APPLICATIONS ${BUILD_APPLICATIONS} )

option( BUILD_SHARED_LIBS "build with shared libraries" ON )
option( comma_BUILD_NAME_VALUE_YAML "build name_value with yaml support; requires libyaml; e.g. apt install libyaml-dev" OFF )
option( comma_BUILD_PYTHON "build python" ON )
option( comma_BUILD_XML "build xml" OFF )
option( comma_BUILD_ZEROMQ "build io with zeromq support" OFF )
option( comma_USE_BOOST_FILESYSTEM "use boost::filesystem rather than std::filesystem" OFF )

if( comma_USE_BOOST_FILESYSTEM )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCOMMA_USE_BOOST_FILESYSTEM -DBOOST_FILESYSTEM_NO_DEPRECATED" )
else()
    # see https://gcc.gnu.org/onlinedocs/libstdc++/manual/status.html#status.iso.2017
    if( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9 )
        link_libraries( "-lstdc++fs" )
    endif()
endif()

SET(comma_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})

IF( NOT BUILD_SHARED_LIBS )
    SET( Boost_USE_STATIC_LIBS ON )
ENDIF( NOT BUILD_SHARED_LIBS )

set( comma_boost_components thread system date_time iostreams program_options regex )
if( comma_USE_BOOST_FILESYSTEM )
    set( comma_boost_components ${comma_boost_components} filesystem )
endif()

IF( BUILD_PYTHON_PACKAGES AND BUILD_CPP_PYTHON_BINDINGS )
    IF( NOT BUILD_SHARED_LIBS )
        MESSAGE( WARNING "Linking C++/Python bindings with static libraries is requested. If linking fails, recompile required libraries with -fPIC or use shared libraries. Note that this feature has only been tested with shared libraries." )
    ENDIF( NOT BUILD_SHARED_LIBS )
    SET( Python_ADDITIONAL_VERSIONS 2.7 )
    FIND_PACKAGE( PythonLibs REQUIRED )
    INCLUDE_DIRECTORIES( ${PYTHON_INCLUDE_DIRS} )
    set( comma_boost_components ${comma_boost_components} python )
ENDIF( BUILD_PYTHON_PACKAGES AND BUILD_CPP_PYTHON_BINDINGS )

FIND_PACKAGE( Boost COMPONENTS ${comma_boost_components} )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIRS} )
LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )

IF( comma_BUILD_TESTS )

find_package( GTest REQUIRED )
include_directories( ${GTEST_INCLUDE_DIRS} )

    # todo: maybe else clause is actually the right way to do things
#    IF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
#        FIND_PATH( GTest_SOURCE_DIR NAMES CMakeLists.txt HINTS /usr/src/googletest/googletest DOC "source code of gtest" ) # FIND_PATH( GTest_SOURCE_DIR NAMES CMakeLists.txt HINTS /usr/src/gtest DOC "source code of gtest" )
#        add_subdirectory( ${GTest_SOURCE_DIR} gtest )
#        INCLUDE_DIRECTORIES( src ${GTest_SOURCE_DIR}/include ${GTest_SOURCE_DIR} )
#        SET( GTEST_BOTH_LIBRARIES gtest gtest_main )
#    ELSE( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
#        FIND_PACKAGE( GTest )
#        INCLUDE_DIRECTORIES( ${GTEST_INCLUDE_DIRS} )
#    ENDIF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )

#TT: Install tests to dir. This allows them to be packed by CPack.
#Note: if your source tree is dirty (eg you've already run tests there and have output/ directories)
#those will also be installed too.

OPTION( INSTALL_TESTS "Install the tests into the nominated directory" OFF )
IF( INSTALL_TESTS )
SET ( comma_CPP_TESTS_INSTALL_DIR bin CACHE PATH "Install directory for cpp unit tests" )
SET ( comma_INSTALL_COMMA_TESTS_DIR share/test )

# Get the list of test directories (using glob).
FILE ( GLOB_RECURSE all_files
		# RELATIVE is tricky. When you use it, you need to specify the path you expect the output to be RELATIVE to. (Yes, that path could be considerably non-relative to the file that is found.)
		RELATIVE ${CMAKE_SOURCE_DIR}
		* )

SET ( test_dirs )
FOREACH ( i ${all_files} )
	if ( ${i} MATCHES "[\\/]test[\\/]" )
		LIST (APPEND test_dirs ${i} )
	endif ()
ENDFOREACH (i)

# Add those directories
FOREACH (file ${test_dirs} )
		# Determine the parent directories for a given file so to prefix the install location (otherwise they just all get installed in the destination root)
		get_filename_component( dest_dir ${file} DIRECTORY )
                    STRING( REGEX MATCH ".*\\/output\\/.*" skip ${file} )
                    IF( NOT skip )
		# See that slash there? 		 vvv It's really important.
		INSTALL ( 	DIRECTORY ${dest_dir}/
					DESTINATION ${comma_INSTALL_COMMA_TESTS_DIR}/${CMAKE_PROJECT_NAME}/unit/${dest_dir}
					USE_SOURCE_PERMISSIONS
                        PATTERN "output" EXCLUDE )
                    ENDIF( NOT skip )
ENDFOREACH (file)
ENDIF( INSTALL_TESTS )
ENDIF( comma_BUILD_TESTS )

IF( comma_BUILD_ZEROMQ )
    FIND_LIBRARY( ZeroMQ_LIBRARY NAMES zmq )
    ADD_DEFINITIONS( -DUSE_ZEROMQ )
    INCLUDE_DIRECTORIES( ${ZeroMQ_INCLUDE_PATH} )
ENDIF( comma_BUILD_ZEROMQ )

SET( comma_ALL_LIBRARIES
        debug comma_application-d
        optimized comma_application
        debug comma_base-d
        optimized comma_base
        debug comma_csv-d
        optimized comma_csv
        debug comma_io-d
        optimized comma_io
        debug comma_string-d
        optimized comma_string
        debug comma_xpath-d
        optimized comma_xpath
        debug comma_name_value-d
        optimized comma_name_value
        debug comma_timing-d
        optimized comma_timing )

if(WIN32)
    set( comma_ALL_EXTERNAL_LIBRARIES Ws2_32.lib )
endif(WIN32)

if( UNIX AND NOT QNXNTO AND NOT APPLE )
    set( comma_ALL_EXTERNAL_LIBRARIES rt pthread c )
endif( UNIX AND NOT QNXNTO AND NOT APPLE )

set( comma_ALL_EXTERNAL_LIBRARIES ${Boost_LIBRARIES} ${comma_ALL_EXTERNAL_LIBRARIES} )

if( comma_BUILD_ZEROMQ )
    set( comma_ALL_EXTERNAL_LIBRARIES ${comma_ALL_EXTERNAL_LIBRARIES} ${ZeroMQ_LIBRARY} )
endif( comma_BUILD_ZEROMQ )

if( BUILD_CPP_PYTHON_BINDINGS )
    set( comma_ALL_EXTERNAL_LIBRARIES ${comma_ALL_EXTERNAL_LIBRARIES} ${PYTHON_LIBRARY} )
endif( BUILD_CPP_PYTHON_BINDINGS )

set( comma_ALL_LIBRARIES ${comma_ALL_LIBRARIES} ${comma_ALL_LIBRARIES} ${comma_ALL_EXTERNAL_LIBRARIES} ) # as gcc parses library list only once
set( comma_EXTERNAL_INCLUDES ${Boost_INCLUDE_DIRS} )

CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET( uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake )

FILE( GLOB comma_PACKAGE_CMAKEFILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/*.cmake )

INCLUDE( ${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/generate_${PROJECT_NAME}_config.cmake )

CONFIGURE_FILE( ${PROJECT_SOURCE_DIR}/CMakeFiles/use_${PROJECT_NAME}.cmake.in
                ${PROJECT_BINARY_DIR}/CMakeFiles/use_${PROJECT_NAME}.cmake COPYONLY IMMEDIATE )

set( comma_PACKAGE_CMAKEFILES ${comma_PACKAGE_CMAKEFILES} ${PROJECT_BINARY_DIR}/CMakeFiles/use_${PROJECT_NAME}.cmake )
install( FILES ${comma_PACKAGE_CMAKEFILES} DESTINATION ${comma_INSTALL_PACKAGE_DIR} )
#install( FILES ${PROJECT_SOURCE_DIR}/copyright DESTINATION share/doc/comma ) # quick and dirty

#-----------------------------------------------------------------------------
# add sources

add_subdirectory( application )
add_subdirectory( base )
add_subdirectory( bash )
add_subdirectory( containers )
add_subdirectory( csv )
add_subdirectory( dispatch )
add_subdirectory( doc )
add_subdirectory( dynamic ) 
add_subdirectory( io )
add_subdirectory( math )
add_subdirectory( name_value )
add_subdirectory( packed )
add_subdirectory( string )
add_subdirectory( sync )
add_subdirectory( timing )
add_subdirectory( util )
add_subdirectory( visiting )
add_subdirectory( web )
if( comma_BUILD_XML )
    add_subdirectory( xml )
endif( comma_BUILD_XML )
add_subdirectory( xpath )

option( INSTALL_BASH_COMPLETION "install the BASH completion scripts" ON )
if( INSTALL_BASH_COMPLETION )
    add_subdirectory( etc/bash_completion.d )
endif( INSTALL_BASH_COMPLETION )

if( comma_BUILD_PYTHON )
  add_subdirectory( python )
endif( comma_BUILD_PYTHON )

#in future, may need to add third party dlls somehow, if they are linked as dlls. Not needed for statically linking to boost.
#if we set them up to install to bin they might be packaged through, e.g. roughly (using GLOB):
#install(FILES "${Boost_LIBRARY_DIRS}/*.dll" DESTINATION bin )

if( CMAKE_COMPILER_IS_GNUCXX )
    if( CMAKE_BUILD_TYPE STREQUAL "Release" )
        set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s" )
        set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s" )
    endif( CMAKE_BUILD_TYPE STREQUAL "Release" )
endif( CMAKE_COMPILER_IS_GNUCXX )

option( comma_INSTALL_RUN_POST_INSTALL "run post install step (currently just ldconfig)" ON )
if( comma_INSTALL_RUN_POST_INSTALL )
    add_subdirectory( CMakeFiles/post_install ) # uber-quick and dirty
endif( comma_INSTALL_RUN_POST_INSTALL )

# todo
#     build ppa
#         see here? http://schneegans.github.io/lessons/2011/11/02/ppa-launchpad-cmake
#     lintian errors and warnings
#         fix warnings
#             W: comma: package-name-doesnt-match-sonames: try comma_string -> comma-string, etc?
#             W: comma: maintscript-calls-ldconfig [postinst]: don't call ldconfig from package?
#             W: comma: maintscript-calls-ldconfig [postrm]: don't call ldconfig from package?
#             W: comma: distant-prerequisite-in-shlibs comma (ON 1.0.0): try to add full version to shared libs
#         investigate
#             W: comma: non-standard-dir-in-usr usr/CMakeFiles/
#         ignore for now
#             W: comma: link-to-shared-library-in-wrong-package: deemed ok for small packages
#             W: comma: no-manual-page: ok for now, lots of packages don't have man pages

set( CPACK_PACKAGE_NAME "comma" )
set( CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}" CACHE STRING "package name" )
#set( CPACK_PACKAGE_DESCRIPTION "cli utilities for structured or fixed-width offline or realtime data" )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "cli utilities for structured or fixed-width offline or realtime data" )
set( CPACK_PACKAGE_VENDOR "orthographic" )
set( CPACK_DEBIAN_PACKAGE_DESCRIPTION "CLI for processing structured or fixed-width data offline or in realtime" )
set( CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://gitlab.com/orthographic/comma/-/wikis/home" )
set( CPACK_DEBIAN_PACKAGE_MAINTAINER "vsevolod vlaskine <vsevolod.vlaskine@gmail.com>" )
set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${PROJECT_SOURCE_DIR}/system/package/debian/changelog" )
set( CPACK_PACKAGE_CONTACT "vsevolod.vlaskine@gmail.com" )
set( CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/COPYING" )
set( CPACK_GENERATOR "DEB" CACHE STRING "cpack generator: PRM or DEB" )
set( CPACK_PACKAGE_VERSION_MAJOR ${comma_version_major} )
set( CPACK_PACKAGE_VERSION_MINOR ${comma_version_minor} )
set( CPACK_PACKAGE_VERSION_PATCH ${comma_version_patch} )
set( CPACK_DEPENDENCIES_LIBPROCPS "libprocps8 (>= 1)" CACHE STRING "libprocps versions: libprocps6 (>= 2:3.3.0) for Ubuntu 18.04; libprocps8 (>= 1) for Ubuntu 20.04 or later" )
option( CPACK_DEPENDENCIES_PYTHON "add python dependencies, currently: python3-numpy" OFF )
#set( CPACK_DEBIAN_PACKAGE_DEPENDS "comma-py, ${CPACK_DEPENDENCIES_LIBPROCPS}, libzmq5 (>= 4.2.5-1), libboost-all-dev (>=1.65), recode (>=3.6), socat, libc (>=6)" )
#set( CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEPENDENCIES_LIBPROCPS}, libzmq5 (>= 4.2.5-1), libboost-all-dev (>=1.65), recode (>=3.6), socat, libc-dev (>=6)" )
#set( CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEPENDENCIES_LIBPROCPS}, libzmq5 (>= 4.2.5-1), libboost-all-dev (>=1.65), recode (>=3.6), socat, libc-dev" )
#set( CPACK_DEBIAN_PACKAGE_SHLIBDEPS "libboost-all-dev (>=1.65), libc" )
set( CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEPENDENCIES_LIBPROCPS}, libzmq5 (>= 4.2.5-1), libboost-all-dev (>=1.65), recode (>=3.6), socat, libc-dev" )
if( CPACK_DEPENDENCIES_PYTHON )
    set( CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, python3-numpy" )
endif( CPACK_DEPENDENCIES_PYTHON )
set( CPACK_DEBIAN_PACKAGE_SHLIBDEPS "libboost-all-dev (>=1.65), libc-dev" )
set( CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON )
set( CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS_POLICY ON )

# if the user builds shared, make sure that the RPM package (that might be generated) runs ldconfig when it's installed.
if( BUILD_SHARED_LIBS )
    #set( CPACK_RPM_SPEC_MORE_DEFINE "%posttrans -p /sbin/ldconfig" )
    set( CPACK_RPM_POST_INSTALL_SCRIPT_FILE "%posttrans -p /sbin/ldconfig" ) # this line works but commented out to avoid lintian error
    #set( SHLIBS_FILE "${CMAKE_CURRENT_BINARY_DIR}/shlibs" )
    # witchcraft from here: https://github.com/roboception/rcdiscover/blob/master/cmake/package_debian.cmake
    set( TRIGGERS_FILE "${CMAKE_CURRENT_BINARY_DIR}/triggers" )
    file( WRITE "${TRIGGERS_FILE}" "activate-noawait ldconfig\n" )
    #file( WRITE "${SHLIBS_FILE}" "" )
    #foreach ( libname ${sharedlibs} )
    #    get_target_property( so_abiversion ${libname} SOVERSION )
    #    if(NOT ${so_abiversion})
    #      set(so_abiversion ${abiversion})
    #      message(STATUS "SOVERSION of shared lib \"${libname}\" not set explicitly. Using <Major.Minor> of latest tag: ${so_abiversion}")
    #      set_target_properties(${libname} PROPERTIES SOVERSION ${so_abiversion})
    #    endif()
    #    file(APPEND "${SHLIBS_FILE}" "lib${libname} ${so_abiversion} ${CPACK_PACKAGE_NAME}\n")
    #endforeach (libname)
    #execute_process( COMMAND chmod 644 "${SHLIBS_FILE}" "${TRIGGERS_FILE}" )
    execute_process( COMMAND chmod 644 "${TRIGGERS_FILE}" )
    if( CPACK_DEB_COMPONENT_INSTALL )
        set( CPACK_DEBIAN_BIN_PACKAGE_CONTROL_EXTRA "${CPACK_DEBIAN_BIN_PACKAGE_CONTROL_EXTRA};${SHLIBS_FILE};${TRIGGERS_FILE}" )
    else ( CPACK_DEB_COMPONENT_INSTALL )
        set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA};${SHLIBS_FILE};${TRIGGERS_FILE}" )
    endif ( CPACK_DEB_COMPONENT_INSTALL )
endif( BUILD_SHARED_LIBS )

IF( BUILD_PYTHON_PACKAGES AND ADD_PYTHON_PACKAGES_TO_RPM )
    SET( CPACK_RPM_PACKAGE_REQUIRES "boost-thread recode socat psmisc python3 numpy" )
    SET( CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_SOURCE_DIR}/python/comma-rpm-post-install.sh" )
    SET( CPACK_RPM_PRE_INSTALL_SCRIPT_FILE "${CMAKE_SOURCE_DIR}/python/comma-rpm-pre-install.sh" )
    SET( CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/tmp" ) # Prevents /tmp from permission change when installing RPM package
ELSE( BUILD_PYTHON_PACKAGES AND ADD_PYTHON_PACKAGES_TO_RPM )
    SET(CPACK_RPM_PACKAGE_REQUIRES "boost-thread socat psmisc recode")
ENDIF( BUILD_PYTHON_PACKAGES AND ADD_PYTHON_PACKAGES_TO_RPM )

#this line finds the appropriate Visual Studio DLLS in windows. Maybe it would find other dlls if a different build tool were used. Probably should be windows only.
INCLUDE( InstallRequiredSystemLibraries )

#this line does all the work. Actually, even if this is the only line, you get all but the dlls...
INCLUDE( CPack )
