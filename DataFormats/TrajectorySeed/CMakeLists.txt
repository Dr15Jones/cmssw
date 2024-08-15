cms_rootdict( DataFormatsTrajectorySeed  src/classes.h src/classes_def.xml)
add_library( DataFormatsTrajectorySeed  SHARED ${DataFormatsTrajectorySeed_EXTRA_SOURCES}
)
set_target_properties( DataFormatsTrajectorySeed PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsTrajectorySeed )
target_include_directories( DataFormatsTrajectorySeed PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsTrajectorySeed PUBLIC
  ${CLHEP_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsTrajectorySeed PUBLIC
   DataFormatsCommon
   DataFormatsCLHEP
   DataFormatsTrajectoryState
   DataFormatsTrackingRecHit
   FWCoreUtilities
)
add_rootdict_rules( DataFormatsTrajectorySeed )
install( TARGETS  DataFormatsTrajectorySeed DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
