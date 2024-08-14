include_directories(${CMAKE_SOURCE_DIR}
${CMAKE_SOURCE_DIR}/hls
${ROOT_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsL1TMuonPhase2  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsL1TMuonPhase2 )
add_library( DataFormatsL1TMuonPhase2  SHARED ${DataFormatsL1TMuonPhase2_EXTRA_SOURCES}
  src/EMTFHit.cc
  src/EMTFInput.cc
  src/EMTFTrack.cc
  src/MuonStub.cc
  src/SAMuon.cc
  src/TrackerMuon.cc
)
set_target_properties( DataFormatsL1TMuonPhase2 PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsL1TMuonPhase2 )
target_include_directories( DataFormatsL1TMuonPhase2 PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsL1TMuonPhase2 PUBLIC
  ${CMAKE_SOURCE_DIR}/hls
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsL1TMuonPhase2 PUBLIC
   ROOT::Core
   DataFormatsCLHEP
   DataFormatsCommon
   DataFormatsL1Trigger
   DataFormatsL1TMuon
   DataFormatsL1TrackTrigger
)
install( TARGETS  DataFormatsL1TMuonPhase2 DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
