cms_rootdict( DataFormatsMath  src/classes.h src/classes_def.xml)
add_library( DataFormatsMath  SHARED ${DataFormatsMath_EXTRA_SOURCES}
  src/ExtVec.cc
  src/FastMath.cc
  src/SSEVec.cc
  src/libminifloat.cc
)
set_target_properties( DataFormatsMath PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsMath )
target_include_directories( DataFormatsMath PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsMath PUBLIC
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsMath PUBLIC
   ROOT::GenVector
   ROOT::MathMore
   ROOT::Tree
   ROOT::Net
   ROOT::MathCore
   ROOT::Core
   ROOT::Thread
   ROOT::RIO
   ROOT::Core
   DataFormatsCommon
   FWCoreUtilities
)
add_rootdict_rules( DataFormatsMath )
install( TARGETS  DataFormatsMath DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
