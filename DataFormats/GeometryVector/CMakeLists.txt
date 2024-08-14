include_directories(${CMAKE_SOURCE_DIR}
${ROOT_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsGeometryVector  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsGeometryVector )
add_library( DataFormatsGeometryVector  SHARED ${DataFormatsGeometryVector_EXTRA_SOURCES}
  src/TrackingJacobians.cc
  src/print.cc
)
set_target_properties( DataFormatsGeometryVector PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsGeometryVector )
target_include_directories( DataFormatsGeometryVector PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsGeometryVector PUBLIC
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsGeometryVector PUBLIC
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
)
install( TARGETS  DataFormatsGeometryVector DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
