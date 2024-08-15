cms_rootdict( DataFormatsCaloRecHit  src/classes.h src/classes_def.xml)
add_library( DataFormatsCaloRecHit  SHARED ${DataFormatsCaloRecHit_EXTRA_SOURCES}
  src/CaloCluster.cc
  src/CaloID.cc
  src/CaloRecHit.cc
)
set_target_properties( DataFormatsCaloRecHit PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS DataFormatsCaloRecHit )
target_include_directories( DataFormatsCaloRecHit PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( DataFormatsCaloRecHit PUBLIC
  ${ROOT_INCLUDE_DIR}
/usr/include
)
target_link_libraries( DataFormatsCaloRecHit PUBLIC
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
   DataFormatsMath
   DataFormatsDetId
   FWCoreUtilities
)
add_rootdict_rules( DataFormatsCaloRecHit )
install( TARGETS  DataFormatsCaloRecHit DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
