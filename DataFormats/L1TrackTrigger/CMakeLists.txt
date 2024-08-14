include_directories(${CMAKE_SOURCE_DIR}
${CMAKE_SOURCE_DIR}/hls
/usr/include
)
cms_rootdict( DataFormatsL1TrackTrigger  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsL1TrackTrigger )
add_library( DataFormatsL1TrackTrigger  SHARED ${DataFormatsL1TrackTrigger_EXTRA_SOURCES}
  src/TTCluster.cc
  src/TTDTC.cc
  src/TTTrack.cc
  src/TTTrack_TrackWord.cc
)
set_target_properties( DataFormatsL1TrackTrigger PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsL1TrackTrigger )
target_include_directories( DataFormatsL1TrackTrigger PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsL1TrackTrigger PUBLIC
  ${CMAKE_SOURCE_DIR}/hls
/usr/include
)
target_link_libraries( DataFormatsL1TrackTrigger PUBLIC
   DataFormatsCommon
   DataFormatsDetId
   DataFormatsGeometryCommonDetAlgo
   DataFormatsGeometryVector
   DataFormatsMath
   DataFormatsPhase2TrackerDigi
   DataFormatsSiStripDetId
   DataFormatsTrackerCommon
   FWCoreMessageLogger
   FWCoreUtilities
)
install( TARGETS  DataFormatsL1TrackTrigger DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
