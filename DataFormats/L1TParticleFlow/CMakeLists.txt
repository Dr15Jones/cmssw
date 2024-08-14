include_directories(${CMAKE_SOURCE_DIR}
${ROOT_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsL1TParticleFlow  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsL1TParticleFlow )
add_library( DataFormatsL1TParticleFlow  SHARED ${DataFormatsL1TParticleFlow_EXTRA_SOURCES}
  src/HPSPFTau.cc
  src/PFCandidate.cc
  src/PFCluster.cc
  src/PFJet.cc
  src/PFTau.cc
  src/PFTrack.cc
)
set_target_properties( DataFormatsL1TParticleFlow PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsL1TParticleFlow )
target_include_directories( DataFormatsL1TParticleFlow PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsL1TParticleFlow PUBLIC
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsL1TParticleFlow PUBLIC
   ROOT::Core
   DataFormatsL1Trigger
   DataFormatsL1TrackTrigger
   DataFormatsCandidate
   DataFormatsCommon
   FWCoreUtilities
   DataFormatsJetReco
   DataFormatsL1TMuonPhase2
)
install( TARGETS  DataFormatsL1TParticleFlow DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
