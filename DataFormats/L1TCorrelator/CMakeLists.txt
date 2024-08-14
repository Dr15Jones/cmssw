include_directories(${CMAKE_SOURCE_DIR}
${ROOT_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsL1TCorrelator  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsL1TCorrelator )
add_library( DataFormatsL1TCorrelator  SHARED ${DataFormatsL1TCorrelator_EXTRA_SOURCES}
  src/L1CaloTkTau.cc
  src/L1TrkTau.cc
  src/TkBsCandidate.cc
  src/TkEGTau.cc
  src/TkElectron.cc
  src/TkEm.cc
  src/TkEtMiss.cc
  src/TkGlbMuon.cc
  src/TkHTMiss.cc
  src/TkJet.cc
  src/TkMuon.cc
  src/TkPhiCandidate.cc
  src/TkTau.cc
  src/TkTriplet.cc
)
set_target_properties( DataFormatsL1TCorrelator PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsL1TCorrelator )
target_include_directories( DataFormatsL1TCorrelator PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsL1TCorrelator PUBLIC
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsL1TCorrelator PUBLIC
   ROOT::Core
   DataFormatsHepMCCandidate
   DataFormatsL1TMuon
   DataFormatsMath
   DataFormatsCommon
   DataFormatsL1Trigger
   DataFormatsL1TrackTrigger
)
install( TARGETS  DataFormatsL1TCorrelator DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
