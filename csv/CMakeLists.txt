SET( PROJECT "csv" )
SET( TARGET_NAME comma_${PROJECT} )
SET( dir ${SOURCE_CODE_BASE_DIR}/${PROJECT} )

FILE( GLOB source ${dir}/*.cpp )
FILE( GLOB includes ${dir}/*.h )
FILE( GLOB impl_source ${dir}/impl/*.cpp )
FILE( GLOB impl_includes ${dir}/impl/*.h )

SOURCE_GROUP( ${TARGET_NAME} FILES ${source} ${includes} ${impl_includes} )

ADD_LIBRARY( ${TARGET_NAME} ${source} ${includes} ${impl_source} ${impl_includes} )
SET_TARGET_PROPERTIES( ${TARGET_NAME} PROPERTIES ${comma_LIBRARY_PROPERTIES} )
TARGET_LINK_LIBRARIES( ${TARGET_NAME} comma_application comma_xpath ${comma_ALL_EXTERNAL_LIBRARIES} )
set_target_properties( ${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( ${TARGET_NAME} PROPERTIES SOVERSION ${comma_version_major} ${comma_version_minor} ${comma_version_patch} )

INSTALL( FILES ${includes} DESTINATION ${comma_INSTALL_INCLUDE_DIR}/${PROJECT}/ )
INSTALL( FILES ${impl_includes} DESTINATION ${comma_INSTALL_INCLUDE_DIR}/${PROJECT}/impl )
INSTALL(
    TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime     # .exe, .dll
    LIBRARY DESTINATION ${comma_INSTALL_LIB_DIR} COMPONENT Runtime     # .so, mod.dll
    ARCHIVE DESTINATION ${comma_INSTALL_LIB_DIR} COMPONENT Development # .a, .lib
)

if( comma_BUILD_APPLICATIONS )
    add_subdirectory( applications )
endif( comma_BUILD_APPLICATIONS )

IF( comma_BUILD_TESTS )
    ADD_SUBDIRECTORY( test )
ENDIF( comma_BUILD_TESTS )
