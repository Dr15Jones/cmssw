cms_rootdict( DataFormatsSiStripDetId  src/classes.h src/classes_def.xml)
add_library( DataFormatsSiStripDetId  SHARED ${DataFormatsSiStripDetId_EXTRA_SOURCES}
  src/SiStripDetId.cc
)
set_target_properties( DataFormatsSiStripDetId PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsSiStripDetId )
target_include_directories( DataFormatsSiStripDetId PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsSiStripDetId PUBLIC
/usr/include
)
target_link_libraries( DataFormatsSiStripDetId PUBLIC
   DataFormatsDetId
)
add_rootdict_rules( DataFormatsSiStripDetId )
install( TARGETS  DataFormatsSiStripDetId DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
