cmake_minimum_required(VERSION 3.12)
project(MiniAOD VERSION 0.0.0 LANGUAGES C CXX)

if(BUILDTEST)
  include(CTest)
  enable_testing()
else()
  message('set BUILDTEST=True to run tests')
endif()
cmake_policy( SET CMP0082 NEW)
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
find_package(ROOT)
include(GNUInstallDirs)
set(CMAKE_INSTALL_LIBDIR "lib")
include(${ROOT_USE_FILE})
set(Boost_NO_BOOST_CMAKE ON)
set(Boost_NO_SYSTEM_PATHS ON)
find_package(Boost COMPONENTS filesystem thread iostreams regex serialization system program_options)
add_compile_definitions(BOOST_SPIRIT_THREADSAFE;PHOENIX_THREADSAFE;BOOST_MATH_DISABLE_STD_FPCLASSIFY;BOOST_UUID_RANDOM_PROVIDER_FORCE_POSIX;TBB_USE_GLIBCXX_VERSION=80400;TBB_SUPPRESS_DEPRECATED_MESSAGES)
find_package(Threads)
#find_package(CMakeTools)
#UseCMakeTools()
add_custom_target(ALL_LIBS ALL)
add_custom_target(ALL_DICTS ALL)
add_dependencies(ALL_LIBS ALL_DICTS)
add_custom_target(ALL_PLUGINS ALL)
add_dependencies(ALL_LIBS ALL_PLUGINS)
add_custom_target(ALL_TESTLIBS ALL)
add_dependencies(ALL_LIBS ALL_TESTLIBS)
add_custom_target(ALL_BINS ALL)
add_custom_target(ALL_TESTBINS ALL)
add_dependencies(ALL_BINS ALL_LIBS)
add_dependencies(ALL_TESTBINS ALL_LIBS)

set(pythondir ${CMAKE_BINARY_DIR}/python)
set(sourcedir ${CMAKE_BINARY_DIR}/src)
set(pcmfiles "")
set(rootmapfiles "")
file(MAKE_DIRECTORY ${sourcedir})
file(GLOB sources "${CMAKE_SOURCE_DIR}/*")
file(COPY ${sources} DESTINATION ${sourcedir} PATTERN ".git" EXCLUDE)
find_package(Boost)

find_package(CLHEP)

find_package(TBB)

#find_package(TINYXML2)

#find_package(VDT)

find_package(fmt)
find_package(HepMC3)
find_package(HepMC REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attributes")


 
macro(cms_rootdict name)
  if("${ARGN}" STREQUAL "")
    set(classes_h classes.h)
    set(classes_xml classes_def.xml)
    set(${name}_ROOTDICT ${name}_x)
  else()
    set(classes_h ${ARGV1})
    set(classes_xml ${ARGV2})
    if(${name}_ROOTDICT)
      MATH(EXPR ${name}_DICT_COUNT "${${name}_DICT_COUNT}+1")
      set(${name}_ROOTDICT ${name}_x${${name}_DICT_COUNT})
    else()
      set(${name}_DICT_COUNT 0)
      set(${name}_ROOTDICT ${name}_x)
    endif()
  endif()
  set(${name}_EXTRA_SOURCES ${${name}_EXTRA_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/${${name}_ROOTDICT}.cc)
  set(${name}_ROOTDICTS ${${name}_ROOTDICTS} ${${name}_ROOTDICT})
  set(ROOTDICT_${${name}_ROOTDICT} ${classes_h} ${classes_xml} ${CMAKE_CURRENT_BINARY_DIR}/${${name}_ROOTDICT}.cc)
  unset(clases_h)
  unset(classes_xml)
endmacro()

macro(add_rootdict_rules name)
  set(flags "")
  get_directory_property(defs COMPILE_DEFINITIONS)
  if(defs)
    foreach(d ${defs})
      set(flags ${flags} -D${d})
    endforeach()
  endif()
  get_directory_property(dirs INCLUDE_DIRECTORIES)
  if(dirs)
    foreach(inc ${dirs})
      set(flags ${flags} -I${inc})
    endforeach()
  endif()
  foreach(dict ${${name}_ROOTDICTS})
    list(GET ROOTDICT_${dict} 0 classes_h)
    list(GET ROOTDICT_${dict} 1 classes_xml)
    list(GET ROOTDICT_${dict} 2 classes_cc)
    get_filename_component(classname ${classes_h} NAME_WE)
    set_source_files_properties(${classes_cc} PROPERTIES COMPILE_FLAGS "-Os -Wno-unused-variable")
    add_custom_command(OUTPUT ${classes_cc}
                    COMMAND ${ROOT_genreflex_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/${classes_h}
                    -s ${CMAKE_CURRENT_SOURCE_DIR}/${classes_xml}
                    -o ${classes_cc}
                    --deep
                    --rootmap=${CMAKE_BINARY_DIR}/lib/${dict}.rootmap
                    --rootmap-lib=${CMAKE_BINARY_DIR}/lib/lib${name}.so
                    --library ${CMAKE_BINARY_DIR}/lib/lib${name}.so --multiDict
                    -DCMS_DICT_IMPL -D_REENTRANT -DGNUSOURCE -D__STRICT_ANSI__ ${FLAGS} -DCMSSW_REFLEX_DICT 
                    -I${CMAKE_SOURCE_DIR} ${flags}
                    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${classes_h} ${CMAKE_CURRENT_SOURCE_DIR}/${classes_xml}
                    )
    list(APPEND rootmapfiles ${CMAKE_BINARY_DIR}/lib/${dict}.rootmap)
    list(APPEND pcmfiles ${CMAKE_BINARY_DIR}/lib/lib${dict}_${dict}_x_rdict.pcm)
  endforeach()
  unset(clases_h)
  unset(classes_xml)
  unset(classname)
  unset(FLAGS)
endmacro()


add_subdirectory(DataFormats)
add_subdirectory(FWCore)
add_subdirectory(SimDataFormats)
#add_subdirectory(CondFormats)
add_subdirectory(Geometry)

install(DIRECTORY ${CMAKE_BINARY_DIR}/src DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${pcmfiles} ${rootmapfiles} ${CMAKE_BINARY_DIR}/lib/.edmplugincache DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
