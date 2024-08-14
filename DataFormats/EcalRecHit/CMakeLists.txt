include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsEcalRecHit  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsEcalRecHit )
add_library( DataFormatsEcalRecHit  SHARED ${DataFormatsEcalRecHit_EXTRA_SOURCES}
  src/EcalRecHit.cc
  src/EcalUncalibratedRecHit.cc
)
set_target_properties( DataFormatsEcalRecHit PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsEcalRecHit )
target_include_directories( DataFormatsEcalRecHit PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsEcalRecHit PUBLIC
/usr/include
)
target_link_libraries( DataFormatsEcalRecHit PUBLIC
   DataFormatsCaloRecHit
   DataFormatsCommon
   DataFormatsEcalDetId
   DataFormatsEcalDigi
   DataFormatsDetId
   FWCoreMessageLogger
)
install( TARGETS  DataFormatsEcalRecHit DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
