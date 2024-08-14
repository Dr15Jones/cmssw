include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsEgammaCandidates  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsEgammaCandidates )
add_library( DataFormatsEgammaCandidates  SHARED ${DataFormatsEgammaCandidates_EXTRA_SOURCES}
  src/Conversion.cc
  src/Electron.cc
  src/GsfElectron.cc
  src/GsfElectronCore.cc
  src/Photon.cc
  src/SiStripElectron.cc
)
set_target_properties( DataFormatsEgammaCandidates PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsEgammaCandidates )
target_include_directories( DataFormatsEgammaCandidates PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsEgammaCandidates PUBLIC
/usr/include
)
target_link_libraries( DataFormatsEgammaCandidates PUBLIC
   DataFormatsCandidate
   DataFormatsCLHEP
   DataFormatsCommon
   DataFormatsEgammaReco
   DataFormatsCaloRecHit
   DataFormatsCaloTowers
   DataFormatsMath
   DataFormatsRecoCandidate
   DataFormatsTrackReco
   DataFormatsTrackerRecHit2D
   DataFormatsTrackingRecHit
   DataFormatsVertexReco
   DataFormatsGsfTrackReco
   DataFormatsGeometryCommonDetAlgo
   FWCoreMessageLogger
   DataFormatsGeometryVector
)
install( TARGETS  DataFormatsEgammaCandidates DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
