///
/// \class l1t::GtRecordDump.cc
///
/// Description: Dump/Analyze Input Collections for GT.
///
/// Implementation:
///    Based off of Michael Mulhearn's YellowParamTester
///
/// \author: Brian Winer Ohio State
///

//
//  This simple module simply retreives the YellowParams object from the event
//  setup, and sends its payload as an INFO message, for debugging purposes.
//

#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/Framework/interface/one/EDAnalyzer.h"
//#include "FWCore/ParameterSet/interface/InputTag.h"

// system include files
#include <fstream>
#include <iomanip>
#include <memory>

// user include files
//   base class
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"

#include "DataFormats/L1Trigger/interface/EGamma.h"
#include "DataFormats/L1Trigger/interface/Muon.h"
#include "DataFormats/L1Trigger/interface/MuonShower.h"
#include "DataFormats/L1Trigger/interface/Tau.h"
#include "DataFormats/L1Trigger/interface/Jet.h"
#include "DataFormats/L1Trigger/interface/EtSum.h"

#include "DataFormats/L1TGlobal/interface/GlobalAlgBlk.h"
#include "DataFormats/L1TGlobal/interface/GlobalExtBlk.h"
#include "DataFormats/L1TGlobal/interface/GlobalObject.h"

#include "DataFormats/L1TGlobal/interface/GlobalObjectMapFwd.h"
#include "DataFormats/L1TGlobal/interface/GlobalObjectMap.h"
#include "DataFormats/L1TGlobal/interface/GlobalObjectMapRecord.h"

#include "L1Trigger/L1TGlobal/interface/L1TGlobalUtil.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/MessageLogger/interface/MessageDrop.h"

using namespace edm;
using namespace std;

namespace l1t {

  // class declaration
  class GtRecordDump : public edm::one::EDAnalyzer<edm::one::WatchRuns> {
  public:
    explicit GtRecordDump(const edm::ParameterSet&);
    ~GtRecordDump() override {}
    void beginRun(edm::Run const&, edm::EventSetup const&) override {}
    void analyze(const edm::Event&, const edm::EventSetup&) override;
    void endRun(edm::Run const&, edm::EventSetup const&) override;

    InputTag uGtAlgInputTag;
    InputTag uGtExtInputTag;
    EDGetToken egToken;
    EDGetToken muToken;
    EDGetToken muShowerToken;
    EDGetToken tauToken;
    EDGetToken jetToken;
    EDGetToken etsumToken;
    EDGetToken uGtAlgToken;
    EDGetToken uGtExtToken;
    EDGetToken uGtObjectMapToken;

    void dumpTestVectors(int bx,
                         std::ofstream& myCout,
                         Handle<BXVector<l1t::Muon>> muons,
                         Handle<BXVector<l1t::MuonShower>> muonShowers,
                         Handle<BXVector<l1t::EGamma>> egammas,
                         Handle<BXVector<l1t::Tau>> taus,
                         Handle<BXVector<l1t::Jet>> jets,
                         Handle<BXVector<l1t::EtSum>> etsums,
                         Handle<BXVector<GlobalAlgBlk>> uGtAlg,
                         Handle<BXVector<GlobalExtBlk>> uGtExt);

    cms_uint64_t formatMuon(std::vector<l1t::Muon>::const_iterator mu, int muShowerBit);
    cms_uint64_t formatNonExistantMuon(int muShowerBit);
    unsigned int formatEG(std::vector<l1t::EGamma>::const_iterator eg);
    unsigned int formatTau(std::vector<l1t::Tau>::const_iterator tau);
    unsigned int formatJet(std::vector<l1t::Jet>::const_iterator jet);
    unsigned int formatMissET(std::vector<l1t::EtSum>::const_iterator etSum);
    unsigned int formatTotalET(std::vector<l1t::EtSum>::const_iterator etSum);
    unsigned int formatTowerCounts(std::vector<l1t::EtSum>::const_iterator etSum);
    unsigned int formatAsym(std::vector<l1t::EtSum>::const_iterator etSum);
    unsigned int formatHMB(std::vector<l1t::EtSum>::const_iterator etSum);
    std::pair<unsigned int, unsigned int> formatCentrality(std::vector<l1t::EtSum>::const_iterator etSum);
    std::map<std::string, std::vector<int>, std::less<>> m_algoSummary;

    unsigned int m_absBx;
    int m_bxOffset;

    std::ofstream m_testVectorFile;

    bool m_dumpTestVectors;
    bool m_dumpGTRecord;
    bool m_dumpObjectMap;
    bool m_dumpTriggerResults;
    int m_minBx;
    int m_maxBx;
    int m_minBxVectors;
    int m_maxBxVectors;

    std::unique_ptr<L1TGlobalUtil> m_gtUtil;

  private:
    int m_tvVersion;
  };

  GtRecordDump::GtRecordDump(const edm::ParameterSet& iConfig) {
    uGtAlgInputTag = iConfig.getParameter<InputTag>("uGtAlgInputTag");
    uGtExtInputTag = iConfig.getParameter<InputTag>("uGtExtInputTag");
    egToken = consumes<BXVector<l1t::EGamma>>(iConfig.getParameter<InputTag>("egInputTag"));
    muToken = consumes<BXVector<l1t::Muon>>(iConfig.getParameter<InputTag>("muInputTag"));
    muShowerToken = consumes<BXVector<l1t::MuonShower>>(iConfig.getParameter<InputTag>("muShowerInputTag"));
    tauToken = consumes<BXVector<l1t::Tau>>(iConfig.getParameter<InputTag>("tauInputTag"));
    jetToken = consumes<BXVector<l1t::Jet>>(iConfig.getParameter<InputTag>("jetInputTag"));
    etsumToken = consumes<BXVector<l1t::EtSum>>(iConfig.getParameter<InputTag>("etsumInputTag"));
    uGtAlgToken = consumes<BXVector<GlobalAlgBlk>>(uGtAlgInputTag);
    uGtExtToken = consumes<BXVector<GlobalExtBlk>>(uGtExtInputTag);
    uGtObjectMapToken = consumes<GlobalObjectMapRecord>(iConfig.getParameter<InputTag>("uGtObjectMapInputTag"));

    m_minBx = iConfig.getParameter<int>("minBx");
    m_maxBx = iConfig.getParameter<int>("maxBx");
    m_dumpGTRecord = iConfig.getParameter<bool>("dumpGTRecord");
    m_dumpObjectMap = iConfig.getParameter<bool>("dumpGTObjectMap");
    m_dumpTriggerResults = iConfig.getParameter<bool>("dumpTrigResults");

    m_minBxVectors = iConfig.getParameter<int>("minBxVec");
    m_maxBxVectors = iConfig.getParameter<int>("maxBxVec");
    m_dumpTestVectors = iConfig.getParameter<bool>("dumpVectors");
    std::string fileName = iConfig.getParameter<std::string>("tvFileName");
    if (m_dumpTestVectors)
      m_testVectorFile.open(fileName.c_str());
    m_tvVersion = iConfig.getParameter<int>("tvVersion");

    m_bxOffset = iConfig.getParameter<int>("bxOffset");

    m_absBx = 0;
    m_absBx += m_bxOffset;

    std::string preScaleFileName = iConfig.getParameter<std::string>("psFileName");
    unsigned int preScColumn = iConfig.getParameter<int>("psColumn");

    m_gtUtil = std::make_unique<L1TGlobalUtil>(
        iConfig, consumesCollector(), *this, uGtAlgInputTag, uGtExtInputTag, l1t::UseEventSetupIn::Event);
    m_gtUtil->OverridePrescalesAndMasks(preScaleFileName, preScColumn);
  }

