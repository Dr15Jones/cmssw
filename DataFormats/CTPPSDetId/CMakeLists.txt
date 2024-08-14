include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsCTPPSDetId  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsCTPPSDetId )
add_library( DataFormatsCTPPSDetId  SHARED ${DataFormatsCTPPSDetId_EXTRA_SOURCES}
  src/CTPPSDetId.cc
  src/CTPPSDiamondDetId.cc
  src/CTPPSPixelDetId.cc
  src/TotemRPDetId.cc
  src/TotemT2DetId.cc
  src/TotemTimingDetId.cc
)
set_target_properties( DataFormatsCTPPSDetId PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsCTPPSDetId )
target_include_directories( DataFormatsCTPPSDetId PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsCTPPSDetId PUBLIC
/usr/include
)
target_link_libraries( DataFormatsCTPPSDetId PUBLIC
   DataFormatsDetId
   FWCoreUtilities
)
install( TARGETS  DataFormatsCTPPSDetId DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
