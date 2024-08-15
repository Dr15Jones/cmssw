cms_rootdict( DataFormatsFTLRecHit  src/classes.h src/classes_def.xml)
add_library( DataFormatsFTLRecHit  SHARED ${DataFormatsFTLRecHit_EXTRA_SOURCES}
  src/FTLRecHit.cc
  src/FTLTrackingRecHit.cc
  src/FTLUncalibratedRecHit.cc
)
set_target_properties( DataFormatsFTLRecHit PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsFTLRecHit )
target_include_directories( DataFormatsFTLRecHit PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsFTLRecHit PUBLIC
/usr/include
)
target_link_libraries( DataFormatsFTLRecHit PUBLIC
   DataFormatsCaloRecHit
   DataFormatsCommon
   DataFormatsForwardDetId
   DataFormatsTrackingRecHit
   DataFormatsDetId
)
add_rootdict_rules( DataFormatsFTLRecHit )
install( TARGETS  DataFormatsFTLRecHit DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
