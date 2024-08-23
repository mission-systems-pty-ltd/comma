SET( dir ${SOURCE_CODE_BASE_DIR}/io/applications )
FILE( GLOB source   ${dir}/*.cpp)
FILE( GLOB includes ${dir}/*.h)
SOURCE_GROUP( ${TARGET_NAME} FILES ${source} ${includes} )

IF( comma_BUILD_ZEROMQ )
ADD_EXECUTABLE( zero-cat ${dir}/zero-cat.cpp )
TARGET_LINK_LIBRARIES ( zero-cat ${ZeroMQ_LIBRARY} comma_io comma_application ${comma_ALL_EXTERNAL_LIBRARIES} )
set_target_properties( zero-cat PROPERTIES LINK_FLAGS_RELEASE -s )
INSTALL( TARGETS zero-cat
         RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR}
         COMPONENT Runtime )
ENDIF( comma_BUILD_ZEROMQ )

add_executable( io-buffer ${dir}/io-buffer.cpp )
target_link_libraries( io-buffer comma_application comma_io comma_csv comma_base ) # profiler )
set_target_properties( io-buffer PROPERTIES LINK_FLAGS_RELEASE -s )
install( TARGETS io-buffer RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

if( NOT WIN32 )
    add_executable( io-bandwidth ${dir}/io-bandwidth.cpp )
    target_link_libraries( io-bandwidth comma_application comma_io )
    set_target_properties( io-bandwidth PROPERTIES LINK_FLAGS_RELEASE -s )
    install( TARGETS io-bandwidth RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

    add_executable( io-cat ${dir}/io-cat.cpp )
    target_link_libraries( io-cat comma_application comma_io comma_csv comma_base ) # profiler )
    set_target_properties( io-cat PROPERTIES LINK_FLAGS_RELEASE -s )
    install( TARGETS io-cat RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

    add_executable( io-line io-line.cpp )
    target_link_libraries ( io-line comma_application )
    set_target_properties( io-line PROPERTIES LINK_FLAGS_RELEASE -s )
    install( TARGETS io-line RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

    add_executable( io-ls ${dir}/io-ls.cpp )
    target_link_libraries( io-ls comma_application comma_csv comma_xpath )
    set_target_properties( io-ls PROPERTIES LINK_FLAGS_RELEASE -s )
    install( TARGETS io-ls RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )
    
    add_executable( io-publish ${dir}/io-publish.cpp ${dir}/detail/publish.h ${dir}/detail/publish.cpp )
    target_link_libraries( io-publish comma_base comma_io comma_application comma_xpath comma_name_value ) # profiler )
    set_target_properties( io-publish PROPERTIES LINK_FLAGS_RELEASE -s )
    install( TARGETS io-publish RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

    add_executable( io-tee ${dir}/io-tee.cpp )
    target_link_libraries( io-tee comma_application comma_io comma_csv comma_base ) # profiler )
    set_target_properties( io-tee PROPERTIES LINK_FLAGS_RELEASE -s )
    install( TARGETS io-tee RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )
    
    add_executable( udp-client ${dir}/udp-client.cpp ${dir}/detail/publish.h ${dir}/detail/publish.cpp )
    target_link_libraries( udp-client comma_application comma_name_value comma_io comma_csv comma_base ) # profiler )
    set_target_properties( udp-client PROPERTIES LINK_FLAGS_RELEASE -s )
    install( TARGETS udp-client RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )
    
    add_executable( io-console ${dir}/io-console.cpp )
    target_link_libraries( io-console comma_base comma_io comma_application ) # profiler )
    set_target_properties( io-console PROPERTIES LINK_FLAGS_RELEASE -s )
    install( TARGETS io-console RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )
endif( NOT WIN32 )

install( PROGRAMS io-topics DESTINATION ${comma_INSTALL_BIN_DIR})
option( comma_build_io_rabbit_cat "build rabbit-cat" ON )
if( comma_build_io_rabbit_cat )
    install( PROGRAMS rabbit-cat DESTINATION ${comma_INSTALL_BIN_DIR})
endif( comma_build_io_rabbit_cat )
install( PROGRAMS zero-publish DESTINATION ${comma_INSTALL_BIN_DIR})

