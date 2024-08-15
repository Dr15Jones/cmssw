cms_rootdict( DataFormatsPatCandidates  src/classes_objects.h src/classes_def_objects.xml)
cms_rootdict( DataFormatsPatCandidates  src/classes_other.h src/classes_def_other.xml)
cms_rootdict( DataFormatsPatCandidates  src/classes_trigger.h src/classes_def_trigger.xml)
cms_rootdict( DataFormatsPatCandidates  src/classes_user.h src/classes_def_user.xml)
add_library( DataFormatsPatCandidates  SHARED ${DataFormatsPatCandidates_EXTRA_SOURCES}
  src/CandKinResolution.cc
  src/CompositeCandidate.cc
  src/Conversion.cc
  src/CovarianceParameterization.cc
  src/Electron.cc
  src/EventHypothesis.cc
  src/Flags.cc
  src/GenericParticle.cc
  src/Jet.cc
  src/JetCorrFactors.cc
  src/MET.cc
  src/MHT.cc
  src/Muon.cc
  src/PATObject.cc
  src/PATTauDiscriminator.cc
  src/PFParticle.cc
  src/PackedCandidate.cc
  src/PackedGenParticle.cc
  src/PackedTriggerPrescales.cc
  src/ParametrizationHelper.cc
  src/Particle.cc
  src/Photon.cc
  src/ResolutionHelper.cc
  src/StringMap.cc
  src/Tau.cc
  src/TauJetCorrFactors.cc
  src/TauPFEssential.cc
  src/TauPFSpecific.cc
  src/TriggerAlgorithm.cc
  src/TriggerCondition.cc
  src/TriggerEvent.cc
  src/TriggerFilter.cc
  src/TriggerObject.cc
  src/TriggerObjectStandAlone.cc
  src/TriggerPath.cc
  src/UserData.cc
  src/VIDCutFlowResult.cc
  src/Vertexing.cc
  src/throwMissingLabel.cc
)
set_target_properties( DataFormatsPatCandidates PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsPatCandidates )
target_include_directories( DataFormatsPatCandidates PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsPatCandidates PUBLIC
  ${Boost_INCLUDE_DIR}
  ${TBB_INCLUDE_DIRS}
/usr/include
)
target_link_libraries( DataFormatsPatCandidates PUBLIC
   ${TBB_LIBRARIES}
   DataFormatsBTauReco
   DataFormatsCLHEP
   DataFormatsCaloTowers
   DataFormatsCandidate
   DataFormatsCommon
   DataFormatsEcalRecHit
   DataFormatsEgammaCandidates
   DataFormatsEgammaReco
   DataFormatsGeometryCommonDetAlgo
   DataFormatsGsfTrackReco
   DataFormatsHLTReco
   DataFormatsHepMCCandidate
   DataFormatsJetMatching
   DataFormatsJetReco
   DataFormatsL1GlobalTrigger
   DataFormatsL1Trigger
   DataFormatsMETReco
   DataFormatsMath
   DataFormatsMuonReco
   DataFormatsParticleFlowCandidate
   DataFormatsProvenance
   DataFormatsRecoCandidate
   DataFormatsSiPixelDetId
   DataFormatsSiStripDetId
   DataFormatsTauReco
   DataFormatsTrackReco
   DataFormatsVertexReco
   FWCoreCommon
   FWCoreMessageLogger
   FWCoreParameterSet
   FWCoreReflection
   FWCoreUtilities
)
add_rootdict_rules( DataFormatsPatCandidates )
install( TARGETS  DataFormatsPatCandidates DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
