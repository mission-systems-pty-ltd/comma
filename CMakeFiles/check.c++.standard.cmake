    SET( CXX_STANDARDS "0x;11;14;17" CACHE STRING "list of known c++ standards" )
    MARK_AS_ADVANCED( FORCE CXX_STANDARDS )
    STRING( REGEX REPLACE ";" "," CXX_STANDARDS_READABLE "${CXX_STANDARDS}" )
    SET( CXX_STANDARD_DEFAULT "11" CACHE STRING "default c++ standard to use" )
    MARK_AS_ADVANCED( FORCE CXX_STANDARD_DEFAULT )
    SET( CXX_STANDARD_TO_USE "${CXX_STANDARD_DEFAULT}" CACHE STRING "c++ standard to use (one of ${CXX_STANDARDS_READABLE})" )
    SET_PROPERTY( CACHE CXX_STANDARD_TO_USE PROPERTY STRINGS ${CXX_STANDARDS} )
    STRING( REGEX REPLACE ";" "|" CXX_STANDARDS_REGEX "${CXX_STANDARDS}" )
    if ( NOT ( "${CXX_STANDARD_TO_USE}" MATCHES "${CXX_STANDARDS_REGEX}" ) )
	message( FATAL_ERROR "Unknown C++ standard ${CXX_STANDARD_TO_USE}, not one of ${CXX_STANDARDS_READABLE}" )
    endif()

    IF( NOT( DEFINED CXX_STANDARD_LAST ) )
	 SET( CXX_STANDARD_LAST "NotAnOption" CACHE STRING "last c++ standard used" )
	 SET( CXX_STANDARD_FLAGS "NotStandardFlags" CACHE STRING "compiler flags selecting C++ standard" )
	 MARK_AS_ADVANCED( FORCE CXX_STANDARD_LAST )
	 MARK_AS_ADVANCED( FORCE CXX_STANDARD_FLAGS )
    ENDIF()

    include( CheckCXXCompilerFlag )
    IF( ${CXX_STANDARD_TO_USE} MATCHES "0x" )
        message( WARNING "
 #########################################################################
 ##  you have chosen to use C++${CXX_STANDARD_TO_USE}; compilation may fail for C++${CXX_STANDARD_DEFAULT} code  ##
 ##             proceed at your own risk, the build may fail            ##
 ##   consider using 'make --keep-going' to build at least something    ##
 #########################################################################
" )
    ENDIF()

    IF( NOT ( ${CXX_STANDARD_TO_USE} MATCHES ${CXX_STANDARD_LAST} ) )
        # message( "Have to check if ${CMAKE_CXX_COMPILER} supports C++${CXX_STANDARD_TO_USE}" )
        message( "Attempt to use C++ standard ${CXX_STANDARD_TO_USE}" )
        UNSET( compiler_supports_standard CACHE )
        UNSET( compiler_flag_to_check CACHE )
        SET( compiler_flag_to_check "-std=c++${CXX_STANDARD_TO_USE}" )
        if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND ${CXX_STANDARD_TO_USE} MATCHES "11" )
            set( extra_compiler_flags "${compiler_flag_to_check} -Wc++11-narrowing" )
        endif()
        CHECK_CXX_COMPILER_FLAG( "${compiler_flag_to_check} ${extra_compiler_flags}" compiler_supports_standard )
        if( NOT compiler_supports_standard )
            message( FATAL_ERROR "attempt to use C++ standard ${CXX_STANDARD_TO_USE} but ${CMAKE_CXX_COMPILER} does not support it" )
        endif()
        SET( CXX_STANDARD_LAST ${CXX_STANDARD_TO_USE} CACHE STRING "updating C++ standard to use option" FORCE )
        set( CMAKE_CXX_FLAGS "${extra_compiler_flags}" CACHE STRING "" FORCE )
    ENDIF()

    set( CMAKE_CXX_STANDARD ${CXX_STANDARD_TO_USE} )
