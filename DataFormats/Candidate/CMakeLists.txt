include_directories(${CMAKE_SOURCE_DIR}
${ROOT_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsCandidate  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsCandidate )
add_library( DataFormatsCandidate  SHARED ${DataFormatsCandidate_EXTRA_SOURCES}
  src/Candidate.cc
  src/CompositeCandidate.cc
  src/CompositePtrCandidate.cc
  src/CompositeRefBaseCandidate.cc
  src/CompositeRefCandidate.cc
  src/LeafCandidate.cc
  src/LeafRefCandidateT.cc
  src/NamedCompositeCandidate.cc
  src/OverlapChecker.cc
  src/Particle.cc
  src/ShallowCloneCandidate.cc
  src/ShallowClonePtrCandidate.cc
  src/VertexCompositeCandidate.cc
  src/VertexCompositePtrCandidate.cc
)
set_target_properties( DataFormatsCandidate PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsCandidate )
target_include_directories( DataFormatsCandidate PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsCandidate PUBLIC
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsCandidate PUBLIC
   ROOT::GenVector
   ROOT::MathMore
   ROOT::Tree
   ROOT::Net
   ROOT::MathCore
   ROOT::Core
   ROOT::Thread
   ROOT::RIO
   ROOT::Core
   DataFormatsCommon
   DataFormatsGeometryVector
   DataFormatsMath
   FWCoreUtilities
)
install( TARGETS  DataFormatsCandidate DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
