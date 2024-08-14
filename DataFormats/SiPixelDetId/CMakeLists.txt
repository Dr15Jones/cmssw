include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsSiPixelDetId  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsSiPixelDetId )
add_library( DataFormatsSiPixelDetId  SHARED ${DataFormatsSiPixelDetId_EXTRA_SOURCES}
  src/ES_PixelFEDChannelCollectionMap.cc
  src/PXBDetId.cc
  src/PXFDetId.cc
  src/PixelBarrelNameUpgrade.cc
  src/PixelEndcapNameUpgrade.cc
  src/PixelModuleName.cc
)
set_target_properties( DataFormatsSiPixelDetId PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsSiPixelDetId )
target_include_directories( DataFormatsSiPixelDetId PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsSiPixelDetId PUBLIC
/usr/include
)
target_link_libraries( DataFormatsSiPixelDetId PUBLIC
   DataFormatsDetId
   FWCoreUtilities
   FWCoreMessageLogger
   DataFormatsCommon
)
install( TARGETS  DataFormatsSiPixelDetId DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