  // loop over events
  void GtRecordDump::analyze(const edm::Event& iEvent, const edm::EventSetup& evSetup) {
    //inputs
    Handle<BXVector<l1t::EGamma>> egammas;
    iEvent.getByToken(egToken, egammas);

    Handle<BXVector<l1t::Muon>> muons;
    iEvent.getByToken(muToken, muons);

    Handle<BXVector<l1t::MuonShower>> muonShowers;
    iEvent.getByToken(muShowerToken, muonShowers);

    Handle<BXVector<l1t::Tau>> taus;
    iEvent.getByToken(tauToken, taus);

    Handle<BXVector<l1t::Jet>> jets;
    iEvent.getByToken(jetToken, jets);

    Handle<BXVector<l1t::EtSum>> etsums;
    iEvent.getByToken(etsumToken, etsums);

    Handle<BXVector<GlobalAlgBlk>> uGtAlg;
    iEvent.getByToken(uGtAlgToken, uGtAlg);

    Handle<BXVector<GlobalExtBlk>> uGtExt;
    iEvent.getByToken(uGtExtToken, uGtExt);

    Handle<GlobalObjectMapRecord> gtObjectMapRecord;
    iEvent.getByToken(uGtObjectMapToken, gtObjectMapRecord);

    //Fill the L1 result maps
    m_gtUtil->retrieveL1(iEvent, evSetup, uGtAlgToken);

    LogDebug("GtRecordDump") << "retrieved L1 data " << endl;

    // grab the map for the final decisions
    const auto& initialDecisions = m_gtUtil->decisionsInitial();
    const auto& intermDecisions = m_gtUtil->decisionsInterm();
    const auto& finalDecisions = m_gtUtil->decisionsFinal();
    const auto& prescales = m_gtUtil->prescales();
    const auto& masks = m_gtUtil->masks();

    LogDebug("GtRecordDump") << "retrieved all event vectors " << endl;

    // Dump the results
    if (m_dumpTriggerResults) {
      cout << "    Bit                  Algorithm Name                                      Init    aBXM  Final   PS "
              "Factor     Num Bx Masked    "
           << endl;
      cout << "========================================================================================================"
              "======================="
           << endl;
    }
    for (unsigned int i = 0; i < initialDecisions.size(); i++) {
      // get the name and trigger result
      string name{(initialDecisions.at(i)).first};
      bool resultInit = (initialDecisions.at(i)).second;

      //  put together our map of algorithms and counts across events
      if (m_algoSummary.count(name) == 0) {
        std::vector<int> tst;
        tst.resize(3);
        m_algoSummary[name] = tst;
      }
      if (resultInit)
        (m_algoSummary.find(name)->second).at(0) += 1;

      // get prescaled and final results (need some error checking here)
      bool resultInterm = (intermDecisions.at(i)).second;
      if (resultInterm)
        (m_algoSummary.find(name)->second).at(1) += 1;
      bool resultFin = (finalDecisions.at(i)).second;
      if (resultFin)
        (m_algoSummary.find(name)->second).at(2) += 1;

      // get the prescale and mask (needs some error checking here)
      double prescale = (prescales.at(i)).second;
      std::vector<int> mask = (masks.at(i)).second;

      if (m_dumpTriggerResults && name != "NULL")
        cout << std::dec << setfill(' ') << "   " << setw(5) << i << "   " << setw(60) << name.c_str() << "   "
             << setw(7) << resultInit << setw(7) << resultInterm << setw(7) << resultFin << setw(10) << prescale
             << setw(11) << mask.size() << endl;
    }
    bool finOR = m_gtUtil->getFinalOR();
    if (m_dumpTriggerResults) {
      cout << "                                                                                    FinalOR = " << finOR
           << endl;
      cout << "========================================================================================================"
              "========================"
           << endl;
    }

    if (m_dumpObjectMap) {
      if (!gtObjectMapRecord.isValid()) {
        edm::LogWarning("GtRecordDump")
            << " Warning: GlobalObjectMapRecord requested in configuration, but not found in the event." << std::endl;
      } else {
        const std::vector<GlobalObjectMap>& objMaps = gtObjectMapRecord->gtObjectMap();
        for (size_t imap = 0; imap < objMaps.size(); imap++) {
          GlobalObjectMap oMap = objMaps.at(imap);

          int bit = oMap.algoBitNumber();  //  same as bit from L1T Menu
          int mapDecision = oMap.algoGtlResult();

          // Check Object Map Result Agrees with GlobalAlgBlk result
          if (mapDecision != (finalDecisions.at(bit)).second) {
            std::cout << "WARNING: GlobalAlgBlk and ObjectMap Disagree on result for bit " << bit
                      << " Alg: " << oMap.algoName() << std::endl;
          }

          // dump only if we have a positive trigger
          if (mapDecision != 0) {
            // Header info
            std::cout
                << " -------------------------------------------------------------------------------------------- \n"
                << " Bit " << setw(3) << bit << " Decision " << setw(2) << mapDecision << " Alg Name " << setw(40)
                << oMap.algoName() << std::endl;

            // Combination
            const std::vector<GlobalLogicParser::OperandToken>& opTokenVecObjMap = oMap.operandTokenVector();
            const std::vector<L1TObjectTypeInCond>& condObjTypeVec = oMap.objectTypeVector();

            for (size_t iCond = 0; iCond < opTokenVecObjMap.size(); iCond++) {
              std::cout << "       " << iCond << ") Condition Token: " << opTokenVecObjMap[iCond].tokenName
                        << "  Types: ";
              auto const& condObjType = condObjTypeVec[iCond];
              for (size_t iCondType = 0; iCondType < condObjType.size(); iCondType++) {
                std::cout << condObjType.at(iCondType) << "  ";
              }
              std::cout << std::endl;

              const CombinationsWithBxInCond* condComb = oMap.getCombinationsInCond(iCond);
              std::cout << "            Combinations in Condition [" << condComb->size() << "] : ";
              for (auto const& itComb : *condComb) {
                // loop over objects in a combination for a given condition
                //
                unsigned int iType = 0;
                std::cout << "(";
                for (auto const& [bxIdx, objIdx] : itComb) {
                  // loop over types for the object in a combination.  This object might have more then one type (i.e. mu-eg)
                  //

                  //        		     for (size_t iType =0; iType < condObjType.size(); iType++) {

                  // get object type and push indices on the list
                  //
                  //const l1t::GlobalObject objTypeVal = condObjType.at(iType);

                  std::cout << bxIdx << ":" << objIdx;
                  //std::cout <<objTypeVal << "@" << bxIdx << ":" << objIdx;
                  if (iType < condObjType.size() - 1)
                    std::cout << ",";
                  //std::cout
                  //<< "\tAdd object of type " << objTypeVal << " and bx:index " << bxIdx << ":" << objIdx << " to the seed list."
                  //<< std::endl;

                  //		             } // end loop over objs in combination
                  iType++;

                }  //end loop over objects for a condition
                std::cout << ")  ";
              }
              std::cout << std::endl;
            }
          }  //end if alg fired
        }  //end loop over maps
      }  //end if valid record
    }  //end if dump maps

    if (m_dumpGTRecord) {
      cout << " -----------------------------------------------------  " << endl;
      cout << " *********** Run " << std::dec << iEvent.id().run() << " Event " << iEvent.id().event()
           << " **************  " << endl;
      cout << " ----------------------------------------------------- " << endl;

      //Loop over BX
      for (int i = m_minBx; i <= m_maxBx; ++i) {
        cout << " ========= Rel BX = " << std::dec << i << " ======  Total BX = " << m_absBx << "   ==========" << endl;

        //Loop over EGamma
        int nObj = 0;
        cout << " ------ EGammas -------- " << endl;
        if (egammas.isValid()) {
          if (i >= egammas->getFirstBX() && i <= egammas->getLastBX()) {
            for (std::vector<l1t::EGamma>::const_iterator eg = egammas->begin(i); eg != egammas->end(i); ++eg) {
              cout << "  " << std::dec << std::setw(2) << std::setfill(' ') << nObj << std::setfill('0') << ")";
              cout << "   Pt " << std::dec << std::setw(3) << eg->hwPt() << " (0x" << std::hex << std::setw(3)
                   << std::setfill('0') << eg->hwPt() << ")";
              cout << "   Eta " << std::dec << std::setw(3) << eg->hwEta() << " (0x" << std::hex << std::setw(2)
                   << std::setfill('0') << (eg->hwEta() & 0xff) << ")";
              cout << "   Phi " << std::dec << std::setw(3) << eg->hwPhi() << " (0x" << std::hex << std::setw(2)
                   << std::setfill('0') << eg->hwPhi() << ")";
              cout << "   Iso " << std::dec << std::setw(1) << eg->hwIso();
              cout << "   Qual " << std::dec << std::setw(1) << eg->hwQual();
              cout << endl;
              nObj++;
            }
          } else {
            cout << "No EG stored for this bx " << i << endl;
          }
        } else {
          cout << "No EG Data in this event " << endl;
        }

        //Loop over Muons
        nObj = 0;
        cout << " ------ Muons --------" << endl;
        if (muons.isValid()) {
          if (i >= muons->getFirstBX() && i <= muons->getLastBX()) {
            for (std::vector<l1t::Muon>::const_iterator mu = muons->begin(i); mu != muons->end(i); ++mu) {
              cout << "  " << std::dec << std::setw(2) << std::setfill(' ') << nObj << std::setfill('0') << ")";
              cout << "   Pt " << std::dec << std::setw(3) << mu->hwPt() << " (0x" << std::hex << std::setw(3)
                   << std::setfill('0') << mu->hwPt() << ")";
              cout << "   EtaAtVtx " << std::dec << std::setw(3) << mu->hwEtaAtVtx() << " (0x" << std::hex
                   << std::setw(3) << std::setfill('0') << (mu->hwEtaAtVtx() & 0x1ff) << ")";
              cout << "   Eta " << std::dec << std::setw(3) << mu->hwEta() << " (0x" << std::hex << std::setw(3)
                   << std::setfill('0') << (mu->hwEta() & 0x1ff) << ")";
              cout << "   PhiAtVtx " << std::dec << std::setw(3) << mu->hwPhiAtVtx() << " (0x" << std::hex
                   << std::setw(3) << std::setfill('0') << mu->hwPhiAtVtx() << ")";
              cout << "   Phi " << std::dec << std::setw(3) << mu->hwPhi() << " (0x" << std::hex << std::setw(3)
                   << std::setfill('0') << mu->hwPhi() << ")";
              cout << "   Iso " << std::dec << std::setw(1) << mu->hwIso();
              cout << "   Qual " << std::dec << std::setw(1) << mu->hwQual();
              cout << "   Chrg " << std::dec << std::setw(1) << mu->hwCharge();
              cout << endl;
              nObj++;
            }
          } else {
            cout << "No Muons stored for this bx " << i << endl;
          }
        } else {
          cout << "No Muon Data in this event " << endl;
        }

        //Loop over Muon Showers
        nObj = 0;
        cout << " ------ Muons Showers --------" << endl;
        if (muonShowers.isValid()) {
          std::cout << "========= MuonShower BX index = " << i << "; min BX = " << m_minBx << "; max BX = " << m_maxBx
                    << std::endl;
          if (i >= muonShowers->getFirstBX() && i <= muonShowers->getLastBX()) {
            for (std::vector<l1t::MuonShower>::const_iterator muShower = muonShowers->begin(i);
                 muShower != muonShowers->end(i);
                 ++muShower) {
              cout << "  " << std::dec << std::setw(2) << std::setfill(' ') << nObj << std::setfill('0') << ")";
              cout << "   MUS0 " << std::dec << std::setw(1) << muShower->isOneNominalInTime();
              cout << ";  MUS1 " << std::dec << std::setw(1) << muShower->isOneTightInTime();
              cout << ";  MUS2 " << std::dec << std::setw(1) << muShower->isTwoLooseDiffSectorsInTime();
              cout << ";  MUSOOT0 " << std::dec << std::setw(1) << muShower->musOutOfTime0();
              cout << ";  MUSOOT1 " << std::dec << std::setw(1) << muShower->musOutOfTime1();
              cout << endl;
              nObj++;
            }
          } else {
            cout << "No MuonShowers stored for this bx " << i << endl;
          }
        } else {
          cout << "No MuonShower Data in this event " << endl;
        }

        //Loop over Taus
        nObj = 0;
        cout << " ------ Taus ----------" << endl;
        if (taus.isValid()) {
          if (i >= taus->getFirstBX() && i <= taus->getLastBX()) {
            for (std::vector<l1t::Tau>::const_iterator tau = taus->begin(i); tau != taus->end(i); ++tau) {
              cout << "  " << std::dec << std::setw(2) << std::setfill(' ') << nObj << std::setfill('0') << ")";
              cout << "   Pt " << std::dec << std::setw(3) << tau->hwPt() << " (0x" << std::hex << std::setw(3)
                   << std::setfill('0') << tau->hwPt() << ")";
              cout << "   Eta " << std::dec << std::setw(3) << tau->hwEta() << " (0x" << std::hex << std::setw(2)
                   << std::setfill('0') << (tau->hwEta() & 0xff) << ")";
              cout << "   Phi " << std::dec << std::setw(3) << tau->hwPhi() << " (0x" << std::hex << std::setw(2)
                   << std::setfill('0') << tau->hwPhi() << ")";
              cout << "   Iso " << std::dec << std::setw(1) << tau->hwIso();
              cout << "   Qual " << std::dec << std::setw(1) << tau->hwQual();
              cout << endl;
              nObj++;
            }
          } else {
            cout << "No Taus stored for this bx " << i << endl;
          }
        } else {
          cout << "No Tau Data in this event " << endl;
        }

        //Loop over Jets
        nObj = 0;
        cout << " ------ Jets ----------" << endl;
        if (jets.isValid()) {
          if (i >= jets->getFirstBX() && i <= jets->getLastBX()) {
            for (std::vector<l1t::Jet>::const_iterator jet = jets->begin(i); jet != jets->end(i); ++jet) {
              cout << "  " << std::dec << std::setw(2) << std::setfill(' ') << nObj << std::setfill('0') << ")";
              cout << "   Pt " << std::dec << std::setw(3) << jet->hwPt() << " (0x" << std::hex << std::setw(3)
                   << std::setfill('0') << jet->hwPt() << ")";
              cout << "   Eta " << std::dec << std::setw(3) << jet->hwEta() << " (0x" << std::hex << std::setw(2)
                   << std::setfill('0') << (jet->hwEta() & 0xff) << ")";
              cout << "   Phi " << std::dec << std::setw(3) << jet->hwPhi() << " (0x" << std::hex << std::setw(2)
                   << std::setfill('0') << jet->hwPhi() << ")";
              cout << "   Qual " << std::dec << std::setw(1) << jet->hwQual();
              cout << endl;
              nObj++;
            }
          } else {
            cout << "No Jets stored for this bx " << i << endl;
          }
        } else {
          cout << "No jet Data in this event " << endl;
        }

        //Dump Content
        cout << " ------ EtSums ----------" << endl;
        if (etsums.isValid()) {
          if (i >= etsums->getFirstBX() && i <= etsums->getLastBX()) {
            for (std::vector<l1t::EtSum>::const_iterator etsum = etsums->begin(i); etsum != etsums->end(i); ++etsum) {
              switch (etsum->getType()) {
                case l1t::EtSum::EtSumType::kMissingEt:
                  cout << " ETM:  ";
                  break;
                case l1t::EtSum::EtSumType::kMissingEtHF:
                  cout << " ETMHF:";
                  break;
                case l1t::EtSum::EtSumType::kMissingHtHF:
                  cout << " HTMHF:";
                  break;
                case l1t::EtSum::EtSumType::kMissingHt:
                  cout << " HTM:  ";
                  break;
                case l1t::EtSum::EtSumType::kTotalEt:
                  cout << " ETT:  ";
                  break;
                case l1t::EtSum::EtSumType::kTotalEtEm:
                  cout << " ETTem:";
                  break;
                case l1t::EtSum::EtSumType::kTotalHt:
                  cout << " HTT:  ";
                  break;
                case l1t::EtSum::EtSumType::kTowerCount:
                  cout << " TowerCounts:  ";
                  break;
                case l1t::EtSum::EtSumType::kAsymEt:
                  cout << " AsymEt:  ";
                  break;
                case l1t::EtSum::EtSumType::kAsymHt:
                  cout << " AsymHt:  ";
                  break;
                case l1t::EtSum::EtSumType::kAsymEtHF:
                  cout << " AsymEtHF:  ";
                  break;
                case l1t::EtSum::EtSumType::kAsymHtHF:
                  cout << " AsymHtHF:  ";
                  break;
                case l1t::EtSum::EtSumType::kMinBiasHFP0:
                  cout << " HFP0: ";
                  break;
                case l1t::EtSum::EtSumType::kMinBiasHFM0:
                  cout << " HFM0: ";
                  break;
                case l1t::EtSum::EtSumType::kMinBiasHFP1:
                  cout << " HFP1: ";
                  break;
                case l1t::EtSum::EtSumType::kMinBiasHFM1:
                  cout << " HFM1: ";
                  break;
                case l1t::EtSum::EtSumType::kCentrality:
                  cout << " Centrality: ";
                  break;
                default:
                  cout << " Unknown: ";
                  break;
              }
              cout << " Et " << std::dec << std::setw(3) << etsum->hwPt() << " (0x" << std::hex << std::setw(3)
                   << std::setfill('0') << etsum->hwPt() << ")";
              if (etsum->getType() == l1t::EtSum::EtSumType::kMissingEt ||
                  etsum->getType() == l1t::EtSum::EtSumType::kMissingHt ||
                  etsum->getType() == l1t::EtSum::EtSumType::kMissingEtHF ||
                  etsum->getType() == l1t::EtSum::EtSumType::kMissingHtHF)
                cout << " Phi " << std::dec << std::setw(3) << etsum->hwPhi() << " (0x" << std::hex << std::setw(2)
                     << std::setfill('0') << etsum->hwPhi() << ")";
              cout << endl;
            }
          } else {
            cout << "No EtSums stored for this bx " << i << endl;
          }
        } else {
          cout << "No EtSum Data in this event " << endl;
        }

        // Dump the output record
        cout << " ------ uGtExt ----------" << endl;
        if (uGtExt.isValid()) {
          if (i >= uGtExt->getFirstBX() && i <= uGtExt->getLastBX()) {
            for (std::vector<GlobalExtBlk>::const_iterator extBlk = uGtExt->begin(i); extBlk != uGtExt->end(i);
                 ++extBlk) {
              extBlk->print(std::cout);
            }
          } else {
            cout << "No Ext Conditions stored for this bx " << i << endl;
          }
        } else {
          cout << "No uGtExt Data in this event " << endl;
        }

        // Dump the output record
        cout << " ------ uGtAlg ----------" << endl;
        if (uGtAlg.isValid()) {
          if (i >= uGtAlg->getFirstBX() && i <= uGtAlg->getLastBX()) {
            for (std::vector<GlobalAlgBlk>::const_iterator algBlk = uGtAlg->begin(i); algBlk != uGtAlg->end(i);
                 ++algBlk) {
              algBlk->print(std::cout);
            }
          } else {
            cout << "No Alg Decisions stored for this bx " << i << endl;
          }
        } else {
          cout << "No uGtAlg Data in this event " << endl;
        }

      }  //loop over Bx
      cout << std::dec << endl;
    }  //if dumpGtRecord

    // Dump Test Vectors for this bx
    if (m_dumpTestVectors) {
      for (int i = m_minBxVectors; i <= m_maxBxVectors; i++) {
        //         if(  (i>=egammas->getFirstBX() && i<=egammas->getLastBX())&&
        //	      (i>=muons->getFirstBX()   && i<=muons->getLastBX())  &&
        //	      (i>=taus->getFirstBX()    && i<=taus->getLastBX())   &&
        //	      (i>=jets->getFirstBX()    && i<=jets->getLastBX())   &&
        //	      (i>=etsums->getFirstBX()  && i<=etsums->getLastBX()) &&
        //	      (i>=uGtAlg->getFirstBX()  && i<=uGtAlg->getLastBX()) &&
        //	      (i>=uGtAlg->getFirstBX()  && i<=uGtAlg->getLastBX()) ) {
        dumpTestVectors(i, m_testVectorFile, muons, muonShowers, egammas, taus, jets, etsums, uGtAlg, uGtExt);
        //	 } else {
        //	      edm::LogWarning("GtRecordDump") << "WARNING: Not enough information to dump test vectors for this bx=" << i << endl;
        //	 }
      }
    }
  }

