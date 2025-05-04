SET( PROJECT "python_bindings" )
SET( TARGET_NAME comma_${PROJECT} )
SET( dir ${SOURCE_CODE_BASE_DIR}/python/bindings )

FILE( GLOB_RECURSE source ${dir}/*.cpp )
FILE( GLOB_RECURSE includes ${dir}/*.h )

SOURCE_GROUP( ${TARGET_NAME} FILES ${source} ${includes} ${impl_includes} )

ADD_LIBRARY( ${TARGET_NAME} ${source} ${includes} ${impl_source} ${impl_includes} )
SET_TARGET_PROPERTIES( ${TARGET_NAME} PROPERTIES ${comma_LIBRARY_PROPERTIES} )
target_link_libraries( ${TARGET_NAME} comma_base ${comma_ALL_EXTERNAL_LIBRARIES} )

# INSTALL( FILES ${includes} DESTINATION ${comma_INSTALL_INCLUDE_DIR}/${PROJECT}/ )
# INSTALL( FILES ${impl_includes} DESTINATION ${comma_INSTALL_INCLUDE_DIR}/${PROJECT}/impl )
INSTALL(
    TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime     # .exe, .dll
    LIBRARY DESTINATION ${comma_INSTALL_LIB_DIR} COMPONENT Runtime     # .so, mod.dll
    ARCHIVE DESTINATION ${comma_INSTALL_LIB_DIR} COMPONENT Development # .a, .lib
)
