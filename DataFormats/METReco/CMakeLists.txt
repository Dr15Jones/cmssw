cms_rootdict( DataFormatsMETReco  src/classes.h src/classes_def.xml)
add_library( DataFormatsMETReco  SHARED ${DataFormatsMETReco_EXTRA_SOURCES}
  src/BeamHaloSummary.cc
  src/CSCHaloData.cc
  src/CaloMET.cc
  src/EcalHaloData.cc
  src/GenMET.cc
  src/GlobalHaloData.cc
  src/HaloClusterCandidateECAL.cc
  src/HaloClusterCandidateHCAL.cc
  src/HcalCaloFlagTool.cc
  src/HcalHaloData.cc
  src/HcalNoiseHPD.cc
  src/HcalNoiseRBX.cc
  src/HcalNoiseSummary.cc
  src/MET.cc
  src/PFClusterMET.cc
  src/PFMET.cc
  src/PUSubMETData.cc
  src/PhiWedge.cc
  src/SigInputObj.cc
)
set_target_properties( DataFormatsMETReco PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsMETReco )
target_include_directories( DataFormatsMETReco PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsMETReco PUBLIC
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsMETReco PUBLIC
   ROOT::Physics
   ROOT::Hist
   ROOT::Matrix
   ROOT::Tree
   ROOT::Net
   ROOT::MathCore
   ROOT::Core
   ROOT::Thread
   ROOT::RIO
   ROOT::Core
   DataFormatsCommon
   DataFormatsRecoCandidate
   DataFormatsJetReco
   DataFormatsEcalRecHit
   DataFormatsHcalRecHit
   DataFormatsCaloTowers
   DataFormatsCandidate
   DataFormatsDetId
   DataFormatsEcalDetId
   DataFormatsEgammaReco
   DataFormatsGeometrySurface
   DataFormatsGeometryVector
   DataFormatsHcalDetId
   DataFormatsHcalDigi
   DataFormatsMath
   DataFormatsTrackReco
)
add_rootdict_rules( DataFormatsMETReco )
install( TARGETS  DataFormatsMETReco DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
