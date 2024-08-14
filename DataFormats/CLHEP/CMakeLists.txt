include_directories(${CMAKE_SOURCE_DIR}
${CLHEP_INCLUDE_DIR}
${ROOT_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsCLHEP  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsCLHEP )
add_library( DataFormatsCLHEP  SHARED ${DataFormatsCLHEP_EXTRA_SOURCES}
)
set_target_properties( DataFormatsCLHEP PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsCLHEP )
target_include_directories( DataFormatsCLHEP PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsCLHEP PUBLIC
  ${CLHEP_INCLUDE_DIR}
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsCLHEP PUBLIC
   ${CLHEP_LIBRARIES}
   ROOT::GenVector
   ROOT::MathMore
   ROOT::Tree
   ROOT::Net
   ROOT::MathCore
   ROOT::Core
   ROOT::Thread
   ROOT::RIO
   ROOT::Core
   DataFormatsMath
   DataFormatsMath
)
install( TARGETS  DataFormatsCLHEP DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
