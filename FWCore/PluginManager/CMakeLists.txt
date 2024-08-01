add_library(FWCorePluginManager SHARED ${FWCorePluginManager_EXTRA_SOURCES}
  src/CacheParser.cc
  src/PluginCapabilities.cc
  src/PluginFactoryBase.cc
  src/PluginFactoryManager.cc
  src/PluginManager.cc
  src/PresenceFactory.cc
  src/ProblemTracker.cc
  src/SharedLibrary.cc
  src/standard.cc
)

set_target_properties(FWCorePluginManager PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_dependencies( ALL_LIBS FWCorePluginManager)

target_include_directories(FWCorePluginManager PRIVATE ${CMAKE_SOURCE_DIR})

target_include_directories(FWCorePluginManager PUBLIC
${Boost_INCLUDE_DIR}
${FMT_INCLUDE_DIR}
${ROOT_INCLUDE_DIR}
${TBB_INCLUDE_DIRS}
/usr/include
)

target_link_libraries(FWCorePluginManager PUBLIC
${TBB_LIBRARIES}
FWCoreUtilities
ROOT::Core
Threads::Threads
crypt
dl
rt
stdc++fs
)


install( TARGETS FWCorePluginManager DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
