add_library(FWCoreUtilities SHARED ${FWCoreUtilities_EXTRA_SOURCES}
src/Adler32Calculator.cc
src/ChildrenCPUTimer.cc
src/ConvertException.cc
src/CPUServiceBase.cc
src/CPUTimer.cc
src/CRC32Calculator.cc
src/DebugMacros.cc
src/Digest.cc
src/EDMException.cc
src/ESInputTag.cc
src/Exception.cc
src/ExceptionCollector.cc
src/ExceptionPropagate.cc
src/FileInPath.cc
src/FriendlyName.cc
src/GlobalIdentifier.cc
src/Guid.cc
src/InputTag.cc
src/LuminosityBlockIndex.cc
src/OStreamColumn.cc
src/Parse.cc
src/path_configuration.cc
src/Presence.cc
src/ProcessBlockIndex.cc
src/processGUID.cc
src/RandomNumberGenerator.cc
src/RegexMatch.cc
src/ReleaseVersion.cc
src/resolveSymbolicLinks.cc
src/ResourceInformation.cc
src/RunIndex.cc
src/SingleConsumerQ.cc
src/stemFromPath.cc
src/TimeOfDay.cc
src/TimingServiceBase.cc
src/TypeDemangler.cc
src/TypeID.cc
src/typelookup.cc
src/UnixSignalHandlers.cc
src/WallclockTimer.cc
src/WrappedClassName.cc
src/md5.c
)

set_target_properties(FWCoreUtilities PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_dependencies( ALL_LIBS FWCoreUtilities)

target_include_directories(FWCoreUtilities PRIVATE ${CMAKE_SOURCE_DIR})

target_include_directories(FWCoreUtilities PUBLIC
${Boost_INCLUDE_DIR}
${Boost_INCLUDE_DIR}/python
${FMT_INCLUDE_DIR}
${TBB_INCLUDE_DIRS}
/usr/include
)

target_link_libraries(FWCoreUtilities PUBLIC
TBB::tbb
Threads::Threads
crypt
dl
fmt::fmt
#nsl
rt
stdc++fs
)


install( TARGETS FWCoreUtilities DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
