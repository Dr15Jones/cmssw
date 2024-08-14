include_directories(${CMAKE_SOURCE_DIR}
${CLHEP_INCLUDE_DIR}
${ROOT_INCLUDE_DIR}
${ROOT_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsBeamSpot  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsBeamSpot )
add_library( DataFormatsBeamSpot  SHARED ${DataFormatsBeamSpot_EXTRA_SOURCES}
  src/BeamSpot.cc
)
set_target_properties( DataFormatsBeamSpot PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsBeamSpot )
target_include_directories( DataFormatsBeamSpot PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsBeamSpot PUBLIC
  ${CLHEP_INCLUDE_DIR}
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsBeamSpot PUBLIC
   ${CLHEP_LIBRARIES}
   ROOT::Tree
   ROOT::Net
   ROOT::MathCore
   ROOT::Core
   ROOT::Thread
   ROOT::RIO
   ROOT::Core
   ROOT::Smatrix
   ROOT::Core
   DataFormatsCommon
   DataFormatsCLHEP
   DataFormatsGeometrySurface
   DataFormatsMath
)
install( TARGETS  DataFormatsBeamSpot DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
