include_directories(${CMAKE_SOURCE_DIR}
${ROOT_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsL1TMuon  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsL1TMuon )
add_library( DataFormatsL1TMuon  SHARED ${DataFormatsL1TMuon_EXTRA_SOURCES}
  src/CPPFDigi.cc
  src/EMTFDaqOut.cc
  src/EMTFHit.cc
  src/EMTFRoad.cc
  src/EMTFTrack.cc
  src/L1MuBMTrack.cc
  src/L1MuBMTrackSegEta.cc
  src/L1MuBMTrackSegPhi.cc
  src/L1MuKBMTCombinedStub.cc
  src/L1MuKBMTrack.cc
  src/RegionalMuonCand.cc
  src/RegionalMuonShower.cc
)
set_target_properties( DataFormatsL1TMuon PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsL1TMuon )
target_include_directories( DataFormatsL1TMuon PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsL1TMuon PUBLIC
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsL1TMuon PUBLIC
   ROOT::Core
   DataFormatsCSCDigi
   DataFormatsGEMDigi
   DataFormatsMuonDetId
   DataFormatsCommon
   DataFormatsCandidate
   DataFormatsL1Trigger
)
install( TARGETS  DataFormatsL1TMuon DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
