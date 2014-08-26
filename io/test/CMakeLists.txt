SET( KIT io )

FILE( GLOB source ${SOURCE_CODE_BASE_DIR}/${KIT}/test/*_test.cpp )

ADD_EXECUTABLE( test_${KIT} ${source} )

TARGET_LINK_LIBRARIES( test_${KIT} comma_base comma_string comma_io ${Boost_LIBRARIES} ${GTEST_BOTH_LIBRARIES} pthread rt )