  // ------------ method called when ending the processing of a run  ------------

  void GtRecordDump::endRun(edm::Run const&, edm::EventSetup const&) {
    // Dump the results
    cout << "=========================== Global Trigger Summary Report  ==================================" << endl;
    cout << "                       Algorithm Name                              Init     aBXM     Final   " << endl;
    cout << "=============================================================================================" << endl;
    for (std::map<std::string, std::vector<int>>::const_iterator itAlgo = m_algoSummary.begin();
         itAlgo != m_algoSummary.end();
         itAlgo++) {
      std::string name = itAlgo->first;
      int initCnt = (itAlgo->second).at(0);
      int initPre = (itAlgo->second).at(1);
      int initFnl = (itAlgo->second).at(2);
      if (name != "NULL")
        cout << std::dec << setfill(' ') << setw(60) << name.c_str() << setw(10) << initCnt << setw(10) << initPre
             << setw(10) << initFnl << endl;
    }
    cout
        << "==========================================================================================================="
        << endl;
  }

  void GtRecordDump::dumpTestVectors(int bx,
                                     std::ofstream& myOutFile,
                                     Handle<BXVector<l1t::Muon>> muons,
                                     Handle<BXVector<l1t::MuonShower>> muonShowers,
                                     Handle<BXVector<l1t::EGamma>> egammas,
                                     Handle<BXVector<l1t::Tau>> taus,
                                     Handle<BXVector<l1t::Jet>> jets,
                                     Handle<BXVector<l1t::EtSum>> etsums,
                                     Handle<BXVector<GlobalAlgBlk>> uGtAlg,
                                     Handle<BXVector<GlobalExtBlk>> uGtExt) {
    const int empty = 0;

    // Dump Bx (4 digits)
    myOutFile << std::dec << std::setw(4) << std::setfill('0') << m_absBx;

    // Dump 8 Muons (16 digits + space) + Muon Showers
    int nDumped = 0;

    int muNumber = 0;  //keeps track of which muons get which muon shower information
    if (muons.isValid() && muonShowers.isValid()) {
      for (std::vector<l1t::Muon>::const_iterator mu = muons->begin(bx); mu != muons->end(bx); ++mu) {
        // loop over valid muons in this bx (muon 0 up to max possible of muon 7)
        int muShowerBit = 0;  // default value for muon shower bit
        if (bx >= muonShowers->getFirstBX() && bx <= muonShowers->getLastBX()) {
          if (muonShowers->size(bx) > 0) {
            std::vector<l1t::MuonShower>::const_iterator muShower = muonShowers->begin(bx);
            if (muNumber == 0)
              muShowerBit = muShower->isOneNominalInTime();
            if (muNumber == 2)
              muShowerBit = muShower->isOneTightInTime();
            if (muNumber == 3)
              muShowerBit = muShower->isTwoLooseDiffSectorsInTime();
            if (muNumber == 4)
              muShowerBit = muShower->musOutOfTime0();
            if (muNumber == 6)
              muShowerBit = muShower->musOutOfTime1();
          }
        }
        cms_uint64_t packedWd = formatMuon(mu, muShowerBit);
        if (nDumped < 8) {
          myOutFile << " " << std::hex << std::setw(16) << std::setfill('0') << packedWd;
          nDumped++;
        }
        ++muNumber;  //keeps track of how many muons have been processed
      }  // end loop over Muons in this bx

      // Muon Shower information can exist, even if a muon object does not exist.  Hence,
      // now loop over non-existant muons from muNumber up to max of 7 and add the muon shower info
      int start = muNumber;
      for (int nonExistantMuon = start; nonExistantMuon < 8; nonExistantMuon++) {
        int muShowerBit = 0;  // default value for muon shower bit
        if (bx >= muonShowers->getFirstBX() && bx <= muonShowers->getLastBX()) {
          if (muonShowers->size(bx) > 0) {
            std::vector<l1t::MuonShower>::const_iterator muShower = muonShowers->begin(bx);
            if (muNumber == 0)
              muShowerBit = muShower->isOneNominalInTime();
            if (muNumber == 2)
              muShowerBit = muShower->isOneTightInTime();
            if (muNumber == 3)
              muShowerBit = muShower->isTwoLooseDiffSectorsInTime();
            if (muNumber == 4)
              muShowerBit = muShower->musOutOfTime0();
            if (muNumber == 6)
              muShowerBit = muShower->musOutOfTime1();
          }
        }
        cms_uint64_t packedWd = formatNonExistantMuon(muShowerBit);
        if (nDumped < 8) {
          myOutFile << " " << std::hex << std::setw(16) << std::setfill('0') << packedWd;
          nDumped++;
        }
        ++muNumber;  // keep track of the number of muons processed
      }  // end loop over non-existant muons
    }
    for (int i = nDumped; i < 8; i++) {
      myOutFile << " " << std::hex << std::setw(16) << std::setfill('0') << empty;
    }
    if (!muons.isValid())
      std::cout << "========= WARNING:  ALL MUONS INVALID ==========" << std::endl;
    if (!muonShowers.isValid())
      std::cout << "========= WARNING:  ALL MUON SHOWERS INVALID ==========" << std::endl;
    //===========================================

    // Dump 12 EG (8 digits + space)
    nDumped = 0;
    if (egammas.isValid()) {
      for (std::vector<l1t::EGamma>::const_iterator eg = egammas->begin(bx); eg != egammas->end(bx); ++eg) {
        unsigned int packedWd = formatEG(eg);
        if (nDumped < 12) {
          myOutFile << " " << std::hex << std::setw(8) << std::setfill('0') << packedWd;
          nDumped++;
        }
      }
    }
    for (int i = nDumped; i < 12; i++) {
      myOutFile << " " << std::hex << std::setw(8) << std::setfill('0') << empty;
    }

    // Dump 8 tau (8 digits + space)
    nDumped = 0;
    int maxTau = 8;
    if (m_tvVersion > 1)
      maxTau = 12;
    if (taus.isValid()) {
      for (std::vector<l1t::Tau>::const_iterator tau = taus->begin(bx); tau != taus->end(bx); ++tau) {
        unsigned int packedWd = formatTau(tau);
        if (nDumped < maxTau) {
          myOutFile << " " << std::hex << std::setw(8) << std::setfill('0') << packedWd;
          nDumped++;
        }
      }
    }
    for (int i = nDumped; i < maxTau; i++) {
      myOutFile << " " << std::hex << std::setw(8) << std::setfill('0') << empty;
    }

    // Dump 12 Jets (8 digits + space)
    nDumped = 0;
    if (jets.isValid()) {
      for (std::vector<l1t::Jet>::const_iterator jet = jets->begin(bx); jet != jets->end(bx); ++jet) {
        unsigned int packedWd = formatJet(jet);
        if (nDumped < 12) {
          myOutFile << " " << std::hex << std::setw(8) << std::setfill('0') << packedWd;
          nDumped++;
        }
      }
    }
    for (int i = nDumped; i < 12; i++) {
      myOutFile << " " << std::hex << std::setw(8) << std::setfill('0') << empty;
    }

    // Dump Et Sums (ETT, HT, ETM, ETMHF, HTM)
    unsigned int ETTpackWd = 0;
    unsigned int HTTpackWd = 0;
    unsigned int ETMpackWd = 0;
    unsigned int HTMpackWd = 0;
    unsigned int ETMHFpackWd = 0;
    unsigned int HTMHFpackWd = 0;

    // quantities packed into the words
    unsigned int ETTempackWd = 0;
    unsigned int HFP0packWd = 0;
    unsigned int HFM0packWd = 0;
    unsigned int HFP1packWd = 0;
    unsigned int HFM1packWd = 0;
    unsigned int TowerCountspackWd = 0;  // ccla
    unsigned int AsymEtpackWd = 0;
    unsigned int AsymHtpackWd = 0;
    unsigned int AsymEtHFpackWd = 0;
    unsigned int AsymHtHFpackWd = 0;
    unsigned int CENT30packWd = 0;  // centrality bits 3:0 in ETMHF word
    unsigned int CENT74packWd = 0;  // centrality bits 7:4 in HTMHF word
    std::pair<unsigned int, unsigned int> centrality(0, 0);

    if (etsums.isValid()) {
      for (std::vector<l1t::EtSum>::const_iterator etsum = etsums->begin(bx); etsum != etsums->end(bx); ++etsum) {
        switch (etsum->getType()) {
          case l1t::EtSum::EtSumType::kMissingEt:
            ETMpackWd = formatMissET(etsum);
            break;
          case l1t::EtSum::EtSumType::kMissingEtHF:
            ETMHFpackWd = formatMissET(etsum);
            break;
          case l1t::EtSum::EtSumType::kMissingHtHF:
            HTMHFpackWd = formatMissET(etsum);
            break;
          case l1t::EtSum::EtSumType::kMissingHt:
            HTMpackWd = formatMissET(etsum);
            break;
          case l1t::EtSum::EtSumType::kTotalEt:
            ETTpackWd = formatTotalET(etsum);
            break;
          case l1t::EtSum::EtSumType::kTotalEtEm:
            ETTempackWd = formatTotalET(etsum);
            break;
          case l1t::EtSum::EtSumType::kTotalHt:
            HTTpackWd = formatTotalET(etsum);
            break;
          case l1t::EtSum::EtSumType::kTowerCount:
            TowerCountspackWd = formatTowerCounts(etsum);
            break;
          case l1t::EtSum::EtSumType::kMinBiasHFP0:
            HFP0packWd = formatHMB(etsum);
            break;
          case l1t::EtSum::EtSumType::kMinBiasHFM0:
            HFM0packWd = formatHMB(etsum);
            break;
          case l1t::EtSum::EtSumType::kMinBiasHFP1:
            HFP1packWd = formatHMB(etsum);
            break;
          case l1t::EtSum::EtSumType::kMinBiasHFM1:
            HFM1packWd = formatHMB(etsum);
            break;
          case l1t::EtSum::EtSumType::kCentrality:
            centrality = formatCentrality(etsum);
            CENT30packWd = centrality.first;
            CENT74packWd = centrality.second;
            break;
          case l1t::EtSum::EtSumType::kAsymEt:
            AsymEtpackWd = formatAsym(etsum);
            break;
          case l1t::EtSum::EtSumType::kAsymHt:
            AsymHtpackWd = formatAsym(etsum);
            break;
          case l1t::EtSum::EtSumType::kAsymEtHF:
            AsymEtHFpackWd = formatAsym(etsum);
            break;
          case l1t::EtSum::EtSumType::kAsymHtHF:
            AsymHtHFpackWd = formatAsym(etsum);
            break;
          default:
            break;
        }  //end switch statement
      }  //end loop over etsums
    }

    // Put HMB bits in upper part of other SumEt Words
    ETTpackWd |= HFP0packWd;
    HTTpackWd |= HFM0packWd;
    ETMpackWd |= HFP1packWd;
    HTMpackWd |= HFM1packWd;

    // ETTem goes into ETT word bits 12 - 23
    if (m_tvVersion > 1)
      ETTpackWd |= (ETTempackWd << 12);

    // ccla Towercounts go in HTT word, bits 12-24
    if (m_tvVersion > 1)
      HTTpackWd |= (TowerCountspackWd << 12);
    if (m_tvVersion > 2) {
      ETMpackWd |= AsymEtpackWd;
      HTMpackWd |= AsymHtpackWd;
      ETMHFpackWd |= AsymEtHFpackWd;
      HTMHFpackWd |= AsymHtHFpackWd;

      ETMHFpackWd |= CENT30packWd;
      HTMHFpackWd |= CENT74packWd;
    }
    // Fill in the words in appropriate order
    myOutFile << " " << std::hex << std::setw(8) << std::setfill('0') << ETTpackWd;
    myOutFile << " " << std::hex << std::setw(8) << std::setfill('0') << HTTpackWd;
    myOutFile << " " << std::hex << std::setw(8) << std::setfill('0') << ETMpackWd;
    myOutFile << " " << std::hex << std::setw(8) << std::setfill('0') << HTMpackWd;
    if (m_tvVersion > 1) {
      myOutFile << " " << std::hex << std::setw(8) << std::setfill('0') << ETMHFpackWd;
      if (m_tvVersion > 2) {
        myOutFile << " " << std::hex << std::setw(8) << std::setfill('0') << HTMHFpackWd;
      } else {
        myOutFile << " " << std::hex << std::setw(8) << std::setfill('0') << empty;
      }
    }

    // If tvVersion > 1 put in placeholds for empty link (6 words (frames)) all zeros.
    if (m_tvVersion > 1) {
      for (int i = 0; i < 6; i++)
        myOutFile << " " << std::hex << std::setw(8) << std::setfill('0') << empty;
    }

    // External Condition (64 digits + space)
    int digit = 0;
    myOutFile << " ";
    if (uGtExt.isValid()) {
      for (std::vector<GlobalExtBlk>::const_iterator extBlk = uGtExt->begin(bx); extBlk != uGtExt->end(bx); ++extBlk) {
        for (int i = 255; i > -1; i--) {
          if (extBlk->getExternalDecision(i))
            digit |= (1 << (i % 4));
          if ((i % 4) == 0) {
            myOutFile << std::hex << std::setw(1) << digit;
            digit = 0;
          }
        }  //end loop over external bits
      }  //loop over objects
    } else {
      myOutFile << std::hex << std::setw(64) << std::setfill('0') << empty;
    }

    // Algorithm Dump (128 digits + space)
    digit = 0;
    myOutFile << " ";
    if (uGtAlg.isValid()) {
      for (std::vector<GlobalAlgBlk>::const_iterator algBlk = uGtAlg->begin(bx); algBlk != uGtAlg->end(bx); ++algBlk) {
        for (int i = 511; i > -1; i--) {
          if (algBlk->getAlgoDecisionFinal(i))
            digit |= (1 << (i % 4));
          if ((i % 4) == 0) {
            myOutFile << std::hex << std::setw(1) << digit;
            digit = 0;
          }
        }  //end loop over algorithm bits

        // Final OR (1 digit + space)
        unsigned int finalOr = (algBlk->getFinalOR() & 0x1);
        myOutFile << " " << std::hex << std::setw(1) << std::setfill('0') << finalOr;
      }
    } else {
      myOutFile << std::hex << std::setw(128) << std::setfill('0') << empty;
    }

    myOutFile << endl;

    m_absBx++;
  }

