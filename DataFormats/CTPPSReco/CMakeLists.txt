include_directories(${CMAKE_SOURCE_DIR}
${ROOT_INCLUDE_DIR}
${ROOT_INCLUDE_DIR}
${ROOT_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsCTPPSReco  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsCTPPSReco )
add_library( DataFormatsCTPPSReco  SHARED ${DataFormatsCTPPSReco_EXTRA_SOURCES}
  src/CTPPSDiamondLocalTrack.cc
  src/CTPPSPixelLocalTrack.cc
  src/CTPPSTimingLocalTrack.cc
  src/TotemRPLocalTrack.cc
  src/TotemRPUVPattern.cc
)
set_target_properties( DataFormatsCTPPSReco PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsCTPPSReco )
target_include_directories( DataFormatsCTPPSReco PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsCTPPSReco PUBLIC
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsCTPPSReco PUBLIC
   ROOT::Smatrix
   ROOT::Core
   ROOT::GenVector
   ROOT::MathMore
   ROOT::Tree
   ROOT::Net
   ROOT::MathCore
   ROOT::Core
   ROOT::Thread
   ROOT::RIO
   ROOT::Core
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
   DataFormatsCommon
   DataFormatsCTPPSDigi
   DataFormatsGeometrySurface
   DataFormatsMath
   DataFormatsCTPPSDetId
   DataFormatsGeometryVector
)
install( TARGETS  DataFormatsCTPPSReco DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
