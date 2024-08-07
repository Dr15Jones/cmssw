
# Use this as follows:
#
# cmsRun FWCore/Integration/test/testFrameworkExceptionHandling_cfg.py testNumber=1
#
# with the value assigned to testNumber having a value from 1 to 16.
# That value specifies which transition to throw an exception in.
# If the value is not specified, then no exception is thrown.

import FWCore.ParameterSet.Config as cms

nStreams = 4
nRuns = 17
nLumisPerRun = 1
nEventsPerLumi = 6

nEventsPerRun = nLumisPerRun*nEventsPerLumi
nLumis = nRuns*nLumisPerRun
nEvents = nRuns*nEventsPerRun

process = cms.Process("TEST")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100

from FWCore.ParameterSet.VarParsing import VarParsing

process.TestServiceOne = cms.Service("TestServiceOne",
    verbose = cms.untracked.bool(False),
    printTimestamps = cms.untracked.bool(True)
)

process.TestServiceTwo = cms.Service("TestServiceTwo",
    verbose = cms.untracked.bool(False),
    printTimestamps = cms.untracked.bool(True)
)

options = VarParsing()

options.register("testNumber", 0,
                 VarParsing.multiplicity.singleton,
                 VarParsing.varType.int,
                 "Test number")

options.parseArguments()

process.source = cms.Source("EmptySource",
    firstRun = cms.untracked.uint32(1),
    firstLuminosityBlock = cms.untracked.uint32(1),
    firstEvent = cms.untracked.uint32(1),
    numberEventsInLuminosityBlock = cms.untracked.uint32(nEventsPerLumi),
    numberEventsInRun = cms.untracked.uint32(nEventsPerRun)
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(nEvents)
)

process.options = cms.untracked.PSet(
    numberOfThreads = cms.untracked.uint32(4),
    numberOfStreams = cms.untracked.uint32(nStreams),
    numberOfConcurrentRuns = cms.untracked.uint32(4),
    numberOfConcurrentLuminosityBlocks = cms.untracked.uint32(4)
)

process.busy1 = cms.EDProducer("BusyWaitIntProducer",ivalue = cms.int32(1), iterations = cms.uint32(10*1000*1000))

process.throwException = cms.EDProducer("ExceptionThrowingProducer",
    verbose = cms.untracked.bool(False)
)
process.doNotThrowException = cms.EDProducer("ExceptionThrowingProducer")

print('testNumber', options.testNumber)

# Below, the EventID's are selected such that it is likely that in the process
# configured by this file that more than 1 run, more than 1 lumi and more than 1 event
# (stream) will be in flight when the exception is thrown.

if options.testNumber == 1:
    process.throwException.eventIDThrowOnEvent = cms.untracked.EventID(3, 1, 5)
elif options.testNumber == 2:
    process.throwException.eventIDThrowOnGlobalBeginRun = cms.untracked.EventID(4, 0, 0)
    process.throwException.expectedGlobalBeginRun = cms.untracked.uint32(4)
    process.throwException.expectedOffsetNoGlobalEndRun = cms.untracked.uint32(1)
    process.throwException.expectedOffsetNoWriteRun = cms.untracked.uint32(1)
    process.doNotThrowException.expectedOffsetNoGlobalEndRun = cms.untracked.uint32(1)
    process.doNotThrowException.expectedOffsetNoWriteRun = cms.untracked.uint32(1)
elif options.testNumber == 3:
    process.throwException.eventIDThrowOnGlobalBeginLumi = cms.untracked.EventID(4, 1, 0)
    process.throwException.expectedGlobalBeginLumi = cms.untracked.uint32(4)
    process.throwException.expectedOffsetNoGlobalEndLumi = cms.untracked.uint32(1)
    process.throwException.expectedOffsetNoWriteLumi = cms.untracked.uint32(1)
    process.doNotThrowException.expectedOffsetNoGlobalEndLumi = cms.untracked.uint32(1)
    process.doNotThrowException.expectedOffsetNoWriteLumi = cms.untracked.uint32(1)
elif options.testNumber == 4:
    process.throwException.eventIDThrowOnGlobalEndRun = cms.untracked.EventID(3, 0, 0)
    process.throwException.expectedGlobalBeginRun = cms.untracked.uint32(3)
    process.throwException.expectedOffsetNoWriteRun = cms.untracked.uint32(1)
    process.doNotThrowException.expectedOffsetNoWriteRun = cms.untracked.uint32(1)
elif options.testNumber == 5:
    process.throwException.eventIDThrowOnGlobalEndLumi = cms.untracked.EventID(3, 1, 0)
    process.throwException.expectedGlobalBeginLumi = cms.untracked.uint32(3)
    process.throwException.expectedOffsetNoWriteLumi = cms.untracked.uint32(1)
    process.doNotThrowException.expectedOffsetNoWriteLumi = cms.untracked.uint32(1)