  cms_uint64_t GtRecordDump::formatMuon(std::vector<l1t::Muon>::const_iterator mu, int muShowerBit) {
    cms_uint64_t packedVal = 0;

    // Pack Bits
    packedVal |= ((cms_uint64_t)(mu->hwPhi() & 0x3ff) << 43);
    packedVal |= ((cms_uint64_t)(mu->hwPhiAtVtx() & 0x3ff) << 0);  // & 0x3ff) <<18);
    // packedVal |= ((cms_uint64_t)(mu->hwEta() & 0x1ff) << 53);         // removed
    packedVal |= ((cms_uint64_t)(mu->hwPtUnconstrained() & 0xff) << 53);  // added
    packedVal |= ((cms_uint64_t)(mu->hwDXY() & 0x3) << 62);               // added
    packedVal |= ((cms_uint64_t)(muShowerBit & 0x1) << 61);               // added
    packedVal |= ((cms_uint64_t)(mu->hwEtaAtVtx() & 0x1ff) << 23);        // & 0x1ff) <<9);
    packedVal |= ((cms_uint64_t)(mu->hwPt() & 0x1ff) << 10);              // & 0x1ff) <<0);
    packedVal |= ((cms_uint64_t)(mu->hwChargeValid() & 0x1) << 35);       // & 0x1)   <<28);
    packedVal |= ((cms_uint64_t)(mu->hwCharge() & 0x1) << 34);            // & 0x1)   <<29);
    packedVal |= ((cms_uint64_t)(mu->hwQual() & 0xf) << 19);              // & 0xf)   <<30);
    packedVal |= ((cms_uint64_t)(mu->hwIso() & 0x3) << 32);               // & 0x3)   <<34);
    packedVal |= ((cms_uint64_t)(mu->tfMuonIndex() & 0x7f) << 36);

    //    if (false) {  // for debugging purposes
    //      std::cout << "----------------------" << std::endl;
    //      std::cout << "<<  0; mu->hwPhiAtVtx()        = " << std::hex << std::setw(16) << std::setfill('0')
    //                << ((cms_uint64_t)(mu->hwPhiAtVtx() & 0x3ff) << 0) << std::endl;
    //      std::cout << "<< 10; mu->hwPt()              = " << std::hex << std::setw(16) << std::setfill('0')
    //                << ((cms_uint64_t)(mu->hwPt() & 0x1ff) << 10) << std::endl;
    //      std::cout << "<< 19; mu->hwQual()            = " << std::hex << std::setw(16) << std::setfill('0')
    //                << ((cms_uint64_t)(mu->hwQual() & 0xf) << 19) << std::endl;
    //      std::cout << "<< 23; mu->hwEtaAtVtx()        = " << std::hex << std::setw(16) << std::setfill('0')
    //                << ((cms_uint64_t)(mu->hwEtaAtVtx() & 0x1ff) << 23) << std::endl;
    //      std::cout << "<< 32; mu->hwIso()             = " << std::hex << std::setw(16) << std::setfill('0')
    //                << ((cms_uint64_t)(mu->hwIso() & 0x3) << 32) << std::endl;
    //      std::cout << "<< 34; mu->hwCharge()          = " << std::hex << std::setw(16) << std::setfill('0')
    //                << ((cms_uint64_t)(mu->hwCharge() & 0x1) << 34) << std::endl;
    //      std::cout << "<< 35; mu->hwChargeValid()     = " << std::hex << std::setw(16) << std::setfill('0')
    //                << ((cms_uint64_t)(mu->hwChargeValid() & 0x1) << 35) << std::endl;
    //      std::cout << "<< 43; mu->hwPhi()             = " << std::hex << std::setw(16) << std::setfill('0')
    //                << ((cms_uint64_t)(mu->hwPhi() & 0x3ff) << 43) << std::endl;
    //      std::cout << "<< 53; mu->hwPtUnconstrained() = " << std::hex << std::setw(16) << std::setfill('0')
    //                << ((cms_uint64_t)(mu->hwPtUnconstrained() & 0xff) << 53) << std::endl;
    //      std::cout << "<< 61; muShowerBit             = " << std::hex << std::setw(16) << std::setfill('0')
    //                << ((cms_uint64_t)(muShowerBit & 0x1) << 61) << std::endl;
    //      std::cout << "<< 62; mu->hwDXY()             = " << std::hex << std::setw(16) << std::setfill('0')
    //                << ((cms_uint64_t)(mu->hwDXY() & 0x3) << 62) << std::endl;
    //      std::cout << "packedWord                     = " << std::hex << std::setw(16) << std::setfill('0') << packedVal
    //                << std::endl;
    //      std::cout << "----------------------" << std::endl;
    //    }

    return packedVal;
  }

