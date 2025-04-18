import FWCore.ParameterSet.Config as cms
import os 

process = cms.Process("HcalValid")
process.load("DQMServices.Core.DQM_cfg")

process.load('Configuration.StandardSequences.EDMtoMEAtRunEnd_cff')
#Magnetic Field 		
process.load("Configuration.StandardSequences.MagneticField_cff")

#Geometry
process.load("Geometry.CMSCommonData.cmsExtendedGeometryXML_cfi")

process.load("Validation.HcalHits.ZdcSimHitStudy_cfi")
process.load("Validation.HcalDigis.ZDCDigiStudy_cfi")

process.load('FWCore.MessageService.MessageLogger_cfi')

if 'MessageLogger' in process.__dict__:
    process.MessageLogger.G4cout=dict()
    process.MessageLogger.ZdcSim=dict()
    process.MessageLogger.ZdcDigiStudy=dict()

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)
process.source = cms.Source("PoolSource",
#    debugFlag = cms.untracked.bool(True),
#    debugVebosity = cms.untracked.uint32(11),
       fileNames = cms.untracked.vstring('file:simevent.root')
)

# load DQM
process.load("DQMServices.Core.DQM_cfg")
process.load("DQMServices.Components.DQMEnvironment_cfi")
process.dqmsave_step = cms.Path(process.DQMSaver)

print(process.ZDCDigiStudy.Verbose)
process.p1 = cms.Path(
                     process.ZDCDigiStudy
                     *process.zdcSimHitStudy)

process.schedule = cms.Schedule(process.p1,
                                process.dqmsave_step)

#process.DQM.collectorHost = ''
process.zdcSimHitStudy.outputFile = 'zdcStudy.root'
process.ZDCDigiStudy.outputFile='zdcStudy.root'

