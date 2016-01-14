IF( BUILD_PYTHON_PACKAGES )
    EXECUTE_PROCESS( COMMAND rm -f ${SOURCE_CODE_BASE_DIR}/python/disabled )
ELSE( BUILD_PYTHON_PACKAGES )
    EXECUTE_PROCESS( COMMAND touch ${SOURCE_CODE_BASE_DIR}/python/disabled )
ENDIF( BUILD_PYTHON_PACKAGES )

IF( BUILD_PYTHON_PACKAGES )
    FIND_PROGRAM( PYTHON "python" )

    IF( PYTHON )

        SET( PYTHON_PACKAGE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE STRING "install prefix for python packages, leave empty for python default prefix" )

        SET( INSTALL_PREFIX_OPTION "" )
        IF( PYTHON_PACKAGE_INSTALL_PREFIX )
            SET( INSTALL_PREFIX_OPTION "--prefix=${PYTHON_PACKAGE_INSTALL_PREFIX}" )
            IF( NOT WIN32 )
                SET( INSTALL_PREFIX_OPTION "${INSTALL_PREFIX_OPTION} --exec-prefix=${PYTHON_PACKAGE_INSTALL_PREFIX}" )
            ENDIF( NOT WIN32 )
        ENDIF( PYTHON_PACKAGE_INSTALL_PREFIX )

        SET( SETUP_PY "${CMAKE_CURRENT_SOURCE_DIR}/setup.py" )

        SET( BUILD_BASE "${CMAKE_CURRENT_BINARY_DIR}/build" )

        ADD_CUSTOM_TARGET( python_build ALL
                          ${PYTHON} ${SETUP_PY} build --build-base ${BUILD_BASE}
                          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

        INSTALL( CODE "EXECUTE_PROCESS(
                            COMMAND ${PYTHON} ${SETUP_PY} build --build-base ${BUILD_BASE} install ${INSTALL_PREFIX_OPTION}
                            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )" )

        SET_DIRECTORY_PROPERTIES( PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES build )

        # for RPM PACKAGING
        SET( PYTHON_TMP "/tmp/python_comma_modules" )
        INSTALL( FILES ${SETUP_PY} DESTINATION ${PYTHON_TMP} )
        SET( ROOT "${PYTHON_TMP}/comma" )

        ADD_SUBDIRECTORY( comma )

    ELSE( PYTHON )
        MESSAGE( WARNING "python not found, python modules and scripts will not be installed" )
    ENDIF( PYTHON )
ENDIF( BUILD_PYTHON_PACKAGES )