  cms_uint64_t GtRecordDump::formatNonExistantMuon(int muShowerBit) {
    cms_uint64_t packedVal = 0;

    // Pack Bits
    packedVal |= ((cms_uint64_t)(0 & 0x3ff) << 43);
    packedVal |= ((cms_uint64_t)(0 & 0x3ff) << 0);  // & 0x3ff) <<18);
    // packedVal |= ((cms_uint64_t)(mu->hwEta() & 0x1ff) << 53);         // removed
    packedVal |= ((cms_uint64_t)(0 & 0xff) << 53);           // added
    packedVal |= ((cms_uint64_t)(0 & 0x3) << 62);            // added
    packedVal |= ((cms_uint64_t)(muShowerBit & 0x1) << 61);  // added
    packedVal |= ((cms_uint64_t)(0 & 0x1ff) << 23);          // & 0x1ff) <<9);
    packedVal |= ((cms_uint64_t)(0 & 0x1ff) << 10);          // & 0x1ff) <<0);
    packedVal |= ((cms_uint64_t)(0 & 0x1) << 35);            // & 0x1)   <<28);
    packedVal |= ((cms_uint64_t)(0 & 0x1) << 34);            // & 0x1)   <<29);
    packedVal |= ((cms_uint64_t)(0 & 0xf) << 19);            // & 0xf)   <<30);
    packedVal |= ((cms_uint64_t)(0 & 0x3) << 32);            // & 0x3)   <<34);

    return packedVal;
  }

