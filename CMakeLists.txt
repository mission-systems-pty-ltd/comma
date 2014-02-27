INSTALL( PROGRAMS aero-bash-util
                  aero-progress
                  csv-from-bin-schema
                  csv-merge-bin-schema
                  csv-replace
                  google-map-create
                  graph-find
                  mesh-to-edges
                  coordinates-format
         DESTINATION ${aero_INSTALL_BIN_DIR} )

add_executable(qadrax-mesh-to-csv qadrax-mesh-to-csv.cpp )
target_link_libraries(qadrax-mesh-to-csv aero_geometry ${comma_ALL_LIBRARIES})
add_executable(aero-progress aero-progress.cpp )
target_link_libraries(aero-progress boost_system boost_date_time ${comma_ALL_LIBRARIES})

INSTALL( TARGETS qadrax-mesh-to-csv
         RUNTIME DESTINATION ${aero_INSTALL_BIN_DIR}
         COMPONENT Runtime )

