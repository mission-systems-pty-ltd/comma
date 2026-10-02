set( PROJECT "csv" )
set( TARGET_NAME comma_${PROJECT} )
set( dir ${SOURCE_CODE_BASE_DIR}/${PROJECT} )

file( GLOB source ${dir}/*.cpp )
file( GLOB includes ${dir}/*.h )
file( GLOB impl_source ${dir}/impl/*.cpp )
file( GLOB impl_includes ${dir}/impl/*.h )
file( GLOB named_variant_source ${dir}/named_variant/*.cpp )
file( GLOB named_variant_includes ${dir}/named_variant/*.h )

add_library( ${TARGET_NAME} ${source} ${includes} ${impl_source} ${impl_includes} ${named_variant_source} ${named_variant_includes} )
SET_TARGET_PROPERTIES( ${TARGET_NAME} PROPERTIES ${comma_LIBRARY_PROPERTIES} )
target_link_libraries( ${TARGET_NAME} comma_application comma_timing comma_xpath ${comma_ALL_EXTERNAL_LIBRARIES} )

INSTALL( FILES ${includes} DESTINATION ${comma_INSTALL_INCLUDE_DIR}/${PROJECT}/ )
INSTALL( FILES ${impl_includes} DESTINATION ${comma_INSTALL_INCLUDE_DIR}/${PROJECT}/impl )
INSTALL( FILES ${named_variant_includes} DESTINATION ${comma_INSTALL_INCLUDE_DIR}/${PROJECT}/named_variant )
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
