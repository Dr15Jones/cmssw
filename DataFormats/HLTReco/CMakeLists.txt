cms_rootdict( DataFormatsHLTReco  src/classes.h src/classes_def.xml)
add_library( DataFormatsHLTReco  SHARED ${DataFormatsHLTReco_EXTRA_SOURCES}
  src/EgammaObject.cc
  src/HLTPerformanceInfo.cc
)
set_target_properties( DataFormatsHLTReco PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsHLTReco )
target_include_directories( DataFormatsHLTReco PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsHLTReco PUBLIC
/usr/include
)
target_link_libraries( DataFormatsHLTReco PUBLIC
   FWCoreUtilities
   DataFormatsCommon
   DataFormatsCandidate
   DataFormatsL1Trigger
   DataFormatsRecoCandidate
   DataFormatsEgammaCandidates
   DataFormatsEgammaReco
   DataFormatsJetReco
   DataFormatsMETReco
   DataFormatsHcalIsolatedTrack
   DataFormatsTauReco
   DataFormatsL1TParticleFlow
   DataFormatsL1TCorrelator
   DataFormatsL1TMuonPhase2
   DataFormatsGsfTrackReco
)
add_rootdict_rules( DataFormatsHLTReco )
install( TARGETS  DataFormatsHLTReco DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
