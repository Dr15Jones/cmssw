include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsMuonDetId  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsMuonDetId )
add_library( DataFormatsMuonDetId  SHARED ${DataFormatsMuonDetId_EXTRA_SOURCES}
  src/CSCDetId.cc
  src/CSCIndexer.cc
  src/CSCTriggerNumbering.cc
  src/DTChamberId.cc
  src/DTLayerId.cc
  src/DTSectCollId.cc
  src/DTSuperLayerId.cc
  src/DTWireId.cc
  src/GEMDetId.cc
  src/ME0DetId.cc
  src/RPCCompDetId.cc
  src/RPCDetId.cc
)
set_target_properties( DataFormatsMuonDetId PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsMuonDetId )
target_include_directories( DataFormatsMuonDetId PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsMuonDetId PUBLIC
/usr/include
)
target_link_libraries( DataFormatsMuonDetId PUBLIC
   DataFormatsDetId
   FWCoreUtilities
)
install( TARGETS  DataFormatsMuonDetId DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
