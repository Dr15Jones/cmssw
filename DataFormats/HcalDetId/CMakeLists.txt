include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsHcalDetId  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsHcalDetId )
add_library( DataFormatsHcalDetId  SHARED ${DataFormatsHcalDetId_EXTRA_SOURCES}
  src/CastorElectronicsId.cc
  src/HcalCalibDetId.cc
  src/HcalCastorDetId.cc
  src/HcalDcsDetId.cc
  src/HcalDetId.cc
  src/HcalElectronicsId.cc
  src/HcalFrontEndId.cc
  src/HcalGenericDetId.cc
  src/HcalOtherDetId.cc
  src/HcalTestNumbering.cc
  src/HcalTrigTowerDetId.cc
  src/HcalZDCDetId.cc
)
set_target_properties( DataFormatsHcalDetId PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsHcalDetId )
target_include_directories( DataFormatsHcalDetId PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsHcalDetId PUBLIC
/usr/include
)
target_link_libraries( DataFormatsHcalDetId PUBLIC
   DataFormatsDetId
   FWCoreUtilities
   FWCoreMessageLogger
)
install( TARGETS  DataFormatsHcalDetId DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
