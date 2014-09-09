SET( dir ${SOURCE_CODE_BASE_DIR}/xpath/applications )
FILE( GLOB source   ${dir}/*.cpp)
FILE( GLOB includes ${dir}/*.h)
SOURCE_GROUP( ${TARGET_NAME} FILES ${source} ${includes} )

IF( comma_BUILD_EXPAT )

add_executable( xml-map-split ${dir}/xml-map-split.cpp )

SOURCE_GROUP( comma_xpath_application FILES stream_util.cpp stream_util.h expat_util.cpp expat_util.h )
ADD_LIBRARY( comma_xpath_application ${source} ${includes} )

add_executable( xml-grep ${dir}/xml-grep.cpp )
add_executable( xml-map ${dir}/xml-map.cpp )
add_executable( xml-split ${dir}/xml-split.cpp )

target_link_libraries ( xml-map-split ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io comma_xpath_application )
target_link_libraries ( xml-grep ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io comma_xpath comma_xpath_application )
target_link_libraries ( xml-map ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io  comma_xpath comma_xpath_application )
target_link_libraries ( xml-split ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io comma_xpath comma_xpath_application )

install( TARGETS xml-grep
                 xml-map
                 xml-map-split
                 xml-split
         RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR}
         COMPONENT Runtime )
ENDIF( comma_BUILD_EXPAT )

