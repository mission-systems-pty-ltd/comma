set( dir ${SOURCE_CODE_BASE_DIR}/containers/applications )

add_executable( math-lookup math-lookup.cpp ) # todo! should math-lookup be here at all? does it create circular dependencies?
#add_dependencies( math-lookup comma_containers )
target_link_libraries( math-lookup ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_csv )
set_target_properties( math-lookup PROPERTIES LINK_FLAGS_RELEASE -s )
install( TARGETS math-lookup RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )
