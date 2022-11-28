SET( dir ${SOURCE_CODE_BASE_DIR}/csv/applications )
FILE( GLOB source   ${dir}/*.cpp)
FILE( GLOB includes ${dir}/*.h)
FILE( GLOB play_source   ${dir}/play/*.cpp)
FILE( GLOB play_includes ${dir}/play/*.h)
SOURCE_GROUP( ${TARGET_NAME} FILES ${play_source} ${play_includes} ${source} ${includes} )

add_executable( csv-quote csv-quote.cpp )
target_link_libraries( csv-quote ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_string comma_csv )
set_target_properties( csv-quote PROPERTIES LINK_FLAGS_RELEASE -s )
install( TARGETS csv-quote RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

add_executable( csv-fields ${dir}/csv-fields.cpp )
target_link_libraries ( csv-fields ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_string comma_csv )
set_target_properties( csv-fields PROPERTIES LINK_FLAGS_RELEASE -s )
install( TARGETS csv-fields RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

add_executable( csv-format ${dir}/csv-format.cpp )
add_executable( csv-size ${dir}/csv-size.cpp )
add_executable( csv-select ${dir}/csv-select.cpp )
add_executable( csv-bin-cut ${dir}/csv-bin-cut.cpp )
add_executable( csv-from-columns ${dir}/csv-from-columns.cpp )
add_executable( csv-join ${dir}/csv-join.cpp )
add_executable( csv-sort ${dir}/csv-sort.cpp )
add_executable( csv-paste ${dir}/csv-paste.cpp )
add_executable( csv-split ${dir}/csv-split.cpp ${dir}/split/split.cpp ${dir}/split/split.h )
add_executable( csv-time ${dir}/csv-time.cpp )
add_executable( csv-time-delay ${dir}/csv-time-delay.cpp )
add_executable( csv-time-join ${dir}/csv-time-join.cpp )
add_executable( csv-time-stamp ${dir}/csv-time-stamp.cpp )
add_executable( csv-to-bin ${dir}/csv-to-bin.cpp )
add_executable( csv-from-bin ${dir}/csv-from-bin.cpp )
add_executable( csv-calc ${dir}/csv-calc.cpp )
add_executable( csv-calc-new ${dir}/csv-calc.new.cpp )
add_executable( csv-crc ${dir}/csv-crc.cpp )
add_executable( csv-play ${dir}/csv-play.cpp ${dir}/play/multiplay.cpp ${dir}/play/play.cpp )
add_executable( csv-shape ${dir}/csv-shape.cpp )
add_executable( csv-shuffle ${dir}/csv-shuffle.cpp )
add_executable( csv-thin ${dir}/csv-thin.cpp )
add_executable( csv-analyse ${dir}/csv-analyse.cpp )
add_executable( csv-to-sql ${dir}/csv-to-sql.cpp )

target_link_libraries ( csv-format ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_string comma_csv )
target_link_libraries ( csv-size ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_string comma_csv )
target_link_libraries ( csv-bin-cut ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_string comma_csv comma_xpath )
target_link_libraries ( csv-split comma_csv comma_application comma_io comma_string comma_xpath comma_name_value ${comma_ALL_EXTERNAL_LIBRARIES} )
target_link_libraries ( csv-from-columns ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io comma_string )
target_link_libraries ( csv-join ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_csv comma_io comma_xpath comma_string comma_name_value )
target_link_libraries ( csv-sort ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_csv comma_io comma_xpath comma_string )
target_link_libraries ( csv-select ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_csv comma_xpath comma_string comma_name_value )
target_link_libraries ( csv-paste ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_string comma_csv comma_io comma_name_value )
target_link_libraries ( csv-time ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_csv comma_io comma_xpath comma_string )
target_link_libraries ( csv-time-delay ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_csv comma_string comma_xpath )
target_link_libraries ( csv-time-join ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_csv comma_io comma_string comma_xpath comma_name_value )
target_link_libraries ( csv-time-stamp ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_csv comma_string )
target_link_libraries ( csv-to-bin ${comma_ALL_EXTERNAL_LIBRARIES} comma_csv comma_xpath comma_application ) # profiler )
target_link_libraries ( csv-from-bin ${comma_ALL_EXTERNAL_LIBRARIES} comma_csv comma_xpath comma_application )
target_link_libraries ( csv-calc ${comma_ALL_EXTERNAL_LIBRARIES} comma_csv comma_xpath comma_application comma_string )
target_link_libraries ( csv-calc-new ${comma_ALL_EXTERNAL_LIBRARIES} comma_csv comma_xpath comma_application comma_string )
target_link_libraries ( csv-crc ${comma_ALL_EXTERNAL_LIBRARIES} comma_csv comma_xpath comma_application comma_string )
target_link_libraries ( csv-play ${comma_ALL_EXTERNAL_LIBRARIES} comma_csv comma_xpath comma_application comma_io comma_name_value )
target_link_libraries ( csv-shape ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_string comma_csv )
target_link_libraries ( csv-shuffle ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_string comma_csv )
target_link_libraries ( csv-thin ${comma_ALL_EXTERNAL_LIBRARIES} comma_csv comma_xpath comma_application comma_io )
target_link_libraries ( csv-analyse ${comma_ALL_EXTERNAL_LIBRARIES} comma_application )
target_link_libraries ( csv-to-sql ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_string comma_csv )

set_target_properties( csv-bin-cut PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-format PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-join PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-sort PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-from-columns PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-paste PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-split PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-time PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-time-delay PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-time-join PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-time-stamp PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-to-bin PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-from-bin PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-size PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-calc PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-play PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-shape PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-shuffle PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-crc PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-select PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-thin PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-analyse PROPERTIES LINK_FLAGS_RELEASE -s )
set_target_properties( csv-to-sql PROPERTIES LINK_FLAGS_RELEASE -s )

install( TARGETS csv-bin-cut
                 csv-fields
                 csv-format
                 csv-join
                 csv-sort
                 csv-from-columns
                 csv-paste
                 csv-split
                 csv-time
                 csv-time-delay
                 csv-time-join
                 csv-time-stamp
                 csv-to-bin
                 csv-from-bin
                 csv-size
                 csv-calc
                 csv-play
                 csv-shape
                 csv-shuffle
                 csv-crc
                 csv-select
                 csv-thin
                 csv-analyse
                 csv-to-sql
         RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR}
         COMPONENT Runtime )

install ( PROGRAMS csv-gate DESTINATION ${comma_INSTALL_BIN_DIR} )

add_executable( csv-blocks ${dir}/csv-blocks.cpp )
target_link_libraries ( csv-blocks ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io comma_string comma_xpath comma_csv )
set_target_properties( csv-blocks PROPERTIES LINK_FLAGS_RELEASE -s )
install( TARGETS csv-blocks RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

add_executable( csv-cast ${dir}/csv-cast.cpp )
target_link_libraries ( csv-cast ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_csv )
set_target_properties( csv-cast PROPERTIES LINK_FLAGS_RELEASE -s )
install( TARGETS csv-cast RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

add_executable( csv-enumerate ${dir}/csv-enumerate.cpp )
target_link_libraries ( csv-enumerate ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io comma_string comma_xpath comma_csv )
set_target_properties( csv-enumerate PROPERTIES LINK_FLAGS_RELEASE -s )
install( TARGETS csv-enumerate RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

add_executable( csv-intervals ${dir}/csv-intervals.cpp )
target_link_libraries ( csv-intervals ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_csv comma_io comma_xpath comma_name_value )
set_target_properties( csv-intervals PROPERTIES LINK_FLAGS_RELEASE -s )
install( TARGETS csv-intervals RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )
         
add_executable( csv-units ${dir}/csv-units.cpp )
target_link_libraries ( csv-units ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_csv comma_io comma_xpath comma_string )
set_target_properties( csv-units PROPERTIES LINK_FLAGS_RELEASE -s )
install( TARGETS csv-units RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

add_executable( csv-random ${dir}/csv-random.cpp )
target_link_libraries ( csv-random ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io comma_string comma_xpath comma_csv )
set_target_properties( csv-random PROPERTIES LINK_FLAGS_RELEASE -s )
install( TARGETS csv-random RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

add_executable( csv-strings ${dir}/csv-strings.cpp )
target_link_libraries ( csv-strings ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io comma_string comma_xpath comma_csv )
set_target_properties( csv-strings PROPERTIES LINK_FLAGS_RELEASE -s )
install( TARGETS csv-strings RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

add_executable( csv-update ${dir}/csv-update.cpp )
target_link_libraries ( csv-update ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_io comma_string comma_xpath comma_csv )
set_target_properties( csv-update PROPERTIES LINK_FLAGS_RELEASE -s )
install( TARGETS csv-update RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

if( NOT WIN32 )
    add_executable( csv-repeat ${dir}/csv-repeat.cpp )
    target_link_libraries ( csv-repeat comma_application comma_csv comma_io )
    set_target_properties( csv-repeat PROPERTIES LINK_FLAGS_RELEASE -s )
    install( TARGETS csv-repeat RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )
endif()

add_executable( csv-bin-reverse ${dir}/csv-bin-reverse.cpp )
target_link_libraries ( csv-bin-reverse ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_string comma_xpath comma_csv )
set_target_properties( csv-bin-reverse PROPERTIES LINK_FLAGS_RELEASE -s )
install( TARGETS csv-bin-reverse RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )

add_executable( csv-bits ${dir}/csv-bits.cpp )
target_link_libraries ( csv-bits ${comma_ALL_EXTERNAL_LIBRARIES} comma_application comma_csv comma_string )
set_target_properties( csv-bits PROPERTIES LINK_FLAGS_RELEASE -s )
install( TARGETS csv-bits RUNTIME DESTINATION ${comma_INSTALL_BIN_DIR} COMPONENT Runtime )
