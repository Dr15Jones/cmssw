cms_rootdict( DataFormatsSiStripDigi  src/classes.h src/classes_def.xml)
add_library( DataFormatsSiStripDigi  SHARED ${DataFormatsSiStripDigi_EXTRA_SOURCES}
  src/SiStripDigi.cc
)
set_target_properties( DataFormatsSiStripDigi PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsSiStripDigi )
target_include_directories( DataFormatsSiStripDigi PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsSiStripDigi PUBLIC
/usr/include
)
target_link_libraries( DataFormatsSiStripDigi PUBLIC
   DataFormatsCommon
)
add_rootdict_rules( DataFormatsSiStripDigi )
install( TARGETS  DataFormatsSiStripDigi DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
