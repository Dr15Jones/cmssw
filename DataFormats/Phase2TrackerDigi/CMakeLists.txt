include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsPhase2TrackerDigi  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsPhase2TrackerDigi )
add_library( DataFormatsPhase2TrackerDigi  SHARED ${DataFormatsPhase2TrackerDigi_EXTRA_SOURCES}
)
set_target_properties( DataFormatsPhase2TrackerDigi PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsPhase2TrackerDigi )
target_include_directories( DataFormatsPhase2TrackerDigi PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsPhase2TrackerDigi PUBLIC
/usr/include
)
target_link_libraries( DataFormatsPhase2TrackerDigi PUBLIC
   DataFormatsCommon
)
install( TARGETS  DataFormatsPhase2TrackerDigi DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
