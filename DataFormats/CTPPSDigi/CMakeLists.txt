include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsCTPPSDigi  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsCTPPSDigi )
add_library( DataFormatsCTPPSDigi  SHARED ${DataFormatsCTPPSDigi_EXTRA_SOURCES}
  src/CTPPSDiamondDigi.cc
  src/CTPPSPixelDataError.cc
  src/CTPPSPixelDigi.cc
  src/CTPPSPixelDigiCollection.cc
  src/TotemTimingDigi.cc
  src/TotemTimingEventInfo.cc
  src/TotemVFATStatus.cc
)
set_target_properties( DataFormatsCTPPSDigi PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsCTPPSDigi )
target_include_directories( DataFormatsCTPPSDigi PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsCTPPSDigi PUBLIC
/usr/include
)
target_link_libraries( DataFormatsCTPPSDigi PUBLIC
   DataFormatsCommon
   FWCoreUtilities
)
install( TARGETS  DataFormatsCTPPSDigi DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
