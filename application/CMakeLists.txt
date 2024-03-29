SET( PROJECT "application" )
SET( TARGET_NAME comma_${PROJECT} )

FILE( GLOB source   ${SOURCE_CODE_BASE_DIR}/${PROJECT}/*.cpp)
FILE( GLOB includes ${SOURCE_CODE_BASE_DIR}/${PROJECT}/*.h)

SOURCE_GROUP( ${TARGET_NAME} FILES ${source} ${includes} )

ADD_LIBRARY( ${TARGET_NAME} ${source} ${includes} )
SET_TARGET_PROPERTIES( ${TARGET_NAME} PROPERTIES ${comma_LIBRARY_PROPERTIES} )
#set_target_properties( ${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE -s )
#set_target_properties( ${TARGET_NAME} PROPERTIES SOVERSION "${comma_version}" ) #set_target_properties( ${TARGET_NAME} PROPERTIES SOVERSION ${comma_version_major} ${comma_version_minor} ${comma_version_patch} )
TARGET_LINK_LIBRARIES( ${TARGET_NAME} ${comma_ALL_EXTERNAL_LIBRARIES} comma_base comma_io comma_string )

INSTALL( FILES ${includes} DESTINATION ${comma_INSTALL_INCLUDE_DIR}/${PROJECT} )
INSTALL( TARGETS ${TARGET_NAME}
         RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime       # .exe, .dll
         LIBRARY DESTINATION ${comma_INSTALL_LIB_DIR} COMPONENT Runtime       # .so, mod.dll
         ARCHIVE DESTINATION ${comma_INSTALL_LIB_DIR} COMPONENT Development ) # .a, .lib

if( comma_BUILD_APPLICATIONS )
    add_subdirectory( applications )
endif( comma_BUILD_APPLICATIONS )

IF( comma_BUILD_TESTS )
    ADD_SUBDIRECTORY( test )
ENDIF( comma_BUILD_TESTS )
