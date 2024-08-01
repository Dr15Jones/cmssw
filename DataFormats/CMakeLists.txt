#add_subdirectory(TestObjects)

add_subdirectory(Provenance)

#add_subdirectory(Streamer)

#add_subdirectory(WrappedStdDictionaries)

add_subdirectory(Common)

#add_subdirectory(FEDRawData)

#add_subdirectory(StdDictionaries)

#add_subdirectory(FWLite)

add_custom_target(DataFormats_all)

add_dependencies(DataFormats_all
#  DataFormatsTestObjects
  DataFormatsProvenance
#  DataFormatsStreamer
  DataFormatsCommon
#  DataFormatsFEDRawData
#  DataFormatsFWLite
)
