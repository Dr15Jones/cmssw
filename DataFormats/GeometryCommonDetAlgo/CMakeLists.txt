include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsGeometryCommonDetAlgo  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsGeometryCommonDetAlgo )
add_library( DataFormatsGeometryCommonDetAlgo  SHARED ${DataFormatsGeometryCommonDetAlgo_EXTRA_SOURCES}
  src/AlignmentPositionError.cc
  src/PerpendicularBoundPlaneBuilder.cc
  src/TransverseBoundPlaneFactory.cc
)
set_target_properties( DataFormatsGeometryCommonDetAlgo PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsGeometryCommonDetAlgo )
target_include_directories( DataFormatsGeometryCommonDetAlgo PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsGeometryCommonDetAlgo PUBLIC
/usr/include
)
target_link_libraries( DataFormatsGeometryCommonDetAlgo PUBLIC
   DataFormatsMath
   DataFormatsCLHEP
   DataFormatsGeometrySurface
   DataFormatsGeometryVector
   DataFormatsCommon
   FWCoreUtilities
)
install( TARGETS  DataFormatsGeometryCommonDetAlgo DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
