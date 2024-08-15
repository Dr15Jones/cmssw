cms_rootdict( DataFormatsPhase2TrackerCluster  src/classes.h src/classes_def.xml)
add_library( DataFormatsPhase2TrackerCluster  SHARED ${DataFormatsPhase2TrackerCluster_EXTRA_SOURCES}
)
set_target_properties( DataFormatsPhase2TrackerCluster PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsPhase2TrackerCluster )
target_include_directories( DataFormatsPhase2TrackerCluster PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsPhase2TrackerCluster PUBLIC
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsPhase2TrackerCluster PUBLIC
   ROOT::Core
   DataFormatsCommon
   DataFormatsPhase2TrackerDigi
)
add_rootdict_rules( DataFormatsPhase2TrackerCluster )
install( TARGETS  DataFormatsPhase2TrackerCluster DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
