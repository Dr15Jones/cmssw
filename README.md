# CMS_MiniAOD

## Purpose
This package is meant to allow non CMS developers to build enough of CMSSW to be able to read CMS' MiniAOD ROOT TTree based files.

The code was copied from the release CMSSW_14_1_0_pre6 and altered to make it easier to compile. The changes were
- Removing all directories not needed to build dictionaries related to the MiniAOD format. This included removal of test/ etc subdirectories of the kept packages.
- Removed some code not needed for reading the files in order to avoid additional unnecessary 3rd party dependencies.
- Removed code related to GPU data structures. This avoided additional dependencies.
- Adding two 3rd party header only packages directly into the source area to make it easier to build the code. This required modification of some CMSSW code to look for the headers in their new place.
- Generating CMakeLists.txt files from the BuildFile.xml files used by CMS build tools

## Needed 3rd Party Code
The code needs access to the following 3rd party sources in order to be built.

### Compiler
This code was compiled with gcc 12.3.1. Other compilers should work, however the code most likely expects the C++ standard version to be at least c++17.

### ROOT
The source can be found at https://github.com/root-project/root. This code was tested using a version of ROOT 6.33. In principal, newer versions should work fine as well.

### Boost
The source can be found at https://www.boost.org. The version of the code was tested using version 1.80. Only headers from boost are used, no libraries from boost are linked into the libraries built by this code. This code was tested using a version of Boost obtained from /cvmfs/cms.cern.ch/el8_amd64_gcc12/external/boost/1.80.0-a1544032d9d65904ac2112b6d35bba55

### CLHEP
The code was tested using the source from https://github.com/cms-externals/clhep with the git branch origin/cms/v2.4.7.1. The CMS version appears to have changed some global `const` variables to `constexpr` and the CMSSW code requires the `constexpr` version.

### TBB
The source can be found at https://github.com/oneapi-src/oneTBB.git. This was tested using the master branch from around July 31, 2024.

### fmt
The source can be found at https://github.com/fmtlib/fmt.git. This was tested using the master branch from around July 31, 2024.

### HepMC3
The source can be found at https://gitlab.cern.ch/hepmc/HepMC3.git. The code was tested using the git branch 3.2.7.

### HepMC
The source can be found at https://github.com/cms-externals/hepmc.git. The code was tested using the git branch cms/2.06.10. A FindHepMC.cmake file is included in this source distribution to allow easy use of HepMC with CMake.

## Building the Code
```
cmake -DBoost_INCLUDE_DIR=<where to find boost headers> \
      -DCLHEP_DIR=<where cmake file is found> \
      -DROOT_DIR=${ROOTSYS}/cmake \
      -DTBB_DIR=<where cmake file is found> \
      -Dfmt_DIR=<where cmake file is found> \
      -DHepMC3_DIR=<where cmake file is found> \
      -DHepMC_ROOT_DIR=<directory above include/ and lib/ holding the package> \
      ../cmssw
```

For my tests, most of the 3rd party libraries were told to install to ../external directory. With that, my cmake command was
```
cmake -DBoost_INCLUDE_DIR=/cvmfs/cms.cern.ch/el8_amd64_gcc12/external/boost/1.80.0-a1544032d9d65904ac2112b6d35bba55/include -DCLHEP_DIR=../../external/lib/CLHEP-2.1.4.2 -DROOT_DIR=${ROOTSYS}/cmake -DTBB_DIR=../../external/lib64/cmake/TBB -Dfmt_DIR=../../external/lib64/cmake/fmt -DHepMC3_DIR=../../external/share/HepMC3/cmake -DHepMC_ROOT_DIR=../../external ../cmssw
```

## Using libraries
To work, you will manually have to extend `LD_LIBRARY_PATH` to include the directories containing the libraries genrated by this code as well as the directories holding the 3rd party libraries. ROOT uses LD_LIBRARY_PATH to find the dictionaries it needs to read the CMS files.