cms_rootdict( DataFormatsBTauReco  src/classes.h src/classes_def.xml)
add_library( DataFormatsBTauReco  SHARED ${DataFormatsBTauReco_EXTRA_SOURCES}
  src/IsolatedTauTagInfo.cc
  src/TaggingVariable.cc
  src/TauImpactParameterInfo.cc
  src/TauMassTagInfo.cc
  src/TemplatedSoftLeptonTagInfo.cc
  src/VertexTypes.cc
)
set_target_properties( DataFormatsBTauReco PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsBTauReco )
target_include_directories( DataFormatsBTauReco PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsBTauReco PUBLIC
  ${ROOT_INCLUDE_DIR}
  ${CLHEP_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsBTauReco PUBLIC
   ${CLHEP_LIBRARIES}
   ROOT::GenVector
   ROOT::MathMore
   ROOT::Tree
   ROOT::Net
   ROOT::MathCore
   ROOT::Core
   ROOT::Thread
   ROOT::RIO
   ROOT::Core
   DataFormatsCommon
   DataFormatsJetReco
   DataFormatsMath
   DataFormatsParticleFlowCandidate
   DataFormatsTrackReco
   DataFormatsVertexReco
   FWCoreUtilities
   DataFormatsCandidate
   DataFormatsEgammaReco
   DataFormatsGeometryCommonDetAlgo
   DataFormatsGeometryVector
)
add_rootdict_rules( DataFormatsBTauReco )
install( TARGETS  DataFormatsBTauReco DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
