cms_rootdict( DataFormatsDetId  src/classes.h src/classes_def.xml)
add_library( DataFormatsDetId  SHARED ${DataFormatsDetId_EXTRA_SOURCES}
  src/DetId.cc
)
set_target_properties( DataFormatsDetId PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsDetId )
target_include_directories( DataFormatsDetId PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsDetId PUBLIC
/usr/include
)
target_link_libraries( DataFormatsDetId PUBLIC
   DataFormatsCommon
)
add_rootdict_rules( DataFormatsDetId )
install( TARGETS  DataFormatsDetId DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
