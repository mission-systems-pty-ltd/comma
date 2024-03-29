SET( dir ${SOURCE_CODE_BASE_DIR}/name_value/applications )
FILE( GLOB source   ${dir}/*.cpp)
FILE( GLOB includes ${dir}/*.h)
SOURCE_GROUP( ${TARGET_NAME} FILES ${source} ${includes} )
ADD_EXECUTABLE( name-value-convert ${dir}/name-value-convert.cpp )
ADD_EXECUTABLE( name-value-get ${dir}/name-value-get.cpp )
ADD_EXECUTABLE( name-value-eval-preparse ${dir}/name-value-eval-preparse.cpp )
ADD_EXECUTABLE( name-value-permute ${dir}/name-value-permute.cpp )

TARGET_LINK_LIBRARIES( name-value-convert comma_application comma_string comma_xpath comma_name_value )
TARGET_LINK_LIBRARIES( name-value-get comma_application comma_string comma_xpath comma_name_value )
TARGET_LINK_LIBRARIES( name-value-eval-preparse comma_application comma_name_value )
TARGET_LINK_LIBRARIES( name-value-permute comma_application comma_string comma_xpath comma_name_value )

set_target_properties( name-value-convert PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( name-value-get PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( name-value-eval-preparse PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( name-value-permute PROPERTIES LINK_FLAGS_RELEASE -s )

INSTALL( TARGETS name-value-convert
                 name-value-get
                 name-value-eval-preparse
                 name-value-permute
                 RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )
         
add_executable( name-value-from-csv ${dir}/name-value-from-csv.cpp )
target_link_libraries( name-value-from-csv comma_application comma_string )
set_target_properties( name-value-from-csv PROPERTIES LINK_FLAGS_RELEASE -s )
install( TARGETS name-value-from-csv RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

add_executable( name-value-to-csv ${dir}/name-value-to-csv.cpp )
target_link_libraries( name-value-to-csv comma_application comma_string )
set_target_properties( name-value-to-csv PROPERTIES LINK_FLAGS_RELEASE -s )
install( TARGETS name-value-to-csv RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

install( PROGRAMS name-value-apply DESTINATION ${comma_INSTALL_BIN_DIR} )
install( PROGRAMS name-value-calc DESTINATION ${comma_INSTALL_BIN_DIR} )
install( PROGRAMS name-value-eval DESTINATION ${comma_INSTALL_BIN_DIR} )
install( PROGRAMS name-value-forms DESTINATION ${comma_INSTALL_BIN_DIR} )
