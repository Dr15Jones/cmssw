include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsHcalRecHit  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsHcalRecHit )
add_library( DataFormatsHcalRecHit  SHARED ${DataFormatsHcalRecHit_EXTRA_SOURCES}
  src/CastorRecHit.cc
  src/HBHEChannelInfo.cc
  src/HBHERecHit.cc
  src/HFPreRecHit.cc
  src/HFQIE10Info.cc
  src/HFRecHit.cc
  src/HORecHit.cc
  src/HcalCalibRecHit.cc
  src/HcalSourcePositionData.cc
  src/ZDCRecHit.cc
)
set_target_properties( DataFormatsHcalRecHit PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsHcalRecHit )
target_include_directories( DataFormatsHcalRecHit PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsHcalRecHit PUBLIC
/usr/include
)
target_link_libraries( DataFormatsHcalRecHit PUBLIC
   DataFormatsCaloRecHit
   DataFormatsCommon
   DataFormatsHcalDetId
   DataFormatsHcalDigi
)
install( TARGETS  DataFormatsHcalRecHit DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
