#!/usr/bin/env python
import sys

"""
Description: script used for offline validation of the Global Trigger firmware and emulator agreement                                 
(Contacts: Richard Cavanaugh, Elisa Fontanesi)                                                                                
-----------------------------------------------------------------------------------------------------                      
The parameters can be changed by adding command line arguments of the form:                                               
    runGlobalFakeInputProducer.py nevents=-1                                                                                      
The latter can be used to change parameters in crab.                                                     
Running on 3564 events (=one orbit) is recommended for test vector production for GT firmware validation.    
"""  

job = 0 #job number
njob = 1 #number of jobs
nevents = 3564 #number of events
rootout = False #whether to produce root file
dump = False #dump python
newXML = False #whether running with the new Grammar

# ----------------                                                              
# Argument parsing                                                                           
# ----------------
if len(sys.argv) == 2 and ':' in sys.argv[1]:
    argv = sys.argv[1].split(':')
else:
    argv = sys.argv[1:]

for arg in argv:
    (k, v) = map(str.strip, arg.split('='))
    if k not in globals():
        raise "Unknown argument '%s'!" % (k,)
    if isinstance(globals()[k], bool):
        globals()[k] = v.lower() in ('y', 'yes', 'true', 't', '1')
    elif isinstance(globals()[k], int):
        globals()[k] = int(v)
    else:
        globals()[k] = v

neventsPerJob = int(nevents/njob)
skip = job * neventsPerJob

if skip>4:
    skip = skip-4
    neventsPerJob = neventsPerJob+4

import FWCore.ParameterSet.Config as cms

process = cms.Process('L1TEMULATION')

process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration/StandardSequences/FrontierConditions_GlobalTag_cff')

# ------------------------------------------------------                                                                                                        
# Message Logger output:                                                                   
# Select the Message Logger output you would like to see
# ------------------------------------------------------
process.load('FWCore.MessageService.MessageLogger_cfi')
#process.load('L1Trigger/L1TGlobal/debug_messages_cfi')

# DEBUG                                                                                                                         
process.MessageLogger.debugModules = ["simGtStage2Digis"]                                                                        
process.MessageLogger.debugModules = ["l1t|Global"]                                                             
process.MessageLogger.cerr = cms.untracked.PSet(                                                                       
    threshold = cms.untracked.string('DEBUG')                                                                                     
    )

# DEBUG                                                                                
process.MessageLogger.l1t_debug = cms.untracked.PSet()                                                                           
process.MessageLogger.l1t = cms.untracked.PSet(                                                                               
    limit = cms.untracked.int32(100000),                                                          
)

# ------------                                                          
# Input source                                                               
# ------------
# Set the number of events
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(neventsPerJob)
    )

process.source = cms.Source("PoolSource",
    secondaryFileNames = cms.untracked.vstring(),
    fileNames = cms.untracked.vstring(
        # TTbar CMSSW_14X samples
        "/store/relval/CMSSW_14_0_1/RelValTTbar_14TeV/GEN-SIM-DIGI-RAW/140X_mcRun3_2024_realistic_v4_PU_AlpakaVal_AlpakaDeviceVSHost-v14/50000/34ae12a4-2d90-4d5d-b243-e949af0952ae.root",
        "/store/relval/CMSSW_14_0_1/RelValTTbar_14TeV/GEN-SIM-DIGI-RAW/140X_mcRun3_2024_realistic_v4_PU_AlpakaVal_AlpakaDeviceVSHost-v14/50000/5c08de0e-0571-4792-aa37-1b7d1915dbda.root",
        "/store/relval/CMSSW_14_0_1/RelValTTbar_14TeV/GEN-SIM-DIGI-RAW/140X_mcRun3_2024_realistic_v4_PU_AlpakaVal_AlpakaDeviceVSHost-v14/50000/1aed742b-2f48-4cc3-8758-24153c38c79b.root",
        "/store/relval/CMSSW_14_0_1/RelValTTbar_14TeV/GEN-SIM-DIGI-RAW/140X_mcRun3_2024_realistic_v4_PU_AlpakaVal_AlpakaDeviceVSHost-v14/50000/096ba83b-620d-449d-a408-ebb209b54d76.root"
	),
    skipEvents = cms.untracked.uint32(skip)
    )

process.output =cms.OutputModule("PoolOutputModule",
        outputCommands = cms.untracked.vstring('keep *'),
	fileName = cms.untracked.string('testGlobalMCInputProducer_'+repr(job)+'.root')
	)

process.options = cms.untracked.PSet(
    wantSummary = cms.bool(True)
)

# -----------------------------------------------
# Additional output definition: TTree output file
# -----------------------------------------------
process.load("CommonTools.UtilAlgos.TFileService_cfi")
process.TFileService.fileName = cms.string('l1t_histos.root')

# ----------
# Global Tag
# ----------
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '123X_mcRun3_2021_realistic_v13', '')

# ----------------                                                                                     
# Load the L1 menu                                                                   
# ----------------
process.load('L1Trigger.L1TGlobal.GlobalParameters_cff')
process.load("L1Trigger.L1TGlobal.TriggerMenu_cff")
xmlMenu="L1Menu_Collisions2024_v1_1_0.xml"
process.TriggerMenu.L1TriggerMenuFile = cms.string(xmlMenu)
process.ESPreferL1TXML = cms.ESPrefer("L1TUtmTriggerMenuESProducer","TriggerMenu")

# DEBUG: Information about names and types of algos parsed by the emulator from the menu
#process.menuDumper = cms.EDAnalyzer("L1TUtmTriggerMenuDumper") 
process.dumpMenu = cms.EDAnalyzer("L1MenuViewer")

# Flag to switch between using MC particles and injecting individual particles
useMCtoGT = True

process.dumpGT = cms.EDAnalyzer("l1t::GtInputDump",
                egInputTag    = cms.InputTag("gtInput"),
		muInputTag    = cms.InputTag("gtInput"),
		muShowerInputTag = cms.InputTag("gtInput"),
		tauInputTag   = cms.InputTag("gtInput"),
		jetInputTag   = cms.InputTag("gtInput"),
		etsumInputTag = cms.InputTag("gtInput"),
		minBx         = cms.int32(0),
		maxBx         = cms.int32(0)
		 )
process.dumpED = cms.EDAnalyzer("EventContentAnalyzer")
process.dumpES = cms.EDAnalyzer("PrintEventSetupContent")

process.mcL1GTinput = cms.EDProducer("l1t::GenToInputProducer",
                                     bxFirst = cms.int32(-2),
                                     bxLast = cms.int32(2),
				     maxMuCand = cms.int32(8),
				     maxMuShowerCand = cms.int32(8),
				     maxJetCand = cms.int32(12),
				     maxEGCand  = cms.int32(12),
				     maxTauCand = cms.int32(8),
                                     jetEtThreshold = cms.double(1),
                                     tauEtThreshold = cms.double(1),
                                     egEtThreshold  = cms.double(1),
                                     muEtThreshold  = cms.double(1),
				     emptyBxTrailer = cms.int32(5),
				     emptyBxEvt = cms.int32(neventsPerJob)
                                     )

process.mcL1GTinput.maxMuCand = cms.int32(8)
process.mcL1GTinput.maxMuShowerCand = cms.int32(8)
process.mcL1GTinput.maxJetCand = cms.int32(12)
process.mcL1GTinput.maxEGCand  = cms.int32(12)
process.mcL1GTinput.maxTauCand = cms.int32(8)

# --------------
# Fake the input
# --------------
process.fakeL1GTinput = cms.EDProducer("l1t::FakeInputProducer",

# Note: There is no error checking on these parameters...you are responsible.
                       egParams = cms.untracked.PSet(
		           egBx    = cms.untracked.vint32(-2, -1,  0,  0,  1,  2),
			   egHwPt  = cms.untracked.vint32(10, 20, 30, 61, 40, 50),
			   egHwPhi = cms.untracked.vint32(11, 21, 31, 61, 41, 51),
			   egHwEta = cms.untracked.vint32(12, 22, 32, 62, 42, 52),
			   egIso   = cms.untracked.vint32( 0,  0,  1,  1,  0,  0)
		       ),

                       muParams = cms.untracked.PSet(
		           muBx    = cms.untracked.vint32(-2, -1,  0,  0,  1,  2),
			   muHwPt  = cms.untracked.vint32(5, 20, 30, 61, 40, 50),
			   muHwPhi = cms.untracked.vint32(11, 21, 31, 61, 41, 51),
			   muHwEta = cms.untracked.vint32(12, 22, 32, 62, 42, 52),
			   muIso   = cms.untracked.vint32( 0,  0,  1,  1,  0,  0)
		       ),

                       tauParams = cms.untracked.PSet(
		           tauBx    = cms.untracked.vint32(),
			   tauHwPt  = cms.untracked.vint32(),
			   tauHwPhi = cms.untracked.vint32(),
			   tauHwEta = cms.untracked.vint32(),
			   tauIso   = cms.untracked.vint32()
		       ),

                       jetParams = cms.untracked.PSet(
		           jetBx    = cms.untracked.vint32(  0,   0,   2,   1,   1,   2),
			   jetHwPt  = cms.untracked.vint32(100, 200, 130, 170,  85, 145),
			   jetHwPhi = cms.untracked.vint32(  2,  67,  10,   3,  78,  10),
			   jetHwEta = cms.untracked.vint32(  110,  -99,  11,   0,  17,  11)
		       ),

                       etsumParams = cms.untracked.PSet(
		           etsumBx    = cms.untracked.vint32( -2, -1,   0,  1,  2),
			   etsumHwPt  = cms.untracked.vint32(  2,  1, 204,  3,  4),
			   etsumHwPhi = cms.untracked.vint32(  2,  1,  20,  3,  4)
		       )
                    )

# ------------------------
# Fill External conditions
# ------------------------
process.load('L1Trigger.L1TGlobal.simGtExtFakeProd_cfi')
process.simGtExtFakeProd.bxFirst = cms.int32(-2)
process.simGtExtFakeProd.bxLast = cms.int32(2)
process.simGtExtFakeProd.setBptxAND   = cms.bool(True)
process.simGtExtFakeProd.setBptxPlus  = cms.bool(True)
process.simGtExtFakeProd.setBptxMinus = cms.bool(True)
process.simGtExtFakeProd.setBptxOR    = cms.bool(True)

# ----------------------------
# Run the Stage 2 uGT emulator
# ----------------------------
process.load('L1Trigger.L1TGlobal.simGtStage2Digis_cfi')
process.simGtStage2Digis.useMuonShowers = cms.bool(True)
process.simGtStage2Digis.PrescaleSet = cms.uint32(1)
process.simGtStage2Digis.ExtInputTag = cms.InputTag("simGtExtFakeProd")
process.simGtStage2Digis.MuonInputTag = cms.InputTag("gtInput")
process.simGtStage2Digis.MuonShowerInputTag  = cms.InputTag("gtInput")
process.simGtStage2Digis.EGammaInputTag = cms.InputTag("gtInput")
process.simGtStage2Digis.TauInputTag = cms.InputTag("gtInput")
process.simGtStage2Digis.JetInputTag = cms.InputTag("gtInput")
process.simGtStage2Digis.EtSumInputTag = cms.InputTag("gtInput")
process.simGtStage2Digis.EmulateBxInEvent = cms.int32(1)
#process.simGtStage2Digis.Verbosity = cms.untracked.int32(1)
#process.simGtStage2Digis.AlgorithmTriggersUnprescaled = cms.bool(True)
#process.simGtStage2Digis.AlgorithmTriggersUnmasked = cms.bool(True)

process.dumpGTRecord = cms.EDAnalyzer("l1t::GtRecordDump",
                egInputTag    = cms.InputTag("gtInput"),
		muInputTag    = cms.InputTag("gtInput"),
		muShowerInputTag = cms.InputTag("gtInput"),
		tauInputTag   = cms.InputTag("gtInput"),
		jetInputTag   = cms.InputTag("gtInput"),
		etsumInputTag = cms.InputTag("gtInput"),
		uGtAlgInputTag = cms.InputTag("simGtStage2Digis"),
		uGtExtInputTag = cms.InputTag("simGtExtFakeProd"),
		uGtObjectMapInputTag = cms.InputTag("simGtStage2Digis"),
		bxOffset       = cms.int32(skip),
		minBx          = cms.int32(-2),
		maxBx          = cms.int32(2),
		minBxVec       = cms.int32(0),
		maxBxVec       = cms.int32(0),
		dumpGTRecord   = cms.bool(True),
		dumpGTObjectMap= cms.bool(False),
                dumpTrigResults= cms.bool(False),
		dumpVectors    = cms.bool(True),
		tvFileName     = cms.string( ("TestVector_ttBar_%03d.txt") % job ),
		tvVersion      = cms.int32(3),
                ReadPrescalesFromFile = cms.bool(True),
                psFileName     = cms.string( "prescale_L1TGlobal.csv" ),
                psColumn       = cms.int32(1),
		unprescaleL1Algos = cms.bool(False),
                unmaskL1Algos     = cms.bool(False)
		 )

process.load("L1Trigger.GlobalTriggerAnalyzer.l1GtTrigReport_cfi")
process.l1GtTrigReport.L1GtRecordInputTag = "simGtStage2Digis"
process.l1GtTrigReport.PrintVerbosity = 2
process.report = cms.Path(process.l1GtTrigReport)

process.MessageLogger.debugModules = ["MuCondition"]                                                             

if useMCtoGT:
    process.gtInput = process.mcL1GTinput.clone()
else:
    process.gtInput = process.fakeL1GTinput.clone()

# -------------------------
# Setup Digi to Raw to Digi
# -------------------------
process.load('EventFilter.L1TRawToDigi.gtStage2Raw_cfi')
process.gtStage2Raw.GtInputTag = cms.InputTag("simGtStage2Digis")
process.gtStage2Raw.ExtInputTag = cms.InputTag("simGtExtFakeProd")
process.gtStage2Raw.EGammaInputTag = cms.InputTag("gtInput")
process.gtStage2Raw.TauInputTag = cms.InputTag("gtInput")
process.gtStage2Raw.JetInputTag = cms.InputTag("gtInput")
process.gtStage2Raw.EtSumInputTag = cms.InputTag("gtInput")
process.gtStage2Raw.MuonInputTag = cms.InputTag("gtInput")
process.gtStage2Raw.MuonShowerInputTag = cms.InputTag("gtInput")

