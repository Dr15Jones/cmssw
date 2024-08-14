# FindHepMC.cmake
# CMake module to find the HepMC library and include files

# Documentation for Find modules:
# The result of this module is stored in a variable HepMC_FOUND
# Other variables set are:
#  HepMC_INCLUDE_DIRS  - the include directory containing HepMC headers
#  HepMC_LIBRARIES     - the HepMC library to link against

# Locate the library and headers
find_path(HepMC_INCLUDE_DIR
    NAMES HepMC/GenEvent.h
    PATHS ${HepMC_ROOT_DIR}/include /usr/include /usr/local/include
)

find_library(HepMC_LIBRARY
    NAMES HepMC
    PATHS ${HepMC_ROOT_DIR}/lib /usr/lib /usr/local/lib
)

# Handle the result of the search
if(HepMC_INCLUDE_DIR AND HepMC_LIBRARY)
    set(HepMC_FOUND TRUE)
    set(HepMC_LIBRARIES ${HepMC_LIBRARY})
    set(HepMC_INCLUDE_DIRS ${HepMC_INCLUDE_DIR})
else()
    set(HepMC_FOUND FALSE)
endif()

# Set HepMC_FOUND to be a required variable for downstream users
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HepMC REQUIRED_VARS HepMC_INCLUDE_DIR HepMC_LIBRARY)

# Print status messages
if(HepMC_FOUND)
    message(STATUS "Found HepMC: ${HepMC_LIBRARY}")
    message(STATUS "HepMC includes: ${HepMC_INCLUDE_DIR}")
else()
    message(WARNING "Could not find HepMC")
endif()

# Export targets
if(NOT TARGET HepMC::HepMC)
    add_library(HepMC::HepMC UNKNOWN IMPORTED)
    set_target_properties(HepMC::HepMC PROPERTIES
        IMPORTED_LOCATION "${HepMC_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${HepMC_INCLUDE_DIR}"
    )
endif()
