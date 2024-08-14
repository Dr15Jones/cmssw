include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsProtonReco  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsProtonReco )
add_library( DataFormatsProtonReco  SHARED ${DataFormatsProtonReco_EXTRA_SOURCES}
  src/ForwardProton.cc
)
set_target_properties( DataFormatsProtonReco PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsProtonReco )
target_include_directories( DataFormatsProtonReco PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsProtonReco PUBLIC
/usr/include
)
target_link_libraries( DataFormatsProtonReco PUBLIC
   DataFormatsCommon
   DataFormatsCTPPSReco
   DataFormatsMath
)
install( TARGETS  DataFormatsProtonReco DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