process.load('EventFilter.L1TRawToDigi.gtStage2Digis_cfi')
process.newGtStage2Digis = process.gtStage2Digis.clone()
process.newGtStage2Digis.InputLabel = cms.InputTag('gtStage2Raw')
# DEBUG 
#process.newGtStage2Digis.debug = cms.untracked.bool(True) 

process.dumpRaw = cms.EDAnalyzer(
    "DumpFEDRawDataProduct",
    label = cms.untracked.string("gtStage2Raw"),
    feds = cms.untracked.vint32 ( 1404 ),
    dumpPayload = cms.untracked.bool ( True )
)

process.newDumpGTRecord = cms.EDAnalyzer("l1t::GtRecordDump",
                egInputTag    = cms.InputTag("newGtStage2Digis","EGamma"),
                muInputTag    = cms.InputTag("newGtStage2Digis","Muon"),
                muShowerInputTag = cms.InputTag("newGtStage2Digis", "MuonShower"),
		tauInputTag   = cms.InputTag("newGtStage2Digis","Tau"),
		jetInputTag   = cms.InputTag("newGtStage2Digis","Jet"),
		etsumInputTag = cms.InputTag("newGtStage2Digis","EtSum"),
		uGtAlgInputTag = cms.InputTag("newGtStage2Digis"),
		uGtExtInputTag = cms.InputTag("newGtStage2Digis"),
		uGtObjectMapInputTag = cms.InputTag("simGtStage2Digis"),
		bxOffset       = cms.int32(skip),
		minBx          = cms.int32(0),
		maxBx          = cms.int32(0),
		minBxVec       = cms.int32(0),
		maxBxVec       = cms.int32(0),
		dumpGTRecord   = cms.bool(True),
		dumpGTObjectMap= cms.bool(True),
                dumpTrigResults= cms.bool(False),
		dumpVectors    = cms.bool(False),
		tvFileName     = cms.string( ("TestVector_%03d.txt") % job ),
                ReadPrescalesFromFile = cms.bool(False),
                psFileName     = cms.string( "prescale_L1TGlobal.csv" ),
                psColumn       = cms.int32(1)
		 )

# -----------
# GT analyzer
# -----------
process.l1tGlobalAnalyzer = cms.EDAnalyzer('L1TGlobalAnalyzer',
    doText = cms.untracked.bool(False),
    gmuToken = cms.InputTag("None"),
    dmxEGToken = cms.InputTag("None"),
    dmxTauToken = cms.InputTag("None"),
    dmxJetToken = cms.InputTag("None"),
    dmxEtSumToken = cms.InputTag("None"),
    muToken = cms.InputTag("gtInput"),
    egToken = cms.InputTag("gtInput"),
    tauToken = cms.InputTag("gtInput"),
    jetToken = cms.InputTag("gtInput"),
    etSumToken = cms.InputTag("gtInput"),
    gtAlgToken = cms.InputTag("simGtStage2Digis"),
    emulDxAlgToken = cms.InputTag("None"),
    emulGtAlgToken = cms.InputTag("simGtStage2Digis")
)


# ------------------                                                                                                    
# Process definition                                                               
# ------------------ 
process.p1 = cms.Path(

## Generate input, emulate, dump results
    process.dumpMenu
    *process.gtInput
#    *process.dumpGT
    *process.simGtExtFakeProd
    *process.simGtStage2Digis
    *process.dumpGTRecord

## Sequence for packing and unpacking uGT data
#    +process.gtStage2Raw
#    +process.dumpRaw
#    +process.newGtStage2Digis
#    +process.newDumpGTRecord

## Analysis/Dumping
    *process.l1tGlobalAnalyzer
#    *process.menuDumper # DEBUG -> to activate the menuDumper
#    *process.debug
#    *process.dumpED
#    *process.dumpES
    )

# -------------------                                                                  
# Schedule definition                                                                                                     
# -------------------
process.schedule = cms.Schedule(
    process.p1
    )
#process.schedule.append(process.report)
if rootout:
    process.outpath = cms.EndPath(process.output)
    process.schedule.append(process.outpath)

# Final summary of the efficiency
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))

# Options for multithreading
#process.options.numberOfThreads = cms.untracked.uint32( 2 )
#process.options.numberOfStreams = cms.untracked.uint32( 0 )

if dump:
    outfile = open('dump_runGlobalFakeInputProducer_'+repr(job)+'.py','w')
    print(process.dumpPython(), file=outfile)
    outfile.close()
