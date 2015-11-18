SET( kit packed )
SET( dir ${SOURCE_CODE_BASE_DIR}/${kit}/test )
FILE( GLOB source ${dir}/*_test.cpp )
FILE( GLOB extras ${dir}/*.cpp ${dir}/*.h )
ADD_EXECUTABLE( test_${kit} ${source} )
TARGET_LINK_LIBRARIES( test_${kit} comma_base comma_string ${comma_ALL_EXTERNAL_LIBRARIES} ${GTEST_BOTH_LIBRARIES} )

INSTALL ( 
		FILES ${PROJECT_BINARY_DIR}/bin/test_${KIT}
		PERMISSIONS WORLD_READ GROUP_READ OWNER_READ OWNER_WRITE WORLD_EXECUTE
		DESTINATION ${comma_TESTS_DIR} )