FILE( GLOB source   *.cpp )
FILE( GLOB includes *.h )
SOURCE_GROUP( ${TARGET_NAME} FILES ${source} ${includes} )

ADD_EXECUTABLE( comma-options-to-name-value comma-options-to-name-value.cpp )
TARGET_LINK_LIBRARIES ( comma-options-to-name-value ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_string )
INSTALL( TARGETS comma-options-to-name-value RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

ADD_EXECUTABLE( comma-options-validate comma-options-validate.cpp )
TARGET_LINK_LIBRARIES ( comma-options-validate ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_string )
INSTALL( TARGETS comma-options-validate RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )
