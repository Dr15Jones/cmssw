include_directories(${CMAKE_SOURCE_DIR}
${ROOT_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsMuonReco  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsMuonReco )
add_library( DataFormatsMuonReco  SHARED ${DataFormatsMuonReco_EXTRA_SOURCES}
  src/CaloMuon.cc
  src/DYTInfo.cc
  src/Muon.cc
  src/MuonChamberMatch.cc
  src/MuonCocktails.cc
  src/MuonRecHitCluster.cc
  src/MuonSelectors.cc
  src/MuonSimInfo.cc
  src/MuonTimeExtra.cc
)
set_target_properties( DataFormatsMuonReco PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsMuonReco )
target_include_directories( DataFormatsMuonReco PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsMuonReco PUBLIC
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsMuonReco PUBLIC
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
   DataFormatsRecoCandidate
   DataFormatsTrackReco
   DataFormatsDTRecHit
   DataFormatsCSCRecHit
   DataFormatsGEMRecHit
   DataFormatsVertexReco
   DataFormatsDetId
   DataFormatsHcalDetId
   DataFormatsMath
   DataFormatsMuonDetId
)
install( TARGETS  DataFormatsMuonReco DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
