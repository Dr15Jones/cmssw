cms_rootdict( DataFormatsCSCDigi  src/classes.h src/classes_def.xml)
add_library( DataFormatsCSCDigi  SHARED ${DataFormatsCSCDigi_EXTRA_SOURCES}
  src/CSCALCTDigi.cc
  src/CSCALCTPreTriggerDigi.cc
  src/CSCALCTStatusDigi.cc
  src/CSCCFEBStatusDigi.cc
  src/CSCCLCTDigi.cc
  src/CSCCLCTPreTriggerDigi.cc
  src/CSCComparatorDigi.cc
  src/CSCCorrelatedLCTDigi.cc
  src/CSCDCCFormatStatusDigi.cc
  src/CSCDCCStatusDigi.cc
  src/CSCDDUStatusDigi.cc
  src/CSCDMBStatusDigi.cc
  src/CSCRPCDigi.cc
  src/CSCShowerDigi.cc
  src/CSCStripDigi.cc
  src/CSCTMBStatusDigi.cc
  src/CSCWireDigi.cc
)
set_target_properties( DataFormatsCSCDigi PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsCSCDigi )
target_include_directories( DataFormatsCSCDigi PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsCSCDigi PUBLIC
/usr/include
)
target_link_libraries( DataFormatsCSCDigi PUBLIC
   DataFormatsCommon
   DataFormatsMuonDetId
   DataFormatsGEMDigi
   FWCoreMessageLogger
)
add_rootdict_rules( DataFormatsCSCDigi )
install( TARGETS  DataFormatsCSCDigi DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
