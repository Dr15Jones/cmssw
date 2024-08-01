add_library(FWCoreMessageLogger SHARED ${FWCoreMessageLogger_EXTRA_SOURCES}
  src/AbstractMLscribe.cc
  src/ELextendedID.cc
  src/ELmap.cc
  src/ELseverityLevel.cc
  src/ErrorObj.cc
  #src/ExceptionMessages.cc
  #src/JobReport.cc
  src/MessageDrop.cc
  src/MessageLogger.cc
  src/MessageLoggerQ.cc
  src/MessageSender.cc
  src/edm_MessageLogger.cc
)

set_target_properties(FWCoreMessageLogger PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_dependencies( ALL_LIBS FWCoreMessageLogger)

target_include_directories(FWCoreMessageLogger PRIVATE ${CMAKE_SOURCE_DIR})

target_include_directories(FWCoreMessageLogger PUBLIC
${Boost_INCLUDE_DIR}
${Boost_INCLUDE_DIR}/python
${FMT_INCLUDE_DIR}
${TBB_INCLUDE_DIRS}
/usr/include
)

target_link_libraries(FWCoreMessageLogger PUBLIC
${TBB_LIBRARIES}
FWCoreUtilities
Threads::Threads
crypt
dl
fmt::fmt
rt
)


install( TARGETS FWCoreMessageLogger DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
