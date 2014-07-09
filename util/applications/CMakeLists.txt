INSTALL( PROGRAMS aero-bash-util
                  aero-bash-test
                  comma-stats
         DESTINATION ${comma_INSTALL_BIN_DIR} )

add_executable(comma-progress comma-progress.cpp )
target_link_libraries(comma-progress boost_system boost_date_time ${comma_ALL_LIBRARIES})

INSTALL( TARGETS comma-progress
         RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR}
         COMPONENT Runtime )

