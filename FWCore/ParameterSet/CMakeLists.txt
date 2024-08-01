add_library(FWCoreParameterSet SHARED ${FWCoreParameterSet_EXTRA_SOURCES}
  src/Entry.cc
  src/ParameterSet.cc
  src/ParameterSetConverter.cc
  src/ParameterSetEntry.cc
  src/Registry.cc
  src/VParameterSetEntry.cc
  src/types.cc
)

set_target_properties(FWCoreParameterSet PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_dependencies( ALL_LIBS FWCoreParameterSet)

target_include_directories(FWCoreParameterSet PRIVATE ${CMAKE_SOURCE_DIR})

target_include_directories(FWCoreParameterSet PUBLIC
${Boost_INCLUDE_DIR}
${FMT_INCLUDE_DIR}
${ROOT_INCLUDE_DIR}
${TBB_INCLUDE_DIRS}
/usr/include
)

target_link_libraries(FWCoreParameterSet PUBLIC
${TBB_LIBRARIES}
DataFormatsProvenance
FWCoreMessageLogger
FWCoreUtilities
Threads::Threads
crypt
dl
rt
)


install( TARGETS FWCoreParameterSet DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
