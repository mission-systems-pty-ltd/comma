SET( PROJECT "name_value" )
SET( TARGET_NAME comma_${PROJECT} )

FILE( GLOB source   ${SOURCE_CODE_BASE_DIR}/${PROJECT}/*.cpp)
FILE( GLOB includes ${SOURCE_CODE_BASE_DIR}/${PROJECT}/*.h )
FILE( GLOB impl_includes ${SOURCE_CODE_BASE_DIR}/${PROJECT}/impl/*.h )
SOURCE_GROUP( ${PROJECT} FILES ${source} ${includes} ${impl_includes} )
#ADD_CUSTOM_TARGET( ${TARGET_NAME} ${source} ${includes} ${impl_includes} comma_application comma_string comma_xpath )

if( comma_BUILD_NAME_VALUE_YAML )
    # find_package( yaml REQUIRED ) # todo
    set( CMAKE_CXX_FLAGS "-Dcomma_BUILD_NAME_VALUE_YAML ${CMAKE_CXX_FLAGS}" ) # todo? move to the main CMakeLists.txt?
    set( source "${source};${SOURCE_CODE_BASE_DIR}/${PROJECT}/impl/yaml.cpp" )
endif( comma_BUILD_NAME_VALUE_YAML )
INSTALL( FILES ${includes} DESTINATION ${comma_INSTALL_INCLUDE_DIR}/${PROJECT} )
INSTALL( FILES ${impl_includes} DESTINATION ${comma_INSTALL_INCLUDE_DIR}/${PROJECT}/impl )

ADD_LIBRARY( ${TARGET_NAME} ${source} ${includes} )
SET_TARGET_PROPERTIES( ${TARGET_NAME} PROPERTIES ${comma_LIBRARY_PROPERTIES} )
target_link_libraries( ${TARGET_NAME} comma_string comma_xpath ${comma_ALL_EXTERNAL_LIBRARIES} ) # comma_application
if( comma_BUILD_NAME_VALUE_YAML )
    target_link_libraries( ${TARGET_NAME} yaml )
endif( comma_BUILD_NAME_VALUE_YAML )
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
