include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsWrappedStdDictionaries  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsWrappedStdDictionaries )
add_library( DataFormatsWrappedStdDictionaries  SHARED ${DataFormatsWrappedStdDictionaries_EXTRA_SOURCES}
)
set_target_properties( DataFormatsWrappedStdDictionaries PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsWrappedStdDictionaries )
target_include_directories( DataFormatsWrappedStdDictionaries PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsWrappedStdDictionaries PUBLIC
/usr/include
)
target_link_libraries( DataFormatsWrappedStdDictionaries PUBLIC
   DataFormatsCommon
)
install( TARGETS  DataFormatsWrappedStdDictionaries DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
