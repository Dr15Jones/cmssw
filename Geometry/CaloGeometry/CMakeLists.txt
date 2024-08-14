include_directories(${CMAKE_SOURCE_DIR}
${CLHEP_INCLUDE_DIR}
/usr/include
)
add_library( GeometryCaloGeometry  SHARED ${GeometryCaloGeometry_EXTRA_SOURCES}
  src/CaloCellCrossing.cc
  src/CaloCellGeometry.cc
  src/CaloGenericDetId.cc
  src/CaloGeometry.cc
  src/CaloNumberingScheme.cc
  src/CaloSubdetectorGeometry.cc
  src/ES_CaloGeometry.cc
  src/ES_CaloSubdetectorGeometry.cc
  src/EcalTrapezoidParameters.cc
  src/FlatHexagon.cc
  src/FlatTrd.cc
  src/IdealObliquePrism.cc
  src/IdealZPrism.cc
  src/PreshowerStrip.cc
  src/TruncatedPyramid.cc
)
set_target_properties( GeometryCaloGeometry PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS GeometryCaloGeometry )
target_include_directories( GeometryCaloGeometry PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( GeometryCaloGeometry PUBLIC
  ${CLHEP_INCLUDE_DIR}
/usr/include
)
target_link_libraries( GeometryCaloGeometry PUBLIC
   ${CLHEP_LIBRARIES}
   DataFormatsEcalDetId
   DataFormatsHcalDetId
   DataFormatsCaloTowers
   DataFormatsGeometryVector
   DataFormatsDetId
   DataFormatsMath
   FWCoreMessageLogger
   FWCoreUtilities
)
install( TARGETS  GeometryCaloGeometry DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
