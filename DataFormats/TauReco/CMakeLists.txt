include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsTauReco  src/classes_1.h src/classes_def_1.xml)
cms_rootdict( DataFormatsTauReco  src/classes_2.h src/classes_def_2.xml)
cms_rootdict( DataFormatsTauReco  src/classes_3.h src/classes_def_3.xml)
cms_rootdict( DataFormatsTauReco  src/classes_hlt.h src/classes_def_hlt.xml)
add_rootdict_rules( DataFormatsTauReco )
add_library( DataFormatsTauReco  SHARED ${DataFormatsTauReco_EXTRA_SOURCES}
  src/BaseTau.cc
  src/BaseTauTagInfo.cc
  src/PFRecoTauChargedHadron.cc
  src/PFTau.cc
  src/PFTau3ProngSummary.cc
  src/PFTauDecayMode.cc
  src/PFTauTagInfo.cc
  src/PFTauTransverseImpactParameter.cc
  src/RecoTauPiZero.cc
)
set_target_properties( DataFormatsTauReco PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsTauReco )
target_include_directories( DataFormatsTauReco PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsTauReco PUBLIC
/usr/include
)
target_link_libraries( DataFormatsTauReco PUBLIC
   DataFormatsCommon
   DataFormatsRecoCandidate
   DataFormatsCandidate
   DataFormatsMath
   DataFormatsTrackReco
   DataFormatsJetReco
   DataFormatsParticleFlowCandidate
   DataFormatsVertexReco
   FWCoreMessageLogger
)
install( TARGETS  DataFormatsTauReco DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
