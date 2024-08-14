include_directories(${CMAKE_SOURCE_DIR}
${Boost_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsL1GlobalTrigger  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsL1GlobalTrigger )
add_library( DataFormatsL1GlobalTrigger  SHARED ${DataFormatsL1GlobalTrigger_EXTRA_SOURCES}
  src/L1GlobalTriggerEvmReadoutRecord.cc
  src/L1GlobalTriggerObjectMap.cc
  src/L1GlobalTriggerObjectMapRecord.cc
  src/L1GlobalTriggerObjectMaps.cc
  src/L1GlobalTriggerReadoutRecord.cc
  src/L1GlobalTriggerReadoutSetup.cc
  src/L1GlobalTriggerRecord.cc
  src/L1GtFdlWord.cc
  src/L1GtLogicParser.cc
  src/L1GtObject.cc
  src/L1GtPsbWord.cc
  src/L1GtTechnicalTrigger.cc
  src/L1GtTechnicalTriggerRecord.cc
  src/L1GtTriggerMenuLite.cc
  src/L1GtfeExtWord.cc
  src/L1GtfeWord.cc
  src/L1TcsWord.cc
)
set_target_properties( DataFormatsL1GlobalTrigger PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsL1GlobalTrigger )
target_include_directories( DataFormatsL1GlobalTrigger PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsL1GlobalTrigger PUBLIC
  ${Boost_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsL1GlobalTrigger PUBLIC
   DataFormatsCommon
   DataFormatsProvenance
   DataFormatsL1GlobalMuonTrigger
   FWCoreMessageLogger
   FWCoreUtilities
)
install( TARGETS  DataFormatsL1GlobalTrigger DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
