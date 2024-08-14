include_directories(${CMAKE_SOURCE_DIR}
${ROOT_INCLUDE_DIR}
${ROOT_INCLUDE_DIR}
${CLHEP_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsParticleFlowCandidate  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsParticleFlowCandidate )
add_library( DataFormatsParticleFlowCandidate  SHARED ${DataFormatsParticleFlowCandidate_EXTRA_SOURCES}
  src/IsolatedPFCandidate.cc
  src/PFCandidate.cc
  src/PFCandidateEGammaExtra.cc
  src/PFCandidateElectronExtra.cc
  src/PFCandidatePhotonExtra.cc
  src/PileUpPFCandidate.cc
)
set_target_properties( DataFormatsParticleFlowCandidate PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsParticleFlowCandidate )
target_include_directories( DataFormatsParticleFlowCandidate PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsParticleFlowCandidate PUBLIC
  ${CLHEP_INCLUDE_DIR}
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsParticleFlowCandidate PUBLIC
   ROOT::Tree
   ROOT::Net
   ROOT::MathCore
   ROOT::Core
   ROOT::Thread
   ROOT::RIO
   ROOT::Core
   ROOT::GenVector
   ROOT::MathMore
   ROOT::Tree
   ROOT::Net
   ROOT::MathCore
   ROOT::Core
   ROOT::Thread
   ROOT::RIO
   ROOT::Core
   DataFormatsParticleFlowReco
   DataFormatsEgammaCandidates
   DataFormatsCandidate
   DataFormatsCommon
   DataFormatsMath
   DataFormatsTrackReco
   DataFormatsGsfTrackReco
   DataFormatsMuonReco
   FWCoreUtilities
   DataFormatsEgammaReco
   DataFormatsVertexReco
)
install( TARGETS  DataFormatsParticleFlowCandidate DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
