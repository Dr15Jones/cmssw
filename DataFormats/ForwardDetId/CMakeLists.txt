cms_rootdict( DataFormatsForwardDetId  src/classes.h src/classes_def.xml)
add_library( DataFormatsForwardDetId  SHARED ${DataFormatsForwardDetId_EXTRA_SOURCES}
  src/BTLDetId.cc
  src/ETLDetId.cc
  src/HFNoseDetId.cc
  src/HFNoseDetIdToModule.cc
  src/HFNoseTriggerDetId.cc
  src/HGCEEDetId.cc
  src/HGCHEDetId.cc
  src/HGCScintillatorDetId.cc
  src/HGCSiliconDetId.cc
  src/HGCSiliconDetIdToModule.cc
  src/HGCSiliconDetIdToROC.cc
  src/HGCTriggerDetId.cc
  src/HGCalDetId.cc
  src/HGCalTriggerBackendDetId.cc
  src/HGCalTriggerDetId.cc
  src/HGCalTriggerModuleDetId.cc
  src/MTDDetId.cc
)
set_target_properties( DataFormatsForwardDetId PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsForwardDetId )
target_include_directories( DataFormatsForwardDetId PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsForwardDetId PUBLIC
/usr/include
)
target_link_libraries( DataFormatsForwardDetId PUBLIC
   FWCoreUtilities
   DataFormatsDetId
)
add_rootdict_rules( DataFormatsForwardDetId )
install( TARGETS  DataFormatsForwardDetId DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
