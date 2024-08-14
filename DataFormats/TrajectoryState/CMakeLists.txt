include_directories(${CMAKE_SOURCE_DIR}
${ROOT_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsTrajectoryState  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsTrajectoryState )
add_library( DataFormatsTrajectoryState  SHARED ${DataFormatsTrajectoryState_EXTRA_SOURCES}
)
set_target_properties( DataFormatsTrajectoryState PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsTrajectoryState )
target_include_directories( DataFormatsTrajectoryState PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsTrajectoryState PUBLIC
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsTrajectoryState PUBLIC
   ROOT::Tree
   ROOT::Net
   ROOT::MathCore
   ROOT::Core
   ROOT::Thread
   ROOT::RIO
   ROOT::Core
   DataFormatsGeometryVector
   DataFormatsMath
)
install( TARGETS  DataFormatsTrajectoryState DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
