include_directories(${CMAKE_SOURCE_DIR}
${ROOT_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsGEMRecHit  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsGEMRecHit )
add_library( DataFormatsGEMRecHit  SHARED ${DataFormatsGEMRecHit_EXTRA_SOURCES}
  src/GEMCSCSegment.cc
  src/GEMRecHit.cc
  src/GEMSegment.cc
  src/ME0RecHit.cc
  src/ME0Segment.cc
)
set_target_properties( DataFormatsGEMRecHit PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsGEMRecHit )
target_include_directories( DataFormatsGEMRecHit PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsGEMRecHit PUBLIC
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsGEMRecHit PUBLIC
   ROOT::Core
   DataFormatsCommon
   DataFormatsMuonDetId
   DataFormatsTrackingRecHit
   DataFormatsCSCRecHit
   FWCoreMessageLogger
)
install( TARGETS  DataFormatsGEMRecHit DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
