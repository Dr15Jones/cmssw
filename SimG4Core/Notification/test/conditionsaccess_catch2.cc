// -*- C++ -*-
//
// Package:     SimG4Core/Notification
// Class  :     conditionsaccess_catch2
//
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Christopher Jones
//         Created:  Fri, 05 Mar 2021 14:53:10 GMT
//

// system include files
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// user include files
#include "SimG4Core/Notification/interface/ConditionsAccess.h"

#include "FWCore/Framework/interface/EventSetupRecordImplementation.h"
//
// constants, enums and typedefs
//
namespace conditionsaccess_test {
  struct DummyRec1 : public edm::eventsetup::EventSetupRecordImplementation<DummyRec1> {};
  struct DummyData1 {};
  struct DummyRec2 : public edm::eventsetup::EventSetupRecordImplementation<DummyRec2> {};
  struct DummyData2 {};
}  // namespace conditionsaccess_test
using namespace conditionsaccess_test;
HCTYPETAG_HELPER_METHODS(DummyRec1);
HCTYPETAG_HELPER_METHODS(DummyRec2);
HCTYPETAG_HELPER_METHODS(DummyData1);
HCTYPETAG_HELPER_METHODS(DummyData2);

TEST_CASE("Test sim::ConditionsAccess", "[ConditionsAccess]") {
  SECTION("Unlabeled data") {
    sim::ConditionsAccess ca;

    REQUIRE(not ca.hasRetriever<DummyData1, DummyRec1>(""));
    REQUIRE(not ca.hasRetriever<DummyData1, DummyRec2>(""));
    ca.insertRetriever<DummyData1, DummyRec1>("", edm::ESGetToken<DummyData1, DummyRec1>());

    REQUIRE(ca.hasRetriever<DummyData1, DummyRec1>(""));
    REQUIRE(not ca.hasRetriever<DummyData1, DummyRec2>(""));
  }

  SECTION("Labeled data") {
    sim::ConditionsAccess ca;

    REQUIRE(not ca.hasRetriever<DummyData1, DummyRec1>("foo"));
    REQUIRE(not ca.hasRetriever<DummyData1, DummyRec1>(""));
    ca.insertRetriever<DummyData1, DummyRec1>("foo", edm::ESGetToken<DummyData1, DummyRec1>());

    REQUIRE(ca.hasRetriever<DummyData1, DummyRec1>("foo"));
    REQUIRE(not ca.hasRetriever<DummyData1, DummyRec1>(""));
    REQUIRE(not ca.hasRetriever<DummyData1, DummyRec2>(""));
  }

  SECTION("Labeled and unlabeled data") {
    sim::ConditionsAccess ca;

    REQUIRE(not ca.hasRetriever<DummyData1, DummyRec1>("foo"));
    REQUIRE(not ca.hasRetriever<DummyData1, DummyRec1>(""));
    ca.insertRetriever<DummyData1, DummyRec1>("foo", edm::ESGetToken<DummyData1, DummyRec1>());

    REQUIRE(ca.hasRetriever<DummyData1, DummyRec1>("foo"));
    REQUIRE(not ca.hasRetriever<DummyData1, DummyRec1>(""));
    REQUIRE(not ca.hasRetriever<DummyData1, DummyRec2>(""));

    ca.insertRetriever<DummyData1, DummyRec1>("", edm::ESGetToken<DummyData1, DummyRec1>());

    REQUIRE(ca.hasRetriever<DummyData1, DummyRec1>("foo"));
    REQUIRE(ca.hasRetriever<DummyData1, DummyRec1>(""));
    REQUIRE(not ca.hasRetriever<DummyData1, DummyRec2>(""));
  }

  SECTION("Same data type different records") {
    sim::ConditionsAccess ca;

    REQUIRE(not ca.hasRetriever<DummyData1, DummyRec1>(""));
    REQUIRE(not ca.hasRetriever<DummyData1, DummyRec2>(""));
    ca.insertRetriever<DummyData1, DummyRec1>("", edm::ESGetToken<DummyData1, DummyRec1>());

    REQUIRE(ca.hasRetriever<DummyData1, DummyRec1>(""));
    REQUIRE(not ca.hasRetriever<DummyData1, DummyRec2>(""));

    ca.insertRetriever<DummyData1, DummyRec2>("", edm::ESGetToken<DummyData1, DummyRec2>());

    REQUIRE(ca.hasRetriever<DummyData1, DummyRec1>(""));
    REQUIRE(ca.hasRetriever<DummyData1, DummyRec2>(""));
  }
}
