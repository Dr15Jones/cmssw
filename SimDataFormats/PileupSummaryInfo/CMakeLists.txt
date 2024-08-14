include_directories(${CMAKE_SOURCE_DIR}
${HepMC_INCLUDE_DIR}
/usr/include
)
cms_rootdict( SimDataFormatsPileupSummaryInfo  src/classes.h src/classes_def.xml)
add_rootdict_rules( SimDataFormatsPileupSummaryInfo )
add_library( SimDataFormatsPileupSummaryInfo  SHARED ${SimDataFormatsPileupSummaryInfo_EXTRA_SOURCES}
  src/PileupSummaryInfo.cc
)
set_target_properties( SimDataFormatsPileupSummaryInfo PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS SimDataFormatsPileupSummaryInfo )
target_include_directories( SimDataFormatsPileupSummaryInfo PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( SimDataFormatsPileupSummaryInfo PUBLIC
  ${HepMC_INCLUDE_DIR}
/usr/include
)
target_link_libraries( SimDataFormatsPileupSummaryInfo PUBLIC
   HepMC::HepMC
   DataFormatsMath
   DataFormatsCommon
   DataFormatsProvenance
)
install( TARGETS  SimDataFormatsPileupSummaryInfo DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
