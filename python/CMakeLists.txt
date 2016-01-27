IF( BUILD_PYTHON_PACKAGES )
    EXECUTE_PROCESS( COMMAND rm -f ${CMAKE_CURRENT_SOURCE_DIR}/disabled )
    MESSAGE( STATUS "${CMAKE_CURRENT_SOURCE_DIR}/disabled has been removed: testing python modules and applications is enabled" )
ELSE( BUILD_PYTHON_PACKAGES )
    MESSAGE( STATUS "comma/python is disabled" )
    EXECUTE_PROCESS( COMMAND touch ${CMAKE_CURRENT_SOURCE_DIR}/disabled )
    MESSAGE( STATUS "disabled file has been dropped in ${CMAKE_CURRENT_SOURCE_DIR}: testing python modules and applications is disabled" )
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

        IF( ADD_PYTHON_PACKAGES_TO_RPM )
            SET( PYTHON_TMP "/tmp/comma/python_modules_for_rpm_install" )
            INSTALL( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${PYTHON_TMP}
                FILES_MATCHING PATTERN "*.py"
                PATTERN test EXCLUDE
                PATTERN output EXCLUDE
                PATTERN stats EXCLUDE
                PATTERN build EXCLUDE
                PATTERN examples EXCLUDE )
        ENDIF( ADD_PYTHON_PACKAGES_TO_RPM )

        ADD_SUBDIRECTORY( comma )

    ELSE( PYTHON )
        MESSAGE( WARNING "python not found, python modules and scripts will not be installed" )
    ENDIF( PYTHON )
ENDIF( BUILD_PYTHON_PACKAGES )
