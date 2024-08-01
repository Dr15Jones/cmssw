add_library(FWCoreReflection SHARED ${FWCoreReflection_EXTRA_SOURCES}
src/BaseWithDict.cc
src/DictionaryTools.cc
src/FunctionWithDict.cc
src/IterWithDict.cc
src/MemberWithDict.cc
src/ObjectWithDict.cc
src/SetClassParsing.cc
src/TypeWithDict.cc
)

set_target_properties(FWCoreReflection PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_dependencies( ALL_LIBS FWCoreReflection)

target_include_directories(FWCoreReflection PRIVATE ${CMAKE_SOURCE_DIR})

target_include_directories(FWCoreReflection PUBLIC
${Boost_INCLUDE_DIR}
${FMT_INCLUDE_DIR}
${ROOT_INCLUDE_DIR}
${TBB_INCLUDE_DIRS}
/usr/include
)

target_link_libraries(FWCoreReflection PUBLIC
${TBB_LIBRARIES}
FWCoreUtilities
ROOT::Core
)


install( TARGETS FWCoreReflection DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
