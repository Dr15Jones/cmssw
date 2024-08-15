cms_rootdict( DataFormatsVertexReco  src/classes.h src/classes_def.xml)
add_library( DataFormatsVertexReco  SHARED ${DataFormatsVertexReco_EXTRA_SOURCES}
  src/TrackTimeLifeInfo.cc
  src/Vertex.cc
)
set_target_properties( DataFormatsVertexReco PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsVertexReco )
target_include_directories( DataFormatsVertexReco PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsVertexReco PUBLIC
/usr/include
)
target_link_libraries( DataFormatsVertexReco PUBLIC
   DataFormatsCommon
   DataFormatsCLHEP
   DataFormatsMath
   DataFormatsTrackReco
   DataFormatsTrajectorySeed
)
add_rootdict_rules( DataFormatsVertexReco )
install( TARGETS  DataFormatsVertexReco DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
