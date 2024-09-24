import FWCore.ParameterSet.Config as cms
from argparse import ArgumentParser

parser = ArgumentParser(description='copy from root file to root file')
parser.add_argument("--input", help="file to read", type=str, default='file:/uscms_data/d3/ncsmith/cms-rntuple-work/data/ul17ttsemi_miniaod.root')
parser.add_argument("--numStreams", help="number of streams to use in a job", type=int, default=1)
parser.add_argument("-n", "--numThreads", help="number of threads", type=str, default="1")
parser.add_argument("--fullSplit", help="force a full split", action="store_true")
parser.add_argument("--dropMeta", help="drop meta data", action="store_true")

args = parser.parse_args()

print(args)

process = cms.Process("COPY")

from IOPool.Input.modules import PoolSource

process.source = PoolSource(fileNames = [args.input]
                            )

from IOPool.Output.modules import PoolOutputModule


outFileName = 'copy_likeCMSDriver'
if args.fullSplit:
    outFileName = 'copy_fullSplit'
    if args.dropMeta:
        outFileName +='_dropMeta'
outFileName +='.root'
from Configuration.EventContent.EventContent_cff import MINIAODSIMEventContent
process.o = cms.OutputModule("PoolOutputModule", MINIAODSIMEventContent,
                             fileName = cms.untracked.string(outFileName),
                             fastCloning = cms.untracked.bool(False),
                             overrideInputFileSplitLevels = cms.untracked.bool(args.fullSplit)
)

process.o.outputCommands = ['keep *']

if args.dropMeta:
    process.o.dropMetaData = cms.untracked.string('ALL')

from PhysicsTools.PatAlgos.slimming.miniAOD_tools import miniAOD_customizeOutput
if not args.fullSplit:
    miniAOD_customizeOutput(process.o)

process.e = cms.EndPath(process.o)

##process.maxEvents.input = 1
process.MessageLogger.cerr.FwkReport.reportEvery = 100
process.options.numberOfThreads = int(args.numThreads)
process.options.numberOfStreams = args.numStreams
process.options.wantSummary = True

from FWCore.Services.modules import Timing
process.add_(Timing(summaryOnly = True))

