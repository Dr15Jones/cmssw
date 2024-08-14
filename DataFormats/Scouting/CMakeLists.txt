include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsScouting  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsScouting )
add_library( DataFormatsScouting  SHARED ${DataFormatsScouting_EXTRA_SOURCES}
)
set_target_properties( DataFormatsScouting PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsScouting )
target_include_directories( DataFormatsScouting PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsScouting PUBLIC
/usr/include
)
target_link_libraries( DataFormatsScouting PUBLIC
   DataFormatsCommon
)
install( TARGETS  DataFormatsScouting DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
