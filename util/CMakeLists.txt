ADD_SUBDIRECTORY( applications )
SET( dir ${SOURCE_CODE_BASE_DIR}/util )

ADD_EXECUTABLE( comma-nap ${dir}/comma-nap.c )

INSTALL( TARGETS comma-nap
         RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR}
         COMPONENT Runtime )

install( PROGRAMS comma-util DESTINATION ${comma_INSTALL_BIN_DIR} )
install( PROGRAMS resources-util DESTINATION ${comma_INSTALL_BIN_DIR} )
