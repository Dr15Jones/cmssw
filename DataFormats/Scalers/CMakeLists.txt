include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsScalers  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsScalers )
add_library( DataFormatsScalers  SHARED ${DataFormatsScalers_EXTRA_SOURCES}
  src/BeamSpotOnline.cc
  src/DcsStatus.cc
  src/L1AcceptBunchCrossing.cc
  src/L1TriggerRates.cc
  src/L1TriggerScalers.cc
  src/Level1TriggerRates.cc
  src/Level1TriggerScalers.cc
  src/LumiScalers.cc
  src/TimeSpec.cc
)
set_target_properties( DataFormatsScalers PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsScalers )
target_include_directories( DataFormatsScalers PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsScalers PUBLIC
/usr/include
)
target_link_libraries( DataFormatsScalers PUBLIC
   DataFormatsCommon
)
install( TARGETS  DataFormatsScalers DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
