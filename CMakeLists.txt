INSTALL( PROGRAMS aero-bash-util
                  csv-from-bin-schema
                  csv-merge-bin-schema
                  csv-replace
                  directs-airport-navaids
                  directs-from-dpacn
                  directs-to-extras
                  google-map-create
                  graph-find
                  mesh-to-edges
         DESTINATION ${aero_INSTALL_BIN_DIR} )

add_executable(qadrax-mesh-to-csv qadrax-mesh-to-csv.cpp )
target_link_libraries(qadrax-mesh-to-csv aero_geometry ${comma_ALL_LIBRARIES})

INSTALL( TARGETS qadrax-mesh-to-csv
         RUNTIME DESTINATION ${aero_INSTALL_BIN_DIR}
         COMPONENT Runtime )

