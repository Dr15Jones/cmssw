include_directories(${CMAKE_SOURCE_DIR}
/usr/include
)
cms_rootdict( DataFormatsL1TGlobal  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsL1TGlobal )
add_library( DataFormatsL1TGlobal  SHARED ${DataFormatsL1TGlobal_EXTRA_SOURCES}
  src/AXOL1TLScore.cc
  src/GlobalAlgBlk.cc
  src/GlobalExtBlk.cc
  src/GlobalLogicParser.cc
  src/GlobalObject.cc
  src/GlobalObjectMap.cc
  src/GlobalObjectMapRecord.cc
)
set_target_properties( DataFormatsL1TGlobal PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsL1TGlobal )
target_include_directories( DataFormatsL1TGlobal PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsL1TGlobal PUBLIC
/usr/include
)
target_link_libraries( DataFormatsL1TGlobal PUBLIC
   DataFormatsCommon
   DataFormatsL1Trigger
   FWCoreMessageLogger
   FWCoreUtilities
)
install( TARGETS  DataFormatsL1TGlobal DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
