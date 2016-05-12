SET( dir ${SOURCE_CODE_BASE_DIR}/io/applications )
FILE( GLOB source   ${dir}/*.cpp)
FILE( GLOB includes ${dir}/*.h)
SOURCE_GROUP( ${TARGET_NAME} FILES ${source} ${includes} )

IF( comma_BUILD_ZEROMQ )
ADD_EXECUTABLE( zero-cat ${dir}/zero-cat.cpp )
TARGET_LINK_LIBRARIES ( zero-cat ${ZeroMQ_LIBRARY} comma_io comma_application ${comma_ALL_EXTERNAL_LIBRARIES} )
INSTALL( TARGETS zero-cat
         RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR}
         COMPONENT Runtime )
ENDIF( comma_BUILD_ZEROMQ )

add_executable( io-buffer ${dir}/io-buffer.cpp )
target_link_libraries( io-buffer comma_application comma_io comma_csv comma_base ${comma_EXTERNAL_ALL_LIBRARIES} ) # profiler )
install( TARGETS io-buffer RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

if( NOT WIN32 )
    add_executable( io-cat ${dir}/io-cat.cpp )
    target_link_libraries( io-cat comma_application comma_io comma_csv comma_base ${comma_EXTERNAL_ALL_LIBRARIES} ) # profiler )
    install( TARGETS io-cat RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )
    
    add_executable( udp-client ${dir}/udp-client.cpp )
    target_link_libraries( udp-client comma_application comma_io comma_csv comma_base ${comma_EXTERNAL_ALL_LIBRARIES} ) # profiler )
    install( TARGETS udp-client RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )
    
    add_executable( io-publish ${dir}/io-publish.cpp ${dir}/publish.h ${dir}/publish.cpp )
    target_link_libraries( io-publish ${comma_EXTERNAL_ALL_LIBRARIES} comma_base comma_io comma_application ) # profiler )
    install( TARGETS io-publish RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

    add_executable( io-tee-worker ${dir}/io-tee-worker.cpp )
    target_link_libraries( io-tee-worker comma_application comma_io comma_csv comma_base ${comma_EXTERNAL_ALL_LIBRARIES} ) # profiler )
    install( TARGETS io-tee-worker RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

    add_executable( io-ls ${dir}/io-ls.cpp )
    target_link_libraries( io-ls ${comma_ALL_LIBRARIES} ${comma_EXTERNAL_ALL_LIBRARIES} )
    install( TARGETS io-ls RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

endif( NOT WIN32 )

install( PROGRAMS zero-publish io-tee DESTINATION ${comma_INSTALL_BIN_DIR})

