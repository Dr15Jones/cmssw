cms_rootdict( DataFormatsDTRecHit  src/classes.h src/classes_def.xml)
add_library( DataFormatsDTRecHit  SHARED ${DataFormatsDTRecHit_EXTRA_SOURCES}
  src/DTChamberRecSegment2D.cc
  src/DTEnums.cc
  src/DTRangeMapAccessor.cc
  src/DTRecHit1D.cc
  src/DTRecHit1DPair.cc
  src/DTRecSegment2D.cc
  src/DTRecSegment4D.cc
  src/DTSLRecCluster.cc
  src/DTSLRecSegment2D.cc
)
set_target_properties( DataFormatsDTRecHit PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsDTRecHit )
target_include_directories( DataFormatsDTRecHit PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsDTRecHit PUBLIC
/usr/include
)
target_link_libraries( DataFormatsDTRecHit PUBLIC
   DataFormatsCommon
   DataFormatsDTDigi
   DataFormatsGeometrySurface
   DataFormatsGeometryVector
   DataFormatsMuonDetId
   DataFormatsTrackingRecHit
   FWCoreUtilities
)
add_rootdict_rules( DataFormatsDTRecHit )
install( TARGETS  DataFormatsDTRecHit DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
