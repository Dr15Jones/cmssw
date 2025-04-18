import FWCore.ParameterSet.Config as cms

hltPhase2L3MuonHighPtTripletStepHitDoublets = cms.EDProducer("HitPairEDProducer",
    clusterCheck = cms.InputTag("hltTrackerClusterCheck"),
    layerPairs = cms.vuint32(0, 1),
    maxElement = cms.uint32(50000000),
    maxElementTotal = cms.uint32(50000000),
    mightGet = cms.optional.untracked.vstring,
    produceIntermediateHitDoublets = cms.bool(True),
    produceSeedingHitSets = cms.bool(False),
    seedingLayers = cms.InputTag("hltPhase2L3MuonHighPtTripletStepSeedLayers"),
    trackingRegions = cms.InputTag("hltPhase2L3MuonPixelTracksAndHighPtTripletTrackingRegions"),
    trackingRegionsSeedingLayers = cms.InputTag("")
)
