include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsL1CaloTrigger  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsL1CaloTrigger )
add_library( DataFormatsL1CaloTrigger  SHARED ${DataFormatsL1CaloTrigger_EXTRA_SOURCES}
  src/L1CaloEmCand.cc
  src/L1CaloMipQuietRegion.cc
  src/L1CaloRegion.cc
  src/L1CaloRegionDetId.cc
)
set_target_properties( DataFormatsL1CaloTrigger PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsL1CaloTrigger )
target_include_directories( DataFormatsL1CaloTrigger PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsL1CaloTrigger PUBLIC
/usr/include
)
target_link_libraries( DataFormatsL1CaloTrigger PUBLIC
   DataFormatsCommon
   DataFormatsDetId
)
install( TARGETS  DataFormatsL1CaloTrigger DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
