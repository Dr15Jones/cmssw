include_directories(${CMAKE_SOURCE_DIR}
${CLHEP_INCLUDE_DIR}
${ROOT_INCLUDE_DIR}
/usr/include
)
cms_rootdict( DataFormatsTrackReco  src/classes.h src/classes_def.xml)
add_rootdict_rules( DataFormatsTrackReco )
add_library( DataFormatsTrackReco  SHARED ${DataFormatsTrackReco_EXTRA_SOURCES}
  src/DeDxData.cc
  src/HitPattern.cc
  src/SeedStopReason.cc
  src/Track.cc
  src/TrackBase.cc
  src/TrackExtra.cc
  src/TrackResiduals.cc
  src/fillCovariance.cc
  src/rootio_HitPattern.cc
)
set_target_properties( DataFormatsTrackReco PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsTrackReco )
target_include_directories( DataFormatsTrackReco PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsTrackReco PUBLIC
  ${CLHEP_INCLUDE_DIR}
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsTrackReco PUBLIC
   ROOT::GenVector
   ROOT::MathMore
   ROOT::Tree
   ROOT::Net
   ROOT::MathCore
   ROOT::Core
   ROOT::Thread
   ROOT::RIO
   ROOT::Core
   DataFormatsBeamSpot
   DataFormatsCandidate
   DataFormatsCommon
   DataFormatsDetId
   DataFormatsForwardDetId
   DataFormatsGeometryVector
   DataFormatsMath
   DataFormatsMuonDetId
   DataFormatsScouting
   DataFormatsSiPixelCluster
   DataFormatsSiPixelDetId
   DataFormatsSiStripCluster
   DataFormatsSiStripDetId
   DataFormatsTrackCandidate
   DataFormatsTrackerCommon
   DataFormatsTrackingRecHit
   DataFormatsTrajectorySeed
   DataFormatsTrajectoryState
   FWCoreUtilities
)
install( TARGETS  DataFormatsTrackReco DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
