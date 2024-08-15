cms_rootdict( DataFormatsTrackerCommon  src/classes.h src/classes_def.xml)
add_library( DataFormatsTrackerCommon  SHARED ${DataFormatsTrackerCommon_EXTRA_SOURCES}
  src/ClusterSummary.cc
  src/ES_TrackerTopology.cc
  src/PixelBarrelName.cc
  src/PixelEndcapName.cc
  src/SiStripSubStructure.cc
  src/TrackerTopology.cc
)
set_target_properties( DataFormatsTrackerCommon PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsTrackerCommon )
target_include_directories( DataFormatsTrackerCommon PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsTrackerCommon PUBLIC
/usr/include
)
target_link_libraries( DataFormatsTrackerCommon PUBLIC
   DataFormatsCommon
   DataFormatsSiStripCluster
   DataFormatsSiPixelDetId
   DataFormatsSiStripDetId
   DataFormatsDetId
   DataFormatsSiPixelCluster
   DataFormatsSiStripDigi
   FWCoreMessageLogger
   FWCoreUtilities
)
add_rootdict_rules( DataFormatsTrackerCommon )
install( TARGETS  DataFormatsTrackerCommon DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
