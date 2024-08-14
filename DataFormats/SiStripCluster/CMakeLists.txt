include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsSiStripCluster  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsSiStripCluster )
add_library( DataFormatsSiStripCluster  SHARED ${DataFormatsSiStripCluster_EXTRA_SOURCES}
  src/SiStripApproximateCluster.cc
  src/SiStripApproximateClusterCollection.cc
  src/SiStripCluster.cc
  src/SiStripClustersSOA.cc
)
set_target_properties( DataFormatsSiStripCluster PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsSiStripCluster )
target_include_directories( DataFormatsSiStripCluster PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsSiStripCluster PUBLIC
/usr/include
)
target_link_libraries( DataFormatsSiStripCluster PUBLIC
   DataFormatsCommon
   DataFormatsTrajectoryState
   DataFormatsSiStripDigi
   DataFormatsSiStripDetId
   FWCoreUtilities
)
install( TARGETS  DataFormatsSiStripCluster DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