  unsigned int GtRecordDump::formatEG(std::vector<l1t::EGamma>::const_iterator eg) {
    unsigned int packedVal = 0;

    // Pack Bits
    packedVal |= ((eg->hwPhi() & 0xff) << 17);
    packedVal |= ((eg->hwEta() & 0xff) << 9);
    packedVal |= ((eg->hwPt() & 0x1ff) << 0);
    packedVal |= ((eg->hwIso() & 0x3) << 25);
    packedVal |= ((eg->hwQual() & 0x31) << 27);

    return packedVal;
  }

  unsigned int GtRecordDump::formatTau(std::vector<l1t::Tau>::const_iterator tau) {
    unsigned int packedVal = 0;

    // Pack Bits
    packedVal |= ((tau->hwPhi() & 0xff) << 17);
    packedVal |= ((tau->hwEta() & 0xff) << 9);
    packedVal |= ((tau->hwPt() & 0x1ff) << 0);
    packedVal |= ((tau->hwIso() & 0x3) << 25);
    packedVal |= ((tau->hwQual() & 0x31) << 27);

    return packedVal;
  }

  unsigned int GtRecordDump::formatJet(std::vector<l1t::Jet>::const_iterator jet) {
    unsigned int packedVal = 0;

    // Pack Bits
    packedVal |= ((jet->hwPhi() & 0xff) << 19);
    packedVal |= ((jet->hwEta() & 0xff) << 11);
    packedVal |= ((jet->hwPt() & 0x7ff) << 0);
    packedVal |= ((jet->hwQual() & 0x1) << 27);

    return packedVal;
  }

