file( GLOB source ${SOURCE_CODE_BASE_DIR}/io/test/*test.cpp )
set( test_name ${CMAKE_PROJECT_NAME}_test_io )
add_executable( ${test_name} ${source} )
target_link_libraries( ${test_name} comma_io ${GTEST_BOTH_LIBRARIES} pthread )
add_test( NAME ${test_name} COMMAND ${CMAKE_PROJECT_NAME}_test_io WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/bin )
if( INSTALL_TESTS )
    install( TARGETS ${test_name} RUNTIME DESTINATION ${comma_CPP_TESTS_INSTALL_DIR} COMPONENT Runtime )
    #INSTALL (
    #    FILES ${PROJECT_BINARY_DIR}/bin/${CMAKE_PROJECT_NAME}_test_io
    #    PERMISSIONS WORLD_READ GROUP_READ OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
    #    DESTINATION ${comma_CPP_TESTS_INSTALL_DIR} )
endif( INSTALL_TESTS )
