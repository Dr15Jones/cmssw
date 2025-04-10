#include "catch.hpp"
#include "DataFormats/Provenance/interface/ProcessHistory.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Provenance/interface/IndexIntoFile.h"
#include "DataFormats/Provenance/interface/ParameterSetID.h"
#include "DataFormats/Provenance/interface/ProcessConfiguration.h"
#include "DataFormats/Provenance/interface/ProcessHistoryID.h"
#include "DataFormats/Provenance/interface/ProcessHistoryRegistry.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Version/interface/GetReleaseVersion.h"

#include <string>
#include <iostream>

bool checkRunOrLumiEntry(edm::IndexIntoFile::RunOrLumiEntry const& rl,
                         edm::IndexIntoFile::EntryNumber_t orderPHIDRun,
                         edm::IndexIntoFile::EntryNumber_t orderPHIDRunLumi,
                         edm::IndexIntoFile::EntryNumber_t entry,
                         int processHistoryIDIndex,
                         edm::RunNumber_t run,
                         edm::LuminosityBlockNumber_t lumi,
                         edm::IndexIntoFile::EntryNumber_t beginEvents,
                         edm::IndexIntoFile::EntryNumber_t endEvents) {
  if (rl.orderPHIDRun() != orderPHIDRun)
    return false;
  if (rl.orderPHIDRunLumi() != orderPHIDRunLumi)
    return false;
  if (rl.entry() != entry)
    return false;
  if (rl.processHistoryIDIndex() != processHistoryIDIndex)
    return false;
  if (rl.run() != run)
    return false;
  if (rl.lumi() != lumi)
    return false;
  if (rl.beginEvents() != beginEvents)
    return false;
  if (rl.endEvents() != endEvents)
    return false;
  return true;
}

