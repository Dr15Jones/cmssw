cms_rootdict( DataFormatsEgammaReco  src/classes.h src/classes_def.xml)
add_library( DataFormatsEgammaReco  SHARED ${DataFormatsEgammaReco_EXTRA_SOURCES}
  src/BasicCluster.cc
  src/ClusterShape.cc
  src/EgammaTrigger.cc
  src/ElectronSeed.cc
  src/HFEMClusterShape.cc
  src/PreshowerCluster.cc
  src/PreshowerClusterShape.cc
  src/SuperCluster.cc
)
set_target_properties( DataFormatsEgammaReco PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsEgammaReco )
target_include_directories( DataFormatsEgammaReco PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsEgammaReco PUBLIC
  ${CLHEP_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsEgammaReco PUBLIC
   DataFormatsCommon
   DataFormatsEcalRecHit
   DataFormatsCaloRecHit
   DataFormatsTrackingRecHit
   DataFormatsTrajectorySeed
   DataFormatsTrackReco
   DataFormatsDetId
   DataFormatsMath
   DataFormatsTrajectoryState
)
add_rootdict_rules( DataFormatsEgammaReco )
install( TARGETS  DataFormatsEgammaReco DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
