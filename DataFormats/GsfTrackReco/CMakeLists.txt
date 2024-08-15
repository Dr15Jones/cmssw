cms_rootdict( DataFormatsGsfTrackReco  src/classes.h src/classes_def.xml)
add_library( DataFormatsGsfTrackReco  SHARED ${DataFormatsGsfTrackReco_EXTRA_SOURCES}
  src/GsfComponent5D.cc
  src/GsfTrack.cc
  src/GsfTrackExtra.cc
)
set_target_properties( DataFormatsGsfTrackReco PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsGsfTrackReco )
target_include_directories( DataFormatsGsfTrackReco PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsGsfTrackReco PUBLIC
/usr/include
)
target_link_libraries( DataFormatsGsfTrackReco PUBLIC
   DataFormatsCommon
   DataFormatsTrackReco
   DataFormatsGeometryCommonDetAlgo
   DataFormatsMath
   FWCoreUtilities
)
add_rootdict_rules( DataFormatsGsfTrackReco )
install( TARGETS  DataFormatsGsfTrackReco DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
