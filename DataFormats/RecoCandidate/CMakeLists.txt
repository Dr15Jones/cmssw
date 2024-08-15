cms_rootdict( DataFormatsRecoCandidate  src/classes.h src/classes_def.xml)
add_library( DataFormatsRecoCandidate  SHARED ${DataFormatsRecoCandidate_EXTRA_SOURCES}
  src/CaloRecHitCandidate.cc
  src/IsoDeposit.cc
  src/IsoDepositVetos.cc
  src/RecoCaloTowerCandidate.cc
  src/RecoCandidate.cc
  src/RecoChargedCandidate.cc
  src/RecoChargedRefCandidate.cc
  src/RecoEcalCandidate.cc
  src/RecoStandAloneMuonCandidate.cc
)
set_target_properties( DataFormatsRecoCandidate PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsRecoCandidate )
target_include_directories( DataFormatsRecoCandidate PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsRecoCandidate PUBLIC
  ${CLHEP_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsRecoCandidate PUBLIC
   ${CLHEP_LIBRARIES}
   DataFormatsCaloRecHit
   DataFormatsCandidate
   DataFormatsCommon
   DataFormatsTrackReco
   DataFormatsCaloTowers
   DataFormatsEgammaReco
   DataFormatsGsfTrackReco
   DataFormatsMath
   FWCoreMessageLogger
   FWCoreUtilities
)
add_rootdict_rules( DataFormatsRecoCandidate )
install( TARGETS  DataFormatsRecoCandidate DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
