cms_rootdict( DataFormatsSiPixelCluster  src/classes.h src/classes_def.xml)
add_library( DataFormatsSiPixelCluster  SHARED ${DataFormatsSiPixelCluster_EXTRA_SOURCES}
  src/SiPixelCluster.cc
  src/SiPixelClusterShapeCache.cc
)
set_target_properties( DataFormatsSiPixelCluster PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsSiPixelCluster )
target_include_directories( DataFormatsSiPixelCluster PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsSiPixelCluster PUBLIC
/usr/include
)
target_link_libraries( DataFormatsSiPixelCluster PUBLIC
   DataFormatsCommon
   DataFormatsProvenance
   FWCoreUtilities
)
add_rootdict_rules( DataFormatsSiPixelCluster )
install( TARGETS  DataFormatsSiPixelCluster DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
