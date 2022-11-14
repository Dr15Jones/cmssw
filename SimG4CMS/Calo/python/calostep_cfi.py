import FWCore.ParameterSet.Config as cms
from SimG4Core.Configuration.SimG4Core_cff import *

g4SimHits.Watchers = cms.VPSet(cms.PSet(
        type         = cms.string('CaloSteppingAction'),
        CaloSteppingAction = cms.PSet(
            EBSDNames       = cms.vstring('EBRY'),
            EESDNames       = cms.vstring('EFRY'),
            HCSDNames       = cms.vstring('HBS','HES','HTS'),
            AllSteps        = cms.int32(100),
            SlopeLightYield = cms.double(0.02),
            BirkC1EC        = cms.double(0.03333),
            BirkSlopeEC     = cms.double(0.253694),
            BirkCutEC       = cms.double(0.1),
            BirkC1HC        = cms.double(0.0052),
            BirkC2HC        = cms.double(0.142),
            BirkC3HC        = cms.double(1.75),
            HitCollNames    = cms.vstring('EcalHitsEB1','EcalHitsEE1',
                                          'HcalHits1'),
            EtaTable        = cms.vdouble(0.000, 0.087, 0.174, 0.261, 0.348,
                                          0.435, 0.522, 0.609, 0.696, 0.783,
                                          0.870, 0.957, 1.044, 1.131, 1.218,
                                          1.305, 1.392, 1.479, 1.566, 1.653,
                                          1.740, 1.830, 1.930, 2.043, 2.172,
                                          2.322, 2.500, 2.650, 2.868, 3.000),
            PhiBin         = cms.vdouble(5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0,
                                         5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0,
                                         5.0, 5.0, 5.0, 5.0, 5.0, 5.0,10.0,
                                        10.0,10.0,10.0,10.0,10.0,10.0,10.0,
                                        10.0),
            PhiOffset      = cms.vdouble( 0.0, 0.0, 0.0,10.0),
            EtaMin         = cms.vint32(1, 16, 29, 1),
            EtaMax         = cms.vint32(16, 29, 41, 15),
            EtaHBHE        = cms.int32(16),
            DepthHBHE      = cms.vint32(2,4),
            Depth29Max     = cms.int32(3),
            RMinHO         = cms.double(3800),
            ZHO            = cms.vdouble(0,1255,1418,3928,4100,6610),
            Eta1           = cms.untracked.vint32(1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                                  1, 1, 1, 1, 1, 1, 1, 4, 4),
            Eta15          = cms.untracked.vint32(1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                                  1, 1, 1, 2, 2, 2, 2, 4, 4),
            Eta16          = cms.untracked.vint32(1, 1, 2, 2, 2, 2, 2, 2, 2, 4,
                                                  4, 4, 4, 4, 4, 4, 4, 4, 4),
            Eta17          = cms.untracked.vint32(2, 2, 2, 2, 2, 2, 2, 2, 3, 3,
                                                  3, 3, 3, 3, 3, 3, 3, 3, 3),
            Eta18          = cms.untracked.vint32(1, 2, 2, 2, 3, 3, 3, 3, 4, 4,
                                                  4, 5, 5, 5, 5, 5, 5, 5, 5),
            Eta19          = cms.untracked.vint32(1, 1, 2, 2, 2, 3, 3, 3, 4, 4,
                                                  4, 5, 5, 5, 5, 6, 6, 6, 6),
            Eta26          = cms.untracked.vint32(1, 1, 2, 2, 3, 3, 4, 4, 5, 5,
                                                  5, 6, 6, 6, 6, 7, 7, 7, 7),
            )))