include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsFEDRawData  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsFEDRawData )
add_library( DataFormatsFEDRawData  SHARED ${DataFormatsFEDRawData_EXTRA_SOURCES}
  src/FEDHeader.cc
  src/FEDNumbering.cc
  src/FEDRawData.cc
  src/FEDRawDataCollection.cc
  src/FEDTrailer.cc
)
set_target_properties( DataFormatsFEDRawData PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsFEDRawData )
target_include_directories( DataFormatsFEDRawData PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsFEDRawData PUBLIC
/usr/include
)
target_link_libraries( DataFormatsFEDRawData PUBLIC
   DataFormatsCommon
   FWCoreUtilities
)
install( TARGETS  DataFormatsFEDRawData DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
