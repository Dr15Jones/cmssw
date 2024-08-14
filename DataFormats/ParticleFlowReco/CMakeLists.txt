include_directories(${CMAKE_SOURCE_DIR}
${ROOT_INCLUDE_DIR}
${ROOT_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsParticleFlowReco  src/classes_1.h src/classes_def_1.xml)
cms_rootdict( DataFormatsParticleFlowReco  src/classes_2.h src/classes_def_2.xml)
add_rootdict_rules( DataFormatsParticleFlowReco )
add_library( DataFormatsParticleFlowReco  SHARED ${DataFormatsParticleFlowReco_EXTRA_SOURCES}
  src/GsfPFRecTrack.cc
  src/HGCalMultiCluster.cc
  src/PFBlock.cc
  src/PFBlockElement.cc
  src/PFBlockElementBrem.cc
  src/PFBlockElementCluster.cc
  src/PFBlockElementGsfTrack.cc
  src/PFBlockElementSuperCluster.cc
  src/PFBlockElementTrack.cc
  src/PFCluster.cc
  src/PFConversion.cc
  src/PFDisplacedVertex.cc
  src/PFDisplacedVertexCandidate.cc
  src/PFDisplacedVertexSeed.cc
  src/PFLayer.cc
  src/PFRecHit.cc
  src/PFRecHitFraction.cc
  src/PFRecTrack.cc
  src/PFSimParticle.cc
  src/PFSuperCluster.cc
  src/PFTrack.cc
  src/PFTrajectoryPoint.cc
  src/PreId.cc
  src/RecoPFClusterRefCandidate.cc
)
set_target_properties( DataFormatsParticleFlowReco PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsParticleFlowReco )
target_include_directories( DataFormatsParticleFlowReco PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsParticleFlowReco PUBLIC
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsParticleFlowReco PUBLIC
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
   DataFormatsCaloRecHit
   DataFormatsCandidate
   DataFormatsCommon
   DataFormatsDetId
   DataFormatsEgammaCandidates
   DataFormatsEgammaReco
   DataFormatsGeometryVector
   DataFormatsGsfTrackReco
   DataFormatsMath
   DataFormatsMuonReco
   DataFormatsTrackReco
   DataFormatsTrackingRecHit
   DataFormatsTrajectorySeed
   DataFormatsVertexReco
   FWCoreMessageLogger
   FWCoreUtilities
   GeometryCaloGeometry
)
install( TARGETS  DataFormatsParticleFlowReco DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
