cms_rootdict( DataFormatsHepMCCandidate  src/classes.h src/classes_def.xml)
add_library( DataFormatsHepMCCandidate  SHARED ${DataFormatsHepMCCandidate_EXTRA_SOURCES}
  src/FlavorHistory.cc
  src/FlavorHistoryEvent.cc
  src/GenParticle.cc
  src/GenStatusFlags.cc
)
set_target_properties( DataFormatsHepMCCandidate PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsHepMCCandidate )
target_include_directories( DataFormatsHepMCCandidate PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsHepMCCandidate PUBLIC
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsHepMCCandidate PUBLIC
   ROOT::Physics
   ROOT::Hist
   ROOT::Matrix
   ROOT::Tree
   ROOT::Net
   ROOT::MathCore
   ROOT::Core
   ROOT::Thread
   ROOT::RIO
   ROOT::Core
   DataFormatsCandidate
   DataFormatsCommon
)
add_rootdict_rules( DataFormatsHepMCCandidate )
install( TARGETS  DataFormatsHepMCCandidate DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
