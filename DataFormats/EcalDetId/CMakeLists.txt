cms_rootdict( DataFormatsEcalDetId  src/classes.h src/classes_def.xml)
add_library( DataFormatsEcalDetId  SHARED ${DataFormatsEcalDetId_EXTRA_SOURCES}
  src/EBDetId.cc
  src/EEDetId.cc
  src/ESDetId.cc
  src/EcalElectronicsId.cc
  src/EcalPnDiodeDetId.cc
  src/EcalScDetId.cc
  src/EcalTrigTowerDetId.cc
  src/EcalTriggerElectronicsId.cc
)
set_target_properties( DataFormatsEcalDetId PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsEcalDetId )
target_include_directories( DataFormatsEcalDetId PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsEcalDetId PUBLIC
/usr/include
)
target_link_libraries( DataFormatsEcalDetId PUBLIC
   DataFormatsCommon
   DataFormatsDetId
   FWCoreUtilities
)
add_rootdict_rules( DataFormatsEcalDetId )
install( TARGETS  DataFormatsEcalDetId DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
