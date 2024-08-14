include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsCSCRecHit  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsCSCRecHit )
add_library( DataFormatsCSCRecHit  SHARED ${DataFormatsCSCRecHit_EXTRA_SOURCES}
  src/CSCRangeMapAccessor.cc
  src/CSCRecHit2D.cc
  src/CSCSegment.cc
)
set_target_properties( DataFormatsCSCRecHit PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsCSCRecHit )
target_include_directories( DataFormatsCSCRecHit PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsCSCRecHit PUBLIC
/usr/include
)
target_link_libraries( DataFormatsCSCRecHit PUBLIC
   DataFormatsCommon
   DataFormatsMuonDetId
   DataFormatsTrackingRecHit
)
install( TARGETS  DataFormatsCSCRecHit DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