TEST_CASE("test ProcessHistory", "[ProcessHistory]") {
  edm::ProcessHistoryRegistry processHistoryRegistry;
  edm::ParameterSet dummyPset;
  edm::ParameterSetID psetID;
  dummyPset.registerIt();
  psetID = dummyPset.id();
  SECTION("ProcessHistory") {
    edm::ProcessHistory pnl1;
    edm::ProcessHistory pnl2;
    SECTION("default initialize comparison") {
      REQUIRE(pnl1 == pnl1);
      REQUIRE(pnl1 == pnl2);
    }
    edm::ProcessConfiguration iHLT("HLT", psetID, "CMSSW_5_100_40", edm::HardwareResourcesDescription());
    edm::ProcessConfiguration iRECO("RECO", psetID, "5_100_42patch100", edm::HardwareResourcesDescription());
    pnl2.push_back(iHLT);
    edm::ProcessHistory pnl3;
    pnl3.push_back(iHLT);
    pnl3.push_back(iRECO);

    SECTION("different history comparison") {
      REQUIRE(pnl1 != pnl2);
      edm::ProcessHistoryID id1 = pnl1.setProcessHistoryID();
      edm::ProcessHistoryID id2 = pnl2.setProcessHistoryID();
      edm::ProcessHistoryID id3 = pnl3.setProcessHistoryID();

      REQUIRE(id1 != id2);
      REQUIRE(id2 != id3);
      REQUIRE(id3 != id1);

      edm::ProcessHistory pnl4;
      pnl4.push_back(iHLT);
      edm::ProcessHistoryID id4 = pnl4.setProcessHistoryID();
      REQUIRE(pnl4 == pnl2);
      REQUIRE(id4 == id2);

      edm::ProcessHistory pnl5;
      pnl5 = pnl3;
      REQUIRE(pnl5 == pnl3);
      REQUIRE(pnl5.id() == pnl3.id());
    }

    SECTION("reduce") {
      edm::ProcessConfiguration pc1("HLT", psetID, "", edm::HardwareResourcesDescription());
      edm::ProcessConfiguration pc2("HLT", psetID, "a", edm::HardwareResourcesDescription());
      edm::ProcessConfiguration pc3("HLT", psetID, "1", edm::HardwareResourcesDescription());
      edm::ProcessConfiguration pc4("HLT", psetID, "ccc500yz", edm::HardwareResourcesDescription());
      edm::ProcessConfiguration pc5("HLT", psetID, "500yz872", edm::HardwareResourcesDescription());
      edm::ProcessConfiguration pc6("HLT", psetID, "500yz872djk999patch10", edm::HardwareResourcesDescription());
      edm::ProcessConfiguration pc7("HLT", psetID, "xb500yz872djk999patch10", edm::HardwareResourcesDescription());
      edm::ProcessConfiguration pc8("HLT", psetID, "CMSSW_4_4_0_pre5", edm::HardwareResourcesDescription());
      edm::HardwareResourcesDescription hrd;
      hrd.microarchitecture = "fred";
      edm::ProcessConfiguration pc9("HLT", psetID, "CMSSW_4_4_0_pre5", hrd);

      pc1.setProcessConfigurationID();
      pc2.setProcessConfigurationID();
      pc3.setProcessConfigurationID();
      pc4.setProcessConfigurationID();
      pc5.setProcessConfigurationID();
      pc6.setProcessConfigurationID();
      pc7.setProcessConfigurationID();
      pc8.setProcessConfigurationID();
      pc9.setProcessConfigurationID();

      REQUIRE(pc9.id() != pc8.id());

      pc1.reduce();
      pc2.reduce();
      pc3.reduce();
      pc4.reduce();
      pc5.reduce();
      pc6.reduce();
      pc7.reduce();
      pc8.reduce();
      pc9.reduce();

      REQUIRE(pc9.id() == pc8.id());

      edm::ProcessConfiguration pc1expected("HLT", psetID, "", edm::HardwareResourcesDescription());
      edm::ProcessConfiguration pc2expected("HLT", psetID, "a", edm::HardwareResourcesDescription());
      edm::ProcessConfiguration pc3expected("HLT", psetID, "1", edm::HardwareResourcesDescription());
      edm::ProcessConfiguration pc4expected("HLT", psetID, "ccc500yz", edm::HardwareResourcesDescription());
      edm::ProcessConfiguration pc5expected("HLT", psetID, "500yz872", edm::HardwareResourcesDescription());
      edm::ProcessConfiguration pc6expected("HLT", psetID, "500yz872", edm::HardwareResourcesDescription());
      edm::ProcessConfiguration pc7expected("HLT", psetID, "xb500yz872", edm::HardwareResourcesDescription());
      edm::ProcessConfiguration pc8expected("HLT", psetID, "CMSSW_4_4", edm::HardwareResourcesDescription());
      edm::ProcessConfiguration pc9expected = pc8expected;

      REQUIRE(pc1 == pc1expected);
      REQUIRE(pc2 == pc2expected);
      REQUIRE(pc3 == pc3expected);
      REQUIRE(pc4 == pc4expected);
      REQUIRE(pc5 == pc5expected);
      REQUIRE(pc6 == pc6expected);
      REQUIRE(pc7 == pc7expected);
      REQUIRE(pc8 == pc8expected);
      REQUIRE(pc9 == pc9expected);

      REQUIRE(pc1.id() == pc1expected.id());
      REQUIRE(pc2.id() == pc2expected.id());
      REQUIRE(pc3.id() == pc3expected.id());
      REQUIRE(pc4.id() == pc4expected.id());
      REQUIRE(pc5.id() == pc5expected.id());
      REQUIRE(pc6.id() == pc6expected.id());
      REQUIRE(pc7.id() == pc7expected.id());
      REQUIRE(pc8.id() == pc8expected.id());
      REQUIRE(pc9.id() == pc9expected.id());

      REQUIRE(pc7.id() != pc8expected.id());
    }
    SECTION("multi-step history reduce") {
      edm::ProcessConfiguration iHLTreduced("HLT", psetID, "CMSSW_5_100", edm::HardwareResourcesDescription());
      edm::ProcessConfiguration iRECOreduced("RECO", psetID, "5_100", edm::HardwareResourcesDescription());
      edm::ProcessHistory phTestExpected;
      phTestExpected.push_back(iHLTreduced);
      phTestExpected.push_back(iRECOreduced);

      edm::ProcessHistory phTest = pnl3;
      phTest.setProcessHistoryID();
      phTest.reduce();
      REQUIRE(phTest == phTestExpected);
      REQUIRE(phTest.id() == phTestExpected.id());
      REQUIRE(phTest.id() != pnl3.id());

      processHistoryRegistry.registerProcessHistory(pnl3);
      edm::ProcessHistoryID reducedPHID = processHistoryRegistry.reducedProcessHistoryID(pnl3.id());
      REQUIRE(reducedPHID == phTest.id());

      processHistoryRegistry.registerProcessHistory(pnl2);
      reducedPHID = processHistoryRegistry.reducedProcessHistoryID(pnl2.id());
      pnl2.reduce();
      REQUIRE(reducedPHID == pnl2.id());
    }
  }
  SECTION("IndexIntoFile") {
    edm::ProcessHistory ph1;
    edm::ProcessHistory ph1a;
    edm::ProcessHistory ph1b;
    edm::ProcessHistory ph2;
    edm::ProcessHistory ph2a;
    edm::ProcessHistory ph2b;
    edm::ProcessHistory ph3;
    edm::ProcessHistory ph4;

    edm::ProcessConfiguration pc1("HLT", psetID, "CMSSW_5_1_40", edm::HardwareResourcesDescription());
    edm::ProcessConfiguration pc1a("HLT", psetID, "CMSSW_5_1_40patch1", edm::HardwareResourcesDescription());
    edm::ProcessConfiguration pc1b("HLT", psetID, "CMSSW_5_1_40patch2", edm::HardwareResourcesDescription());
    edm::ProcessConfiguration pc2("HLT", psetID, "CMSSW_5_2_40", edm::HardwareResourcesDescription());
    edm::ProcessConfiguration pc2a("HLT", psetID, "CMSSW_5_2_40patch1", edm::HardwareResourcesDescription());
    edm::ProcessConfiguration pc2b("HLT", psetID, "CMSSW_5_2_40patch2", edm::HardwareResourcesDescription());
    edm::ProcessConfiguration pc3("HLT", psetID, "CMSSW_5_3_40", edm::HardwareResourcesDescription());
    edm::ProcessConfiguration pc4("HLT", psetID, "CMSSW_5_4_40", edm::HardwareResourcesDescription());

    ph1.push_back(pc1);
    ph1a.push_back(pc1a);
    ph1b.push_back(pc1b);
    ph2.push_back(pc2);
    ph2a.push_back(pc2a);
    ph2b.push_back(pc2b);
    ph3.push_back(pc3);
    ph4.push_back(pc4);

    edm::ProcessHistoryID phid1 = ph1.setProcessHistoryID();
    edm::ProcessHistoryID phid1a = ph1a.setProcessHistoryID();
    edm::ProcessHistoryID phid1b = ph1b.setProcessHistoryID();
    edm::ProcessHistoryID phid2 = ph2.setProcessHistoryID();
    edm::ProcessHistoryID phid2a = ph2a.setProcessHistoryID();
    edm::ProcessHistoryID phid2b = ph2b.setProcessHistoryID();
    edm::ProcessHistoryID phid3 = ph3.setProcessHistoryID();
    edm::ProcessHistoryID phid4 = ph4.setProcessHistoryID();

    processHistoryRegistry.registerProcessHistory(ph1);
    processHistoryRegistry.registerProcessHistory(ph1a);
    processHistoryRegistry.registerProcessHistory(ph1b);
    processHistoryRegistry.registerProcessHistory(ph2);
    processHistoryRegistry.registerProcessHistory(ph2a);
    processHistoryRegistry.registerProcessHistory(ph2b);
    processHistoryRegistry.registerProcessHistory(ph3);
    processHistoryRegistry.registerProcessHistory(ph4);

    edm::ProcessHistory rph1 = ph1;
    edm::ProcessHistory rph1a = ph1a;
    edm::ProcessHistory rph1b = ph1b;
    edm::ProcessHistory rph2 = ph2;
    edm::ProcessHistory rph2a = ph2a;
    edm::ProcessHistory rph2b = ph2b;
    edm::ProcessHistory rph3 = ph3;
    edm::ProcessHistory rph4 = ph4;
    rph1.reduce();
    rph1a.reduce();
    rph1b.reduce();
    rph2.reduce();
    rph2a.reduce();
    rph2b.reduce();
    rph3.reduce();
    rph4.reduce();

    SECTION("sequential") {
      edm::IndexIntoFile indexIntoFile;
      indexIntoFile.addEntry(phid1, 1, 0, 0, 0);
      indexIntoFile.addEntry(phid2, 2, 0, 0, 1);
      indexIntoFile.addEntry(phid3, 3, 0, 0, 2);
      indexIntoFile.addEntry(phid4, 4, 0, 0, 3);

      indexIntoFile.sortVector_Run_Or_Lumi_Entries();

      indexIntoFile.reduceProcessHistoryIDs(processHistoryRegistry);

      std::vector<edm::ProcessHistoryID> const& v = indexIntoFile.processHistoryIDs();
      REQUIRE(v[0] == rph1.id());
      REQUIRE(v[1] == rph2.id());
      REQUIRE(v[2] == rph3.id());
      REQUIRE(v[3] == rph4.id());
    }

    SECTION("non-sequential") {
      edm::IndexIntoFile indexIntoFile1;
      indexIntoFile1.addEntry(phid1, 1, 11, 0, 0);
      indexIntoFile1.addEntry(phid1, 1, 12, 0, 1);
      indexIntoFile1.addEntry(phid1, 1, 0, 0, 0);
      indexIntoFile1.addEntry(phid2, 2, 11, 0, 2);
      indexIntoFile1.addEntry(phid2, 2, 12, 0, 3);
      indexIntoFile1.addEntry(phid2, 2, 0, 0, 1);
      indexIntoFile1.addEntry(phid1a, 1, 11, 1, 0);
      indexIntoFile1.addEntry(phid1a, 1, 11, 2, 1);
      indexIntoFile1.addEntry(phid1a, 1, 11, 0, 4);
      indexIntoFile1.addEntry(phid1a, 1, 12, 1, 2);
      indexIntoFile1.addEntry(phid1a, 1, 12, 2, 3);
      indexIntoFile1.addEntry(phid1a, 1, 12, 0, 5);
      indexIntoFile1.addEntry(phid1a, 1, 0, 0, 2);
      indexIntoFile1.addEntry(phid3, 3, 0, 0, 3);
      indexIntoFile1.addEntry(phid2a, 2, 0, 0, 4);
      indexIntoFile1.addEntry(phid2b, 2, 0, 0, 5);
      indexIntoFile1.addEntry(phid4, 4, 0, 0, 6);
      indexIntoFile1.addEntry(phid1b, 1, 0, 0, 7);
      indexIntoFile1.addEntry(phid1, 5, 11, 0, 6);
      indexIntoFile1.addEntry(phid1, 5, 0, 0, 8);
      indexIntoFile1.addEntry(phid4, 1, 11, 0, 7);
      indexIntoFile1.addEntry(phid4, 1, 0, 0, 9);

      indexIntoFile1.sortVector_Run_Or_Lumi_Entries();

      std::vector<edm::ProcessHistoryID> const& v1 = indexIntoFile1.processHistoryIDs();
      REQUIRE(v1.size() == 8U);

      indexIntoFile1.reduceProcessHistoryIDs(processHistoryRegistry);

      std::vector<edm::ProcessHistoryID> const& rv1 = indexIntoFile1.processHistoryIDs();
      REQUIRE(rv1.size() == 4U);
      REQUIRE(rv1[0] == rph1.id());
      REQUIRE(rv1[1] == rph2.id());
      REQUIRE(rv1[2] == rph3.id());
      REQUIRE(rv1[3] == rph4.id());

      std::vector<edm::IndexIntoFile::RunOrLumiEntry>& runOrLumiEntries = indexIntoFile1.setRunOrLumiEntries();

      REQUIRE(runOrLumiEntries.size() == 18U);

      /*
       std::cout << rv1[0] << "  " << rph1 << "\n";
       std::cout << rv1[1] << "  " << rph2 << "\n";
       std::cout << rv1[2] << "  " << rph3 << "\n";
       std::cout << rv1[3] << "  " << rph4 << "\n";
       
       for (auto const& item : runOrLumiEntries) {
       std::cout << item.orderPHIDRun() << "  "
       << item.orderPHIDRunLumi() << "  "
       << item.entry() << "  "
       << item.processHistoryIDIndex() << "  "
       << item.run() << "  "
       << item.lumi() << "  "
       << item.beginEvents() << "  "
       << item.endEvents() << "\n";
       
       }
       */
      SECTION("checkRunOrLumiEntry") {
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(0), 0, -1, 0, 0, 1, 0, -1, -1));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(1), 0, -1, 2, 0, 1, 0, -1, -1));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(2), 0, -1, 7, 0, 1, 0, -1, -1));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(3), 0, 0, 0, 0, 1, 11, -1, -1));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(4), 0, 0, 4, 0, 1, 11, 0, 2));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(5), 0, 1, 1, 0, 1, 12, -1, -1));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(6), 0, 1, 5, 0, 1, 12, 2, 4));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(7), 1, -1, 1, 1, 2, 0, -1, -1));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(8), 1, -1, 4, 1, 2, 0, -1, -1));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(9), 1, -1, 5, 1, 2, 0, -1, -1));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(10), 1, 2, 2, 1, 2, 11, -1, -1));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(11), 1, 3, 3, 1, 2, 12, -1, -1));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(12), 3, -1, 3, 2, 3, 0, -1, -1));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(13), 6, -1, 6, 3, 4, 0, -1, -1));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(14), 8, -1, 8, 0, 5, 0, -1, -1));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(15), 8, 6, 6, 0, 5, 11, -1, -1));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(16), 9, -1, 9, 3, 1, 0, -1, -1));
        REQUIRE(checkRunOrLumiEntry(runOrLumiEntries.at(17), 9, 7, 7, 3, 1, 11, -1, -1));
      }
    }
  }
}
