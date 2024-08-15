cms_rootdict( DataFormatsJetReco  src/classes_1.h src/classes_def_1.xml)
cms_rootdict( DataFormatsJetReco  src/classes_2.h src/classes_def_2.xml)
cms_rootdict( DataFormatsJetReco  src/classes_3.h src/classes_def_3.xml)
cms_rootdict( DataFormatsJetReco  src/classes_4.h src/classes_def_4.xml)
add_library( DataFormatsJetReco  SHARED ${DataFormatsJetReco_EXTRA_SOURCES}
  src/BasicJet.cc
  src/CaloJet.cc
  src/DiscretizedEnergyFlow.cc
  src/FFTJetProducerSummary.cc
  src/GenJet.cc
  src/GenericJet.cc
  src/JPTJet.cc
  src/Jet.cc
  src/JetExtendedAssociation.cc
  src/JetFloatAssociation.cc
  src/JetTracksAssociation.cc
  src/PFClusterJet.cc
  src/PFJet.cc
  src/PileupJetIdentifier.cc
  src/TrackJet.cc
)
set_target_properties( DataFormatsJetReco PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsJetReco )
target_include_directories( DataFormatsJetReco PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsJetReco PUBLIC
/usr/include
)
target_link_libraries( DataFormatsJetReco PUBLIC
   DataFormatsCaloTowers
   DataFormatsParticleFlowCandidate
   DataFormatsCandidate
   DataFormatsCommon
   DataFormatsHepMCCandidate
   DataFormatsParticleFlowReco
   DataFormatsTrackReco
   FWCoreUtilities
   DataFormatsMath
   DataFormatsRecoCandidate
   DataFormatsVertexReco
   FWCoreMessageLogger
)
add_rootdict_rules( DataFormatsJetReco )
install( TARGETS  DataFormatsJetReco DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
