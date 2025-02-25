from builtins import range
import os, sys
import FWCore.ParameterSet.Config as cms

# Parameters for runType
import FWCore.ParameterSet.VarParsing as VarParsing
from .dqmPythonTypes import *

'''
check that the input directory exists and there are files in it
'''
def checkInputFolder(streamer_folder):
    if not (os.path.exists(streamer_folder) and os.path.isdir(os.path.join(streamer_folder))):
        raise IOError("Input folder '%s' does not exist in CMSSW_SEARCH_PATH" % streamer_folder)

    items = os.listdir(dqm_streamer_folder)
    if not items:
        raise IOError("Input folder '%s' does not contain any file" % streamer_folder)

# Dedine and register options
options = VarParsing.VarParsing("analysis")

# Parameters for runType
options.register('runkey',
                 'pp_run',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Run Keys of CMS")

# Parameter for frontierKey
options.register('runUniqueKey',
                 'InValid',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Unique run key from RCMS for Frontier")

options.register('runNumber',
                 356383,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "Run number. This run number has to be present in https://github.com/cms-data/DQM-Integration")

options.register('datafnPosition',
                 3, # default value
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "Data filename position in the positional arguments array 'data' in json file.")

options.register('streamLabel',
                 '',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 "Name of the stream")

options.register('noDB',
                 True, # default value
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.bool,
                 "Don't upload the BeamSpot conditions to the DB")

options.register('scanOnce',
                 True, # default value
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.bool,
                 "Don't repeat file scans: use what was found during the initial scan. EOR file is ignored and the state is set to 'past end of run'.")

options.register('skipFirstLumis',
                 False, # default value
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.bool,
                 "Skip (and ignore the minEventsPerLumi parameter) for the files which have been available at the beginning of the processing.")

options.register('BeamSplashRun',
                 False, # default value
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.bool,
                 "Set client source settings for beam SPLASH run")

# This is used only by the online clients themselves. 
# We need to register it here because otherwise an error occurs saying that there is an unidentified option.
options.register('unitTest',
                 True,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.bool,
                 "Required to avoid the error.")

options.parseArguments()

# Read streamer files from https://github.com/cms-data/DQM-Integration
dqm_integration_data = [os.path.join(dir,'DQM/Integration/data') for dir in os.getenv('CMSSW_SEARCH_PATH','').split(":") if os.path.exists(os.path.join(dir,'DQM/Integration/data'))][0]
dqm_streamer_folder = os.path.join(dqm_integration_data,'run'+str(options.runNumber))
print("Reading streamer files from:\n ",dqm_streamer_folder)
checkInputFolder(dqm_streamer_folder)

maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)

runType = RunType()
if not options.runkey.strip():
    options.runkey = "pp_run"

runType.setRunType(options.runkey.strip())

# stream label
#  If streamLabel is empty, so not specified as a command-line argument,
#  use the default value, i.e. "streamHIDQM" for runType==hi_run and "streamDQM" otherwise
streamLabel = options.streamLabel
if streamLabel == '':
    if runType.getRunType() == runType.hi_run:
        streamLabel = 'streamHIDQM'
    else:
        streamLabel = 'streamDQM'

# Set the process source
source = cms.Source("DQMStreamerReader",
    runNumber = cms.untracked.uint32(options.runNumber),
    runInputDir = cms.untracked.string(dqm_integration_data),
    SelectEvents = cms.untracked.vstring('*'),
    streamLabel = cms.untracked.string(streamLabel),
    scanOnce = cms.untracked.bool(options.scanOnce),
    datafnPosition = cms.untracked.uint32(options.datafnPosition),
    minEventsPerLumi = cms.untracked.int32(1000),
    delayMillis = cms.untracked.uint32(500),
    nextLumiTimeoutMillis = cms.untracked.int32(0),
    skipFirstLumis = cms.untracked.bool(options.skipFirstLumis),
    deleteDatFiles = cms.untracked.bool(False),
    endOfRunKills  = cms.untracked.bool(False),
    inputFileTransitionsEachEvent = cms.untracked.bool(False),
    unitTest = cms.untracked.bool(True) # stop processing if the input data cannot be deserialized
)
