# The following comments couldn't be translated into the new config version:

# upload to database 

#string timetype = "timestamp"    

import FWCore.ParameterSet.Config as cms

process = cms.Process("Reader")

process.MessageLogger = cms.Service("MessageLogger",
    cerr = cms.untracked.PSet(
        enable = cms.untracked.bool(False)
    ),
    debugModules = cms.untracked.vstring(''),
    files = cms.untracked.PSet(
        QualityReader = cms.untracked.PSet(
            threshold = cms.untracked.string('INFO')
        )
    )
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(100)
)
process.source = cms.Source("EmptySource",
    numberEventsInRun = cms.untracked.uint32(1),
    firstRun = cms.untracked.uint32(1)
)

process.poolDBESSource = cms.ESSource("PoolDBESSource",
   DBParameters = cms.PSet(
        messageLevel = cms.untracked.int32(2),
        authenticationPath = cms.untracked.string('/afs/cern.ch/cms/DB/conddb')
    ),
    connect = cms.string('sqlite_file:dbfile.db'),
    toGet = cms.VPSet(
    cms.PSet(record = cms.string('SiStripBadModuleRcd'),tag = cms.string('SiStripBadModule_Fake_merged2'))
    )
)

process.load("CalibTracker.SiStripESProducers.SiStripQualityESProducer_cfi")
process.siStripQualityESProducer.ListOfRecordToMerge = cms.VPSet(
     cms.PSet( record = cms.string("SiStripBadModuleRcd"),  tag    = cms.string("") )
     )

from CalibTracker.SiStripQuality.siStripQualityStatistics_cfi import siStripQualityStatistics
process.reader = siStripQualityStatistics.clone()

process.p1 = cms.Path(process.reader)


