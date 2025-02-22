include ${SADIR}/Makefile.config

CPPFLAGS := -I. -I${SRCDIR} -I../mkFit-external ${CPPFLAGS}

CMS_DIR := ${SRCDIR}/RecoTracker/MkFitCMS

LIB_CMS := ../libMicCMS.so
MAIN    := ../mkFit
MAIN_DEPS := ${LIB_CMS}
MAIN_LIBS := -lMicCore -lMicCMS
WRMEMF  := ../writeMemoryFile
WMF_DICT_PCM := ../WriteMemFileDict_rdict.pcm
SHELL_DICT_PCM := ../ShellDict_rdict.pcm
ROOTOUT := WriteMemFileDict.cc ShellDict.cc

TGTS := ${LIB_CMS} ${MAIN}

ifdef WITH_ROOT
MAIN_DEPS += ../libMicRntDump.so
MAIN_LIBS += -lMicRntDump
TGTS += ${WRMEMF} ${WMF_DICT_PCM} ${SHELL_DICT_PCM}
endif

.PHONY: all clean distclean

all: ${TGTS}

SRCS := $(wildcard ${CMS_DIR}/src/*.cc) $(wildcard ${SACMS}/*.cc)

ifdef WITH_ROOT
SRCS += ${SACMS}/tkNtuple/WriteMemoryFile.cc
WriteMemFileDict.cc ${WMF_DICT_PCM} &: ${SACMS}/tkNtuple/DictsLinkDef.h
	rootcling ${CPPFLAGS} -I=${SRCDIR} -f WriteMemFileDict.cc $<
	mv WriteMemFileDict_rdict.pcm ${WMF_DICT_PCM}

SRCS += ShellDict.cc
ShellDict.cc ${SHELL_DICT_PCM} &: ${SACMS}/ShellDict.h ${SACMS}/ShellLinkDef.h
	rootcling ${CPPFLAGS} -I=${SRCDIR} -f ShellDict.cc $^
	mv ShellDict_rdict.pcm ${SHELL_DICT_PCM}
else
SRCS := $(filter-out ${SACMS}/Shell.cc, ${SRCS})
endif

SRCB := $(notdir ${SRCS})
DEPS := $(SRCB:.cc=.d)
OBJS := $(SRCB:.cc=.o)

CMS_OBJS := $(filter-out mkFit.o WriteMemoryFile.o, ${OBJS})

vpath %.cc ${CMS_DIR}/src ${SACMS} ${SACMS}/tkNtuple

ifeq ($(filter clean-local clean distclean, ${MAKECMDGOALS}),)
include ${DEPS}
endif

clean-local:
	-rm -f ${TGTS} *.d *.o *.om *.so *.pcm ${ROOTOUT}
	-rm -rf main.dSYM
	-rm -rf plotting/*.so plotting/*.d plotting/*.pcm

clean: clean-local

distclean: clean-local
	-rm -f *.optrpt
	-rm -f ${TGTS}

${LIB_CMS}: ${CMS_OBJS}
	@mkdir -p $(@D)
	${CXX} ${CXXFLAGS} ${VEC_HOST} ${CMS_OBJS} -shared -o $@ ${LDFLAGS_HOST} ${LDFLAGS}

${MAIN}: ${MAIN_DEPS} mkFit.o
	${CXX} ${CXXFLAGS} ${VEC_HOST} ${LDFLAGS} mkFit.o -o $@ ${LDFLAGS_HOST} -ltbb -L.. ${MAIN_LIBS} -Wl,-rpath=.

${WRMEMF}: ${MAIN_DEPS} WriteMemoryFile.o WriteMemFileDict.o
	${CXX} ${CXXFLAGS} ${LDFLAGS} WriteMemoryFile.o WriteMemFileDict.o -o $@ ${LDFLAGS_HOST} -ltbb -L.. ${MAIN_LIBS} -Wl,-rpath=.

${OBJS}: %.o: %.cc %.d
	${CXX} ${CPPFLAGS} ${CXXFLAGS} ${VEC_HOST} -c -o $@ $<

%.d: %.cc
	${MAKEDEPEND} -o $@ $<

echo:
	@echo "CXX=${CXX}"
	@echo SRCS = ${SRCS}
	@echo DEPS = ${DEPS}
	@echo OBJS = ${OBJS}
	@echo MAIN_LIBS = ${MAIN_LIBS}

echo_cc_defs:
	${CXX} -dM -E -mavx2 - < /dev/null

echo-srcs:
	@echo ${SRCS}

echo-flags:
	@echo "CPPFLAGS=${CPPFLAGS}"

echo-tbb:
	@echo "TBB_GCC=${TBB_GCC}, TBB_PREFIX=${TBB_PREFIX}, TBB_ROOT=${TBB_ROOT}"
