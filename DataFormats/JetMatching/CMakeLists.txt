cms_rootdict( DataFormatsJetMatching  src/classes.h src/classes_def.xml)
add_library( DataFormatsJetMatching  SHARED ${DataFormatsJetMatching_EXTRA_SOURCES}
)
set_target_properties( DataFormatsJetMatching PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsJetMatching )
target_include_directories( DataFormatsJetMatching PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsJetMatching PUBLIC
/usr/include
)
target_link_libraries( DataFormatsJetMatching PUBLIC
   DataFormatsCommon
   DataFormatsHepMCCandidate
   DataFormatsJetReco
   DataFormatsMath
)
add_rootdict_rules( DataFormatsJetMatching )
install( TARGETS  DataFormatsJetMatching DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
