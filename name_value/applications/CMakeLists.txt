SET( dir ${SOURCE_CODE_BASE_DIR}/name_value/applications )
FILE( GLOB source   ${dir}/*.cpp)
FILE( GLOB includes ${dir}/*.h)
SOURCE_GROUP( ${TARGET_NAME} FILES ${source} ${includes} )
ADD_EXECUTABLE( name-value-convert ${dir}/name-value-convert.cpp )
ADD_EXECUTABLE( name-value-get ${dir}/name-value-get.cpp )
ADD_EXECUTABLE( name-value-eval-preparse ${dir}/name-value-eval-preparse.cpp )

TARGET_LINK_LIBRARIES( name-value-convert ${comma_EXTERNAL_ALL_LIBRARIES} comma_application comma_string comma_xpath comma_name_value )
TARGET_LINK_LIBRARIES( name-value-get ${comma_EXTERNAL_ALL_LIBRARIES} comma_application comma_string comma_xpath comma_name_value )
TARGET_LINK_LIBRARIES( name-value-eval-preparse ${comma_EXTERNAL_ALL_LIBRARIES} comma_application comma_name_value )

INSTALL( TARGETS name-value-convert name-value-get name-value-eval-preparse
         RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR}
         COMPONENT Runtime )

install( PROGRAMS name-value-eval name-value-calc DESTINATION ${comma_INSTALL_BIN_DIR} )
         
add_executable( name-value-from-csv ${dir}/name-value-from-csv.cpp )
target_link_libraries( name-value-from-csv ${comma_EXTERNAL_ALL_LIBRARIES} comma_application comma_string )
install( TARGETS name-value-from-csv RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )
