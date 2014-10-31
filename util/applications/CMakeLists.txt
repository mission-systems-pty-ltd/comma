EXECUTE_PROCESS(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion
                OUTPUT_VARIABLE GCC_VERSION)
IF(GCC_VERSION VERSION_EQUAL 4.7 OR GCC_VERSION VERSION_GREATER 4.7)
  MESSAGE(STATUS "gcc >= 4.7")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11" )

  add_executable(comma-progress comma-progress.cpp )
  add_executable(comma-nap comma-nap.c )
  target_link_libraries(comma-progress boost_system boost_date_time ${comma_ALL_EXTERNAL_LIBRARIES} comma_csv comma_xpath)

  INSTALL( TARGETS comma-nap comma-progress
           RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR}
           COMPONENT Runtime )

ENDIF(GCC_VERSION VERSION_EQUAL 4.7 OR GCC_VERSION VERSION_GREATER 4.7)

INSTALL( PROGRAMS comma-profile-graph DESTINATION ${comma_INSTALL_BIN_DIR} )
INSTALL( PROGRAMS comma-stats DESTINATION ${comma_INSTALL_BIN_DIR} )
install( PROGRAMS comma-test-match DESTINATION ${comma_INSTALL_BIN_DIR} )
install( PROGRAMS comma-test-run DESTINATION ${comma_INSTALL_BIN_DIR} )
install( PROGRAMS comma-top DESTINATION ${comma_INSTALL_BIN_DIR} )
