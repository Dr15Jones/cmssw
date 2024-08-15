cms_rootdict( DataFormatsDTDigi  src/classes.h src/classes_def.xml)
add_library( DataFormatsDTDigi  SHARED ${DataFormatsDTDigi_EXTRA_SOURCES}
  src/DTDigi.cc
  src/DTLocalTrigger.cc
)
set_target_properties( DataFormatsDTDigi PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsDTDigi )
target_include_directories( DataFormatsDTDigi PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsDTDigi PUBLIC
/usr/include
)
target_link_libraries( DataFormatsDTDigi PUBLIC
   DataFormatsCommon
   DataFormatsMuonDetId
   DataFormatsFEDRawData
   FWCoreUtilities
)
add_rootdict_rules( DataFormatsDTDigi )
install( TARGETS  DataFormatsDTDigi DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
