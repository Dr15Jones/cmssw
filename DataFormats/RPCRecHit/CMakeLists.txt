cms_rootdict( DataFormatsRPCRecHit  src/classes.h src/classes_def.xml)
add_library( DataFormatsRPCRecHit  SHARED ${DataFormatsRPCRecHit_EXTRA_SOURCES}
  src/RPCRecHit.cc
)
set_target_properties( DataFormatsRPCRecHit PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsRPCRecHit )
target_include_directories( DataFormatsRPCRecHit PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsRPCRecHit PUBLIC
/usr/include
)
target_link_libraries( DataFormatsRPCRecHit PUBLIC
   DataFormatsCommon
   DataFormatsMuonDetId
   DataFormatsTrackingRecHit
)
add_rootdict_rules( DataFormatsRPCRecHit )
install( TARGETS  DataFormatsRPCRecHit DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
