from xml.parsers.expat import ParserCreate

import sys
from argparse import ArgumentParser
import os

parser = ArgumentParser()
parser.add_argument("file",type=str)
options = parser.parse_args()

fullFileName = options.file
parts = fullFileName.split('/')
if len(parts) != 3:
    print("need to specify file from the src directory")
    exit(1)
if parts[-1] != 'BuildFile.xml':
    print("need to use a BuildFile.xml file")
    exit(1)

f = open(fullFileName, 'r')

def root_libraries():
    
    root_interface = []
    rootcling = ['ROOT::Core']+ root_interface
    rootrflx = root_interface+ rootcling
    rootsmatrix = ['ROOT::Smatrix'] +rootcling
    rootmathcore = ['ROOT::MathCore']+ rootcling
    rootrio = ['ROOT::RIO'] + rootcling
    rootthread = ['ROOT::Thread'] +rootrio
    rootcore = ['ROOT::Tree', 'ROOT::Net'] + rootmathcore + rootthread
    rootmath = ['ROOT::GenVector', 'ROOT::MathMore'] +rootcore
    roothistmatrix = ['ROOT::Hist', 'ROOT::Matrix']+ rootcore
    rootphysics = ['ROOT::Physics'] +roothistmatrix
    root = rootphysics
    return {'rootcore': rootcore,
            'roothistmatrix': roothistmatrix,
            'rootmath':rootmath,
            'rootphysics':rootphysics,
            'rootrflx':rootrflx,
            'rootsmatrix':rootsmatrix,
            'root':root}

rootlibs = root_libraries()
externals = dict( hls = ['${CMAKE_SOURCE_DIR}/hls',[]],
                  boost = ['${Boost_INCLUDE_DIR}',[]],
                  boost_regex = ['${Boost_INCLUDE_DIR}',[]],
                  clhep = ['${CLHEP_INCLUDE_DIR}',['${CLHEP_LIBRARIES}']],
                  clhepheader = ['${CLHEP_INCLUDE_DIR}', []],
                  hepmc = ['${HepMC_INCLUDE_DIR}', ['HepMC::HepMC']],
                  hepmc3 = ['${HEPMC3_INCLUDE_DIR}', ['${HEPMC3_LIBRARIES}']],
                  root = ['${ROOT_INCLUDE_DIR}',rootlibs['root']],
                  rootcore = ['${ROOT_INCLUDE_DIR}',rootlibs['rootcore']],
                  roothistmatrix=['${ROOT_INCLUDE_DIR}',rootlibs['roothistmatrix']],
                  rootmath=['${ROOT_INCLUDE_DIR}',rootlibs['rootmath']],
                  rootphysics=['${ROOT_INCLUDE_DIR}',rootlibs['rootphysics']],
                  rootrflx=['${ROOT_INCLUDE_DIR}',rootlibs['rootrflx']],
                  rootsmatrix=['${ROOT_INCLUDE_DIR}',rootlibs['rootsmatrix']],
                  tbb=['${TBB_INCLUDE_DIRS}',['${TBB_LIBRARIES}']]
                  )

s = '<start>\n'
for l in f:
    s += l
s += '</start>\n'

#Geometry/CommonDetUnit declares no src so makes no library
exclude_packages = set(['HeterogeneousCore/AlpakaInterface', 'DataFormats/Portable', 'Geometry/CommonDetUnit', 'DataFormats/SoATemplate', 'DataFormats/MuonData', 'CondFormats/Alignment'])

dependencies = []
external_dependencies = []
def start_element(name, attrs):
    if name == 'use':
        if 'name' in attrs:
            lib = attrs['name']
            if lib in externals:
                external_dependencies.append(lib)
            elif lib not in exclude_packages:
                
                dependencies.append("".join(lib.split("/")))
                
#print(s)
parser = ParserCreate()
parser.StartElementHandler = start_element

parser.Parse(s)
package = parts[0]+parts[1]
#print(package)
#print(dependencies)
#print(external_dependencies)


def printRootDict(package, headers,xml):
    #find matching pairs of classes(_\w)*.h and classes(_(\w))*_def(_(\w)).xml
    import re
    for x in xml:
        found = re.search('classes(_(?P<first>\w+))?_def(_(?P<last>\w+))?\.xml', x)
        first = ''
        if found.group("first") is not None:
            first = '_'+found.group("first")
        last = ''
        if found.group("last") is not None:
            last = '_'+found.group("last")
        print("cms_rootdict(",package, " src/classes"+first+last+".h src/"+x+")")

def printRootDictRules(package):
    print("add_rootdict_rules(",package,")")
    
def printLibrary(package, sources, include_dependencies, library_dependencies):
    print("add_library(",package, " SHARED ${"+package+"_EXTRA_SOURCES}")
    for s in sources:
        print("  src/"+s)
    print(")")
    print("set_target_properties(",package,"PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)")
    print("add_dependencies( ALL_LIBS",package,")")
    print("target_include_directories(",package,"PRIVATE ${CMAKE_SOURCE_DIR})")
    print("target_include_directories(",package,"PUBLIC")
    for d in include_dependencies:
        print(" ",d)
    print(  "/usr/include")
    print(")")

    print("target_link_libraries(",package,"PUBLIC")
    for l in library_dependencies:
        print("  ",l)
    print(")")

def printInstall(package):
    print("install( TARGETS ",package, "DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} EXPORT cms_miniaod::)")
    
srcDir = "/".join(parts[0:2])+"/src"
classHeaders = []
classXML = []
sources = []
for f in os.listdir(srcDir):
    if f[-3:] == ".cc":
        sources.append(f)
        #print(f)
    if f[0:7] == 'classes' and f[-2:] == '.h':
        classHeaders.append(f)
        #print(" dict ",f)
    if f[0:7] == 'classes' and f[-4:] == '.xml':
        classXML.append(f)
        #print(" xml ",f)
    if os.path.isdir(f):
        for fs in os.listdir( srcDir+"/"+f):
            if fs[-3:] == '.cc':
                source.append(f+"/"+fs)
        
external_libraries =[]
for dep in external_dependencies:
    for ex in externals[dep][1]:
        external_libraries.append(ex)
if len(classHeaders) > 0:
    printRootDict(package, classHeaders, classXML)
printLibrary(package, sources, list(set((externals[i][0] for i in external_dependencies))),  external_libraries + dependencies)
printRootDictRules(package)
printInstall(package)
