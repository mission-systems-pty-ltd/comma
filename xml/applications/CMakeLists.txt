SET( dir ${SOURCE_CODE_BASE_DIR}/xml/applications )
FILE( GLOB source   ${dir}/*.cpp)
FILE( GLOB includes ${dir}/*.h)
SOURCE_GROUP( ${TARGET_NAME} FILES ${source} ${includes} )

IF( comma_BUILD_XML )
    FIND_LIBRARY( EXPAT_LIBRARY NAMES expat )
    ADD_DEFINITIONS( -DUSE_EXPAT )
    INCLUDE_DIRECTORIES( ${EXPAT_INCLUDE_PATH} )

    FILE( GLOB util_source ${dir}/*util*.cpp )
    add_executable( xml-map-split ${dir}/xml-map-split.cpp ${util_source} )
    add_executable( xml-grep ${dir}/xml-grep.cpp ${util_source} )
    add_executable( xml-map ${dir}/xml-map.cpp ${util_source} )
    add_executable( xml-split ${dir}/xml-split.cpp ${util_source} )
    
    target_link_libraries ( xml-map-split ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io comma_xpath ${EXPAT_LIBRARY} )
    target_link_libraries ( xml-grep ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io comma_xpath ${EXPAT_LIBRARY} )
    target_link_libraries ( xml-map ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io  comma_xpath ${EXPAT_LIBRARY} )
    target_link_libraries ( xml-split ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io comma_xpath ${EXPAT_LIBRARY} )

    install( TARGETS xml-grep
                    xml-map
                    xml-map-split
                    xml-split
            RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR}
            COMPONENT Runtime )
ENDIF( comma_BUILD_XML )

