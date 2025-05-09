
import sys
import os
import FWCore.ParameterSet.Config as cms

print('Number of arguments:', len(sys.argv), 'arguments.')
print('Argument List:', str(sys.argv))
# first arg : cmsRun
# second arg : name of the _cfg file
# third arg : sample name (ex. ZEE_14)

from electronValidationCheck_Env import env

cmsEnv = env() # be careful, cmsEnv != cmsenv. cmsEnv is local

cmsEnv.checkSample() # check the sample value
cmsEnv.checkValues()

import DQMOffline.EGamma.electronDataDiscovery as dd

if cmsEnv.beginTag() == 'Run2_2017':
    from Configuration.Eras.Era_Run2_2017_cff import Run2_2017
    process = cms.Process("electronValidation",Run2_2017)
elif cmsEnv.beginTag() == 'Run3':
    from Configuration.Eras.Era_Run3_cff import Run3
    process = cms.Process('electronValidation', Run3) 
else:
    from Configuration.Eras.Era_Phase2_cff import Phase2
    process = cms.Process('electronValidation',Phase2)

process.DQMStore = cms.Service("DQMStore")
process.load("DQMServices.Components.DQMStoreStats_cfi")
from DQMServices.Components.DQMStoreStats_cfi import *

dqmStoreStats.runOnEndJob = cms.untracked.bool(True)

print("reading files ...")
max_number = -1 # number of events
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(max_number))

data = os.environ['data']
flist = dd.getCMSdata(data)
print(flist)
process.source = cms.Source("PoolSource",
    #eventsToProcess = cms.untracked.VEventRange('1:38-1:40'),
    fileNames = cms.untracked.vstring(*flist))

#process.source = cms.Source ("PoolSource",
#eventsToProcess = cms.untracked.VEventRange('1:2682-1:2682'),
#eventsToProcess = cms.untracked.VEventRange('1:8259-1:8259'),
#eventsToProcess = cms.untracked.VEventRange('1:10-1:10'),
#fileNames = cms.untracked.vstring([
#        'file:PAT_249120E2-D1EC-E611-83C2-0CC47A7C347A.root',
#        'file:PAT_76F9AD07-D3EC-E611-AA87-0CC47A745250.root',
#        'file:PAT_FA0E1D02-D5EC-E611-B8CA-0025905A6080.root',
#        'file:PAT_EE728E01-D5EC-E611-9DC5-0025905A6126.root',
#        '/store/relval/CMSSW_9_1_0_pre2/RelValSingleElectronPt10/MINIAODSIM/90X_upgrade2017_realistic_v20-v1/00000/66B5E60E-5619-E711-8E17-0CC47A4D7632.root',
#        '/store/relval/CMSSW_9_1_0_pre2/RelValSingleElectronPt10/MINIAODSIM/90X_upgrade2017_realistic_v20-v1/00000/BC08FC0E-5619-E711-98E5-0CC47A4D760C.root',
#    ]),
#secondaryFileNames = cms.untracked.vstring() )
#process.source.fileNames.extend(dd.search())
print("reading files done")

process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi')
process.load('Configuration.StandardSequences.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.RawToDigi_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load("Configuration.StandardSequences.EDMtoMEAtJobEnd_cff") # new
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.autoCond import autoCond
#process.GlobalTag.globaltag = os.environ['TEST_GLOBAL_TAG']#+'::All'
#process.GlobalTag.globaltag = '122X_mcRun4_realistic_v1'

# FOR DATA REDONE FROM RAW, ONE MUST HIDE IsoFromDeps
# CONFIGURATION
process.load("Validation.RecoEgamma.electronIsoFromDeps_cff")
process.load("Validation.RecoEgamma.ElectronIsolation_cfi")
process.load("Validation.RecoEgamma.ElectronMcSignalValidatorMiniAOD_cfi")

print("miniAODElectronIsolation call")
from Validation.RecoEgamma.electronValidationSequenceMiniAOD_cff import miniAODElectronIsolation # as _ElectronIsolationCITK
process.miniAODElectronIsolation = miniAODElectronIsolation
print("miniAODElectronIsolation clone done")

# load DQM
process.load("DQMServices.Core.DQM_cfg")
process.load("DQMServices.Components.DQMEnvironment_cfi")

#process.printContent = printContent

process.EDM = cms.OutputModule("PoolOutputModule",
outputCommands = cms.untracked.vstring('drop *',"keep *_MEtoEDMConverter_*_*"),
#fileName = cms.untracked.string(TEST_HISTOS_FILE)
fileName = cms.untracked.string(os.environ['outputFile'])#.replace(".root", "_a.root"))
)

process.electronMcSignalValidatorMiniAOD.InputFolderName = cms.string("EgammaV/ElectronMcSignalValidatorMiniAOD")
process.electronMcSignalValidatorMiniAOD.OutputFolderName = cms.string("EgammaV/ElectronMcSignalValidatorMiniAOD")

#process.p = cms.Path(process.ElectronIsolation * process.electronMcSignalValidatorMiniAOD * process.MEtoEDMConverter ) # * process.dqmStoreStats
#process.p = cms.Path(process.electronMcSignalValidatorMiniAOD * process.MEtoEDMConverter * process.dqmStoreStats)
process.p = cms.Path( process.miniAODElectronIsolation * process.ElectronIsolation * process.electronMcSignalValidatorMiniAOD * process.MEtoEDMConverter ) #  process.printContent *

process.outpath = cms.EndPath(
process.EDM,
)
