import FWCore.ParameterSet.Config as cms
from argparse import ArgumentParser

parser = ArgumentParser(description='copy from root file to root file')
parser.add_argument("--input", help="file to read", type=str, default='file:copy_dropMeta_partialNoSplit.rntpl')
parser.add_argument("--numStreams", help="number of streams to use in a job", type=int, default=1)
parser.add_argument("-n", "--numThreads", help="number of threads", type=str, default="1")

args = parser.parse_args()

print(args)

process = cms.Process("COPY")

from IOPool.RNTupleInput.modules import RNTupleSource

process.source = RNTupleSource(fileName = args.input
                            )


from FWCore.Modules.modules import AsciiOutputModule
process.o = AsciiOutputModule(verbosity = 0)

process.e = cms.EndPath(process.o)

process.maxEvents.input = 20
#process.MessageLogger.cerr.FwkReport.reportEvery = 100
process.options.numberOfThreads = int(args.numThreads)
process.options.numberOfStreams = args.numStreams
process.options.wantSummary = True

from FWCore.Services.modules import Timing
process.add_(Timing(summaryOnly = True))

