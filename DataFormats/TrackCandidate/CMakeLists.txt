include_directories(${CMAKE_SOURCE_DIR}
${CLHEP_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsTrackCandidate  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsTrackCandidate )
add_library( DataFormatsTrackCandidate  SHARED ${DataFormatsTrackCandidate_EXTRA_SOURCES}
  src/TrajectoryStopReasons.cc
)
set_target_properties( DataFormatsTrackCandidate PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsTrackCandidate )
target_include_directories( DataFormatsTrackCandidate PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsTrackCandidate PUBLIC
  ${CLHEP_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsTrackCandidate PUBLIC
   DataFormatsCommon
   DataFormatsCLHEP
   DataFormatsTrackingRecHit
   DataFormatsTrajectorySeed
   FWCoreUtilities
)
install( TARGETS  DataFormatsTrackCandidate DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
