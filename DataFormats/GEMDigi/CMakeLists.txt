include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsGEMDigi  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsGEMDigi )
add_library( DataFormatsGEMDigi  SHARED ${DataFormatsGEMDigi_EXTRA_SOURCES}
  src/GEMAMC.cc
  src/GEMAMC13.cc
  src/GEMCoPadDigi.cc
  src/GEMDigi.cc
  src/GEMPadDigi.cc
  src/GEMPadDigiCluster.cc
  src/GEMVFAT.cc
  src/ME0Digi.cc
  src/ME0DigiPreReco.cc
  src/ME0PadDigi.cc
  src/ME0PadDigiCluster.cc
  src/ME0TriggerDigi.cc
)
set_target_properties( DataFormatsGEMDigi PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsGEMDigi )
target_include_directories( DataFormatsGEMDigi PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsGEMDigi PUBLIC
/usr/include
)
target_link_libraries( DataFormatsGEMDigi PUBLIC
   DataFormatsCommon
   DataFormatsMuonDetId
   DataFormatsGEMRecHit
   FWCoreMessageLogger
)
install( TARGETS  DataFormatsGEMDigi DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
