SET( dir ${SOURCE_CODE_BASE_DIR}/xpath/applications )
FILE( GLOB source   ${dir}/*.cpp)
FILE( GLOB includes ${dir}/*.h)
SOURCE_GROUP( ${TARGET_NAME} FILES ${source} ${includes} )

IF( comma_BUILD_EXPAT )

FILE( GLOB util_source ${dir}/*util*.cpp )
add_executable( xml-map-split ${dir}/xml-map-split.cpp ${util_source} )
add_executable( xml-grep ${dir}/xml-grep.cpp ${util_source} )
add_executable( xml-map ${dir}/xml-map.cpp ${util_source} )
add_executable( xml-split ${dir}/xml-split.cpp ${util_source} )

target_link_libraries ( xml-map-split ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io comma_xpath )
target_link_libraries ( xml-grep ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io comma_xpath )
target_link_libraries ( xml-map ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io  comma_xpath )
target_link_libraries ( xml-split ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io comma_xpath )

install( TARGETS xml-grep
                 xml-map
                 xml-map-split
                 xml-split
         RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR}
         COMPONENT Runtime )
ENDIF( comma_BUILD_EXPAT )

