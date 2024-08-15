cms_rootdict( DataFormatsTrackingRecHit  src/classes.h src/classes_def.xml)
add_library( DataFormatsTrackingRecHit  SHARED ${DataFormatsTrackingRecHit_EXTRA_SOURCES}
  src/InvalidTrackingRecHit.cc
  src/KfComponentsHolder.cc
  src/RecHit1D.cc
  src/RecHit2DLocalPos.cc
  src/TrackingRecHit.cc
)
set_target_properties( DataFormatsTrackingRecHit PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsTrackingRecHit )
target_include_directories( DataFormatsTrackingRecHit PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsTrackingRecHit PUBLIC
  ${CLHEP_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsTrackingRecHit PUBLIC
   ${CLHEP_LIBRARIES}
   DataFormatsCommon
   DataFormatsCLHEP
   DataFormatsGeometrySurface
   DataFormatsGeometryCommonDetAlgo
   FWCoreUtilities
   DataFormatsDetId
   DataFormatsGeometryVector
   DataFormatsMath
)
add_rootdict_rules( DataFormatsTrackingRecHit )
install( TARGETS  DataFormatsTrackingRecHit DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
