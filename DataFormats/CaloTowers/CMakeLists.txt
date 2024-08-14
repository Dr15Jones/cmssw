include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsCaloTowers  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsCaloTowers )
add_library( DataFormatsCaloTowers  SHARED ${DataFormatsCaloTowers_EXTRA_SOURCES}
  src/CaloTower.cc
  src/CaloTowerDetId.cc
)
set_target_properties( DataFormatsCaloTowers PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsCaloTowers )
target_include_directories( DataFormatsCaloTowers PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsCaloTowers PUBLIC
/usr/include
)
target_link_libraries( DataFormatsCaloTowers PUBLIC
   DataFormatsCommon
   DataFormatsDetId
   DataFormatsMath
   FWCoreUtilities
   DataFormatsCandidate
   DataFormatsHcalDetId
   DataFormatsGeometryVector
)
install( TARGETS  DataFormatsCaloTowers DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