elif options.testNumber == 6:
    process.throwException.eventIDThrowOnStreamBeginRun = cms.untracked.EventID(4, 0, 0)
    process.throwException.expectedStreamBeginRun = cms.untracked.uint32(4)
    process.throwException.expectedOffsetNoStreamEndRun = cms.untracked.uint32(1)
    process.doNotThrowException.expectedStreamBeginRun = cms.untracked.uint32(4)
    process.doNotThrowException.expectedOffsetNoStreamEndRun = cms.untracked.uint32(1)
elif options.testNumber == 7:
    process.throwException.eventIDThrowOnStreamBeginLumi = cms.untracked.EventID(4, 1, 0)
    process.throwException.expectedStreamBeginLumi = cms.untracked.uint32(4)
    process.throwException.expectedOffsetNoStreamEndLumi = cms.untracked.uint32(1)
    process.doNotThrowException.expectedStreamBeginLumi = cms.untracked.uint32(4)
    process.doNotThrowException.expectedOffsetNoStreamEndLumi = cms.untracked.uint32(1)
elif options.testNumber == 8:
    process.throwException.eventIDThrowOnStreamEndRun = cms.untracked.EventID(3, 0, 0)
elif options.testNumber == 9:
    process.throwException.eventIDThrowOnStreamEndLumi = cms.untracked.EventID(3, 1, 0)
elif options.testNumber == 10:
    process.throwException.throwInBeginJob = cms.untracked.bool(True)
    process.throwException.expectedNEndJob = cms.untracked.uint32(0)
    process.throwException.expectedOffsetNoEndJob = cms.untracked.uint32(1)
    process.doNotThrowException.expectedNBeginStream = cms.untracked.uint32(0)
    process.doNotThrowException.expectedNBeginProcessBlock = cms.untracked.uint32(0)
    process.doNotThrowException.expectedNEndProcessBlock = cms.untracked.uint32(0)
    process.doNotThrowException.expectedNEndStream = cms.untracked.uint32(0)
    process.doNotThrowException.expectNoRunsProcessed = cms.untracked.bool(True)
    process.doNotThrowException.expectedOffsetNoEndJob = cms.untracked.uint32(1)
elif options.testNumber == 11:
    process.throwException.throwInBeginStream = cms.untracked.bool(True)
    process.throwException.expectedNBeginStream = cms.untracked.uint32(4)
    process.throwException.expectedNBeginProcessBlock = cms.untracked.uint32(0)
    process.throwException.expectedNEndProcessBlock = cms.untracked.uint32(0)
    process.throwException.expectedNEndStream = cms.untracked.uint32(3)
    process.throwException.expectNoRunsProcessed = cms.untracked.bool(True)
    process.throwException.expectedOffsetNoEndStream = cms.untracked.uint32(1)
    process.doNotThrowException.expectedNBeginStream = cms.untracked.uint32(4)
    process.doNotThrowException.expectedNBeginProcessBlock = cms.untracked.uint32(0)
    process.doNotThrowException.expectedNEndProcessBlock = cms.untracked.uint32(0)
    process.doNotThrowException.expectedNEndStream = cms.untracked.uint32(4)
    process.doNotThrowException.expectNoRunsProcessed = cms.untracked.bool(True)
    process.doNotThrowException.expectedOffsetNoEndStream = cms.untracked.uint32(1)
elif options.testNumber == 12:
    process.throwException.throwInBeginProcessBlock = cms.untracked.bool(True)
    process.throwException.expectedNEndProcessBlock = cms.untracked.uint32(0)
    process.throwException.expectNoRunsProcessed = cms.untracked.bool(True)
    process.throwException.expectedOffsetNoEndProcessBlock = cms.untracked.uint32(1)
    process.doNotThrowException.expectNoRunsProcessed = cms.untracked.bool(True)
    process.doNotThrowException.expectedOffsetNoEndProcessBlock = cms.untracked.uint32(1)
elif options.testNumber == 13:
    process.throwException.throwInEndProcessBlock = cms.untracked.bool(True)
elif options.testNumber == 14:
    process.throwException.throwInEndStream = cms.untracked.bool(True)
elif options.testNumber == 15:
    process.throwException.throwInEndJob = cms.untracked.bool(True)
# This one does not throw. It is not used in the unit test but was useful
# when manually debugging the test itself and manually debugging other things.
elif options.testNumber == 16:
    process.throwException.throwInEndJob = cms.untracked.bool(False)
else:
    print("The parameter named testNumber is out of range. An exception will not be thrown. Supported values range from 1 to 16.")
    print("The proper syntax for setting the parameter is:")
    print("")
    print ("    cmsRun FWCore/Integration/test/testFrameworkExceptionHandling_cfg.py testNumber=1")
    print("")
process.path1 = cms.Path(
    process.busy1 *
    process.throwException
)
process.path2 = cms.Path(process.doNotThrowException)
