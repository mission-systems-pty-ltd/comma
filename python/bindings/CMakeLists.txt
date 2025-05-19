set( PROJECT "python_bindings" )
set( TARGET_NAME comma_${PROJECT} )
set( dir ${SOURCE_CODE_BASE_DIR}/python/bindings )

file( GLOB_RECURSE source ${dir}/*.cpp )
file( GLOB_RECURSE includes ${dir}/*.h )

source_group( ${TARGET_NAME} FILES ${source} ${includes} )

add_library( ${TARGET_NAME} ${source} ${includes} )
set_target_properties( ${TARGET_NAME} PROPERTIES ${comma_LIBRARY_PROPERTIES} )
target_link_libraries( ${TARGET_NAME} comma_base ${comma_ALL_EXTERNAL_LIBRARIES} )

# INSTALL( FILES ${includes} DESTINATION ${comma_INSTALL_INCLUDE_DIR}/${PROJECT}/ )
install(
    TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime     # .exe, .dll
    LIBRARY DESTINATION ${comma_INSTALL_LIB_DIR} COMPONENT Runtime     # .so, mod.dll
    ARCHIVE DESTINATION ${comma_INSTALL_LIB_DIR} COMPONENT Development # .a, .lib
)
