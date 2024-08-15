cms_rootdict( SimDataFormatsGeneratorProducts  src/classes.h src/classes_def.xml)
add_library( SimDataFormatsGeneratorProducts  SHARED ${SimDataFormatsGeneratorProducts_EXTRA_SOURCES}
  src/GenEventInfoProduct.cc
  src/GenEventInfoProduct3.cc
  src/GenFilterInfo.cc
  src/GenLumiInfoHeader.cc
  src/GenLumiInfoProduct.cc
  src/GenRunInfoProduct.cc
  src/HepMC3Product.cc
  src/HepMCProduct.cc
  src/LHEEventProduct.cc
  src/LHERunInfoProduct.cc
  src/LHEXMLStringProduct.cc
  src/hepmc_rootio.cc
)
set_target_properties( SimDataFormatsGeneratorProducts PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_dependencies( ALL_LIBS SimDataFormatsGeneratorProducts )
target_include_directories( SimDataFormatsGeneratorProducts PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories( SimDataFormatsGeneratorProducts PUBLIC
  ${ROOT_INCLUDE_DIR}
  ${HepMC_INCLUDE_DIR}
  ${HEPMC3_INCLUDE_DIR}
/usr/include
)
target_link_libraries( SimDataFormatsGeneratorProducts PUBLIC
   HepMC::HepMC
   ${HEPMC3_LIBRARIES}
   ROOT::Hist
   ROOT::Matrix
   ROOT::Tree
   ROOT::Net
   ROOT::MathCore
   ROOT::Core
   ROOT::Thread
   ROOT::RIO
   ROOT::Core
   FWCoreUtilities
   FWCoreMessageLogger
   DataFormatsCommon
)
add_rootdict_rules( SimDataFormatsGeneratorProducts )
install( TARGETS  SimDataFormatsGeneratorProducts DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)
