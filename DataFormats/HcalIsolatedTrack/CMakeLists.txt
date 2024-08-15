cms_rootdict( DataFormatsHcalIsolatedTrack  src/classes.h src/classes_def.xml)
add_library( DataFormatsHcalIsolatedTrack  SHARED ${DataFormatsHcalIsolatedTrack_EXTRA_SOURCES}
  src/EcalIsolatedParticleCandidate.cc
  src/HcalIsolatedTrackCandidate.cc
  src/IsolatedPixelTrackCandidate.cc
)
set_target_properties( DataFormatsHcalIsolatedTrack PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsHcalIsolatedTrack )
target_include_directories( DataFormatsHcalIsolatedTrack PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsHcalIsolatedTrack PUBLIC
/usr/include
)
target_link_libraries( DataFormatsHcalIsolatedTrack PUBLIC
   DataFormatsCandidate
   DataFormatsCommon
   DataFormatsMath
   DataFormatsRecoCandidate
   DataFormatsTrackReco
   DataFormatsL1Trigger
)
add_rootdict_rules( DataFormatsHcalIsolatedTrack )
install( TARGETS  DataFormatsHcalIsolatedTrack DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
