cms_rootdict( DataFormatsTrackerRecHit2D  src/classes.h src/classes_def.xml)
add_library( DataFormatsTrackerRecHit2D  SHARED ${DataFormatsTrackerRecHit2D_EXTRA_SOURCES}
  src/BaseTrackerRecHit.cc
  src/ClusterRemovalInfo.cc
  src/FastMatchedTrackerRecHit.cc
  src/FastProjectedTrackerRecHit.cc
  src/FastSingleTrackerRecHit.cc
  src/FastTrackerRecHit.cc
  src/MTDTrackingRecHit.cc
  src/ProjectedSiStripRecHit2D.cc
  src/SiPixelRecHit.cc
  src/SiPixelRecHitQuality.cc
  src/SiStripMatchedRecHit2D.cc
  src/SiStripRecHit1D.cc
  src/SiStripRecHit2D.cc
  src/SiTrackerMultiRecHit.cc
  src/TrackerSingleRecHit.cc
  src/TrackingRecHitLessFromGlobalPosition.cc
  src/VectorHit.cc
)
set_target_properties( DataFormatsTrackerRecHit2D PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsTrackerRecHit2D )
target_include_directories( DataFormatsTrackerRecHit2D PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsTrackerRecHit2D PUBLIC
/usr/include
)
target_link_libraries( DataFormatsTrackerRecHit2D PUBLIC
   DataFormatsCommon
   DataFormatsCLHEP
   DataFormatsFTLRecHit
   DataFormatsTrackingRecHit
   DataFormatsTrajectorySeed
   DataFormatsPhase2TrackerCluster
   FWCoreMessageLogger
   DataFormatsDetId
   DataFormatsGeometryCommonDetAlgo
   DataFormatsGeometrySurface
   DataFormatsGeometryVector
   DataFormatsMath
   DataFormatsSiPixelCluster
   DataFormatsSiPixelDetId
   DataFormatsSiStripCluster
   DataFormatsSiStripDetId
   FWCoreUtilities
)
add_rootdict_rules( DataFormatsTrackerRecHit2D )
install( TARGETS  DataFormatsTrackerRecHit2D DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
