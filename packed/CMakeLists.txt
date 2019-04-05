SET( PROJECT "packed" )
SET( TARGET_NAME comma_${PROJECT} )

FILE( GLOB source ${SOURCE_CODE_BASE_DIR}/${PROJECT}/*.cpp )
FILE( GLOB includes ${SOURCE_CODE_BASE_DIR}/${PROJECT}/*.h )
FILE( GLOB detail_includes ${SOURCE_CODE_BASE_DIR}/${PROJECT}/detail/*.h )
SOURCE_GROUP( ${PROJECT} FILES ${source} ${includes} ${detail_includes} )
ADD_CUSTOM_TARGET( ${TARGET_NAME} ${source} ${includes} ${detail_includes} )
# target_link_libraries( ${TARGET_NAME} comma_string )

INSTALL( FILES ${includes} DESTINATION ${comma_INSTALL_INCLUDE_DIR}/${PROJECT} )
INSTALL( FILES ${detail_includes} DESTINATION ${comma_INSTALL_INCLUDE_DIR}/${PROJECT}/detail )

IF( comma_BUILD_TESTS )
    ADD_SUBDIRECTORY( test )
ENDIF( comma_BUILD_TESTS )
