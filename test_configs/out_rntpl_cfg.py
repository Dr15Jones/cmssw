from argparse import ArgumentParser

parser = ArgumentParser(description='copy from root file to root file')
parser.add_argument("--input", help="file to read", type=str, default='file:/uscms_data/d3/ncsmith/cms-rntuple-work/data/ul17ttsemi_miniaod.root')
parser.add_argument("--numStreams", help="number of streams to use in a job", type=int, default=1)
parser.add_argument("-n", "--numThreads", help="number of threads", type=str, default="1")
parser.add_argument("--dropMeta", help="drop meta data", action="store_true")
parser.add_argument("--noSplit", help="turn off splitting", action="store_true")
parser.add_argument("--partialNoSplit", help="turn off splitting for specific data products", action="store_true")

args = parser.parse_args()

print(args)

import FWCore.ParameterSet.Config as cms


process = cms.Process("COPY")

from IOPool.Input.modules import PoolSource

process.source = PoolSource(fileNames = ['file:/uscms_data/d3/ncsmith/cms-rntuple-work/data/ul17ttsemi_miniaod.root']
                            )

#from IOPool.Output.modules import PoolOutputModule

outNameBase = 'copy'
if args.dropMeta:
    outNameBase += '_dropMeta'
if args.noSplit:
    outNameBase += '_noSplit'
if args.partialNoSplit:
    outNameBase += '_partialNoSplit'
    
outName = outNameBase+".rntpl"
    
from Configuration.EventContent.EventContent_cff import MINIAODSIMEventContent
process.o = cms.OutputModule("RNTupleOutputModule",
                             compressionAlgorithm = cms.untracked.string("LZMA"),
                             compressionLevel = cms.untracked.uint32(4),
                             fileName = cms.untracked.string(outName),
                             dropPerEventDataProductProvenance = cms.untracked.bool(args.dropMeta),
                             turnOffSplitting = cms.untracked.bool(args.noSplit)
)

if args.partialNoSplit:
    process.o.overrideDataProductSplitting = cms.untracked.VPSet(
        cms.untracked.PSet(product = cms.untracked.string("LHEEventProduct_externalLHEProducer__GEN")),
        cms.untracked.PSet(product = cms.untracked.string("edmTriggerResults_TriggerResults__HLT")),
        cms.untracked.PSet(product = cms.untracked.string("GlobalAlgBlkBXVector_gtStage2Digis__RECO"))
        )

process.e = cms.EndPath(process.o)

#process.maxEvents.input = 1
process.MessageLogger.cerr.FwkReport.reportEvery = 100
process.options.numberOfThreads = int(args.numThreads)
process.options.numberOfStreams = int(args.numStreams)

process.options.wantSummary = True

from FWCore.Services.modules import Timing
process.add_(Timing(summaryOnly = True))
