SET( PROJECT "containers" )
SET( TARGET_NAME comma_${PROJECT} )

FILE( GLOB source   ${SOURCE_CODE_BASE_DIR}/${PROJECT}/*.cpp)
FILE( GLOB includes ${SOURCE_CODE_BASE_DIR}/${PROJECT}/*.h)
FILE( GLOB multidimensional_includes ${SOURCE_CODE_BASE_DIR}/${PROJECT}/multidimensional/*.h )

SOURCE_GROUP( ${TARGET_NAME} FILES ${source} ${includes} ${multidimensional_includes} )

add_custom_target( ${TARGET_NAME} ${source} ${includes} ${multidimensional_includes} ) # ADD_LIBRARY( ${TARGET_NAME} ${source} ${includes} ${impl_includes} )
#SET_TARGET_PROPERTIES( ${TARGET_NAME} PROPERTIES ${comma_LIBRARY_PROPERTIES} )
#TARGET_LINK_LIBRARIES( ${TARGET_NAME} comma_base )

INSTALL( FILES ${includes} DESTINATION ${comma_INSTALL_INCLUDE_DIR}/${PROJECT} )
INSTALL( FILES ${multidimensional_includes} DESTINATION ${comma_INSTALL_INCLUDE_DIR}/${PROJECT}/multidimensional )
#INSTALL(
#    TARGETS ${TARGET_NAME}
#    RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime     # .exe, .dll
#    LIBRARY DESTINATION ${comma_INSTALL_LIB_DIR} COMPONENT Runtime     # .so, mod.dll
#    ARCHIVE DESTINATION ${comma_INSTALL_LIB_DIR} COMPONENT Development # .a, .lib
#)

if( comma_BUILD_TESTS )
    add_subdirectory( test )
endif( comma_BUILD_TESTS )

if( comma_BUILD_APPLICATIONS )
    add_subdirectory( applications )
endif( comma_BUILD_APPLICATIONS )
