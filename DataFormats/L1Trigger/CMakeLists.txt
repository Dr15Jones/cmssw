include_directories(${CMAKE_SOURCE_DIR}
${CMAKE_SOURCE_DIR}/hls
/usr/include
)
cms_rootdict( DataFormatsL1Trigger  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsL1Trigger )
add_library( DataFormatsL1Trigger  SHARED ${DataFormatsL1Trigger_EXTRA_SOURCES}
  src/CaloSpare.cc
  src/EGamma.cc
  src/EtSum.cc
  src/EtSumHelper.cc
  src/HOTPDigiTwinMux.cc
  src/Jet.cc
  src/L1Candidate.cc
  src/L1DataEmulDigi.cc
  src/L1DataEmulRecord.cc
  src/L1DataEmulResult.cc
  src/L1EmParticle.cc
  src/L1EtMissParticle.cc
  src/L1HFRings.cc
  src/L1JetParticle.cc
  src/L1MonitorDigi.cc
  src/L1MuonParticle.cc
  src/L1ParticleMap.cc
  src/L1TriggerError.cc
  src/Muon.cc
  src/MuonShower.cc
  src/P2GTCandidate.cc
  src/Tau.cc
  src/TkJetWord.cc
  src/TkTripletWord.cc
  src/VertexWord.cc
)
set_target_properties( DataFormatsL1Trigger PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsL1Trigger )
target_include_directories( DataFormatsL1Trigger PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsL1Trigger PUBLIC
  ${CMAKE_SOURCE_DIR}/hls
/usr/include
)
target_link_libraries( DataFormatsL1Trigger PUBLIC
   DataFormatsCandidate
   DataFormatsCommon
   DataFormatsL1GlobalCaloTrigger
   DataFormatsL1GlobalMuonTrigger
   DataFormatsL1TrackTrigger
   DataFormatsHcalDetId
   FWCoreConcurrency
   FWCoreMessageLogger
)
install( TARGETS  DataFormatsL1Trigger DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