  unsigned int GtRecordDump::formatMissET(std::vector<l1t::EtSum>::const_iterator etSum) {
    unsigned int packedVal = 0;

    // Pack Bits
    packedVal |= ((etSum->hwPhi() & 0xff) << 12);
    packedVal |= ((etSum->hwPt() & 0xfff) << 0);

    return packedVal;
  }

  unsigned int GtRecordDump::formatTotalET(std::vector<l1t::EtSum>::const_iterator etSum) {
    unsigned int packedVal = 0;

    // Pack Bits
    packedVal |= ((etSum->hwPt() & 0xfff) << 0);

    return packedVal;
  }

  unsigned int GtRecordDump::formatTowerCounts(std::vector<l1t::EtSum>::const_iterator etSum) {
    unsigned int packedVal = 0;
    //unsigned int shift = 12;

    // Pack Bits
    //packedVal |= ((etSum->hwPt()     & 0xfff)   << shift);

    //towercount takes 13 bits
    packedVal |= ((etSum->hwPt() & 0x1fff) << 0);

    return packedVal;
  }

  unsigned int GtRecordDump::formatAsym(std::vector<l1t::EtSum>::const_iterator etSum) {
    //asym takes 8 bits, occupying bits 20-27 in ETM, HTM, ETMHF, and HTMHF etsums
    unsigned int packedVal = 0;
    unsigned int shift = 20;

    // Pack Bits
    packedVal |= ((etSum->hwPt() & 0xff) << shift);

    return packedVal;
  }

  unsigned int GtRecordDump::formatHMB(std::vector<l1t::EtSum>::const_iterator etSum) {
    // 4 bits, occupying bits 28-31.
    unsigned int packedVal = 0;
    unsigned int shift = 28;

    // Pack Bits
    packedVal |= ((etSum->hwPt() & 0xf) << shift);

    return packedVal;
  }

  std::pair<unsigned int, unsigned int> GtRecordDump::formatCentrality(std::vector<l1t::EtSum>::const_iterator etSum) {
    unsigned int centword = etSum->hwPt();

    // unpack word into 2 4 bit words
    int firstfour = (centword & 0xF);
    int lastfour = (centword >> 4) & 0xF;

    // 4 bits, occupying bits 28-31.
    unsigned int packedValLN = 0;
    unsigned int packedValUN = 0;
    unsigned int shift = 28;

    // Pack Bits
    packedValLN |= ((firstfour & 0xf) << shift);
    packedValUN |= ((lastfour & 0xf) << shift);

    std::pair<unsigned int, unsigned int> centrality(packedValLN, packedValUN);
    return centrality;
  }

}  // namespace l1t

DEFINE_FWK_MODULE(l1t::GtRecordDump);
