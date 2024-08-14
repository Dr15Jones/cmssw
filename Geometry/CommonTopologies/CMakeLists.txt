include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
add_library( GeometryCommonTopologies  SHARED ${GeometryCommonTopologies_EXTRA_SOURCES}
  src/BowedSurfaceDeformation.cc
  src/CSCRadialStripTopology.cc
  src/DoubleSensGeomDet.cc
  src/ES_TrackingGeometry.cc
  src/GEMStripTopology.cc
  src/GeomDet.cc
  src/GeomDetEnumerators.cc
  src/GeomDetType.cc
  src/GlobalTrackingGeometry.cc
  src/GluedGeomDet.cc
  src/MuonGeomDet.cc
  src/PixelGeomDetType.cc
  src/PixelGeomDetUnit.cc
  src/PixelTopology.cc
  src/ProxyPixelTopology.cc
  src/RadialStripTopology.cc
  src/RectangularStripTopology.cc
  src/StackGeomDet.cc
  src/StripTopology.cc
  src/SurfaceDeformationFactory.cc
  src/TkRadialStripTopology.cc
  src/Topology.cc
  src/TrackerGeomDet.cc
  src/TrapezoidalStripTopology.cc
  src/TwoBowedSurfacesDeformation.cc
)
set_target_properties( GeometryCommonTopologies PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS GeometryCommonTopologies )
target_include_directories( GeometryCommonTopologies PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( GeometryCommonTopologies PUBLIC
/usr/include
)
target_link_libraries( GeometryCommonTopologies PUBLIC
   FWCoreMessageLogger
   DataFormatsGeometryCommonDetAlgo
   DataFormatsGeometrySurface
   DataFormatsDetId
   FWCoreUtilities
)
install( TARGETS  GeometryCommonTopologies DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
