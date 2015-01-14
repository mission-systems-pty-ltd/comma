add_executable( comma-progress comma-progress.cpp )
target_link_libraries(comma-progress boost_system boost_date_time ${comma_ALL_EXTERNAL_LIBRARIES} comma_csv comma_xpath)
INSTALL( TARGETS comma-progress RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

add_executable(comma-nap comma-nap.cpp )
target_link_libraries(comma-nap boost_date_time boost_thread boost_system )
INSTALL( TARGETS comma-nap RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

INSTALL( PROGRAMS comma-profile-graph DESTINATION ${comma_INSTALL_BIN_DIR} )
INSTALL( PROGRAMS comma-stats DESTINATION ${comma_INSTALL_BIN_DIR} )
install( PROGRAMS comma-test-match DESTINATION ${comma_INSTALL_BIN_DIR} )
install( PROGRAMS comma-test-run DESTINATION ${comma_INSTALL_BIN_DIR} )
install( PROGRAMS comma-top DESTINATION ${comma_INSTALL_BIN_DIR} )
