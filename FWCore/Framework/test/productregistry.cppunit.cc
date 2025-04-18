/**
   \file
   test for ProductRegistry

   \author Stefano ARGIRO
   \date 21 July 2005
*/

#include "DataFormats/Provenance/interface/ProductDescription.h"
#include "DataFormats/Provenance/interface/ProcessConfiguration.h"
#include "DataFormats/TestObjects/interface/ToyProducts.h"
#include "FWCore/Framework/interface/EventProcessor.h"
#include "FWCore/Framework/interface/SignallingProductRegistryFiller.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/PluginManager/interface/ProblemTracker.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Reflection/interface/TypeWithDict.h"
#include "FWCore/ParameterSetReader/interface/ParameterSetReader.h"

#include "cppunit/extensions/HelperMacros.h"

#include <memory>

#include <iostream>

class testProductRegistry : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testProductRegistry);

  CPPUNIT_TEST(testSignal);
  CPPUNIT_TEST(testWatch);
  CPPUNIT_TEST_EXCEPTION(testCircular, cms::Exception);

  CPPUNIT_TEST(testProductRegistration);
  CPPUNIT_TEST(testAddAlias);

  CPPUNIT_TEST_SUITE_END();

public:
  testProductRegistry();
  void setUp();
  void tearDown();
  void testSignal();
  void testWatch();
  void testCircular();
  void testProductRegistration();
  void testAddAlias();

private:
  std::shared_ptr<edm::ProductDescription> intBranch_;
  std::shared_ptr<edm::ProductDescription> floatBranch_;
  std::shared_ptr<edm::ProductDescription> intVecBranch_;
  std::shared_ptr<edm::ProductDescription> simpleVecBranch_;
  std::shared_ptr<edm::ProductDescription> simpleDerivedVecBranch_;
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testProductRegistry);

namespace {
  struct Listener {
    int* heard_;
    Listener(int& hear) : heard_(&hear) {}
    void operator()(edm::ProductDescription const&) { ++(*heard_); }
  };

  struct Responder {
    std::string name_;
    edm::SignallingProductRegistryFiller* reg_;
    Responder(std::string const& iName, edm::SignallingProductRegistryFiller& iReg) : name_(iName), reg_(&iReg) {
      iReg.watchProductAdditions(this, &Responder::respond);
    }
    void respond(edm::ProductDescription const& iDesc) {
      edm::ParameterSet dummyProcessPset;
      dummyProcessPset.registerIt();
      auto pc = std::make_shared<edm::ProcessConfiguration>();
      pc->setParameterSetID(dummyProcessPset.id());

      edm::ProductDescription prod(iDesc.branchType(),
                                   name_,
                                   iDesc.processName(),
                                   iDesc.fullClassName(),
                                   iDesc.friendlyClassName(),
                                   iDesc.productInstanceName() + "-" + name_,
                                   iDesc.unwrappedType());
      reg_->addProduct(prod);
    }
  };
}  // namespace

testProductRegistry::testProductRegistry() {}

void testProductRegistry::setUp() {
  edm::ParameterSet dummyProcessPset;
  dummyProcessPset.registerIt();
  auto processConfiguration = std::make_shared<edm::ProcessConfiguration>();
  processConfiguration->setParameterSetID(dummyProcessPset.id());

  edm::ParameterSet pset;
  pset.registerIt();
  intBranch_ = std::make_shared<edm::ProductDescription>(
      edm::InEvent, "labeli", "PROD", "int", "int", "int", edm::TypeWithDict(typeid(int)));

  floatBranch_ = std::make_shared<edm::ProductDescription>(
      edm::InEvent, "labelf", "PROD", "float", "float", "float", edm::TypeWithDict(typeid(float)));

  intVecBranch_ = std::make_shared<edm::ProductDescription>(
      edm::InEvent, "labelvi", "PROD", "std::vector<int>", "ints", "vint", edm::TypeWithDict(typeid(std::vector<int>)));

  simpleVecBranch_ =
      std::make_shared<edm::ProductDescription>(edm::InEvent,
                                                "labelovsimple",
                                                "PROD",
                                                "edm::OwnVector<edmtest::Simple>",
                                                "edmtestSimplesOwned",
                                                "ovsimple",
                                                edm::TypeWithDict(typeid(edm::OwnVector<edmtest::Simple>)));
  simpleDerivedVecBranch_ =
      std::make_shared<edm::ProductDescription>(edm::InEvent,
                                                "labelovsimplederived",
                                                "PROD",
                                                "edm::OwnVector<edmtest::SimpleDerived>",
                                                "edmtestSimpleDerivedsOwned",
                                                "ovsimplederived",
                                                edm::TypeWithDict(typeid(edm::OwnVector<edmtest::SimpleDerived>)));
}

namespace {
  template <class T>
  void kill_and_clear(std::shared_ptr<T>& p) {
    p.reset();
  }
}  // namespace

void testProductRegistry::tearDown() {
  kill_and_clear(floatBranch_);
  kill_and_clear(intBranch_);
}

void testProductRegistry::testSignal() {
  using namespace edm;
  SignallingProductRegistryFiller reg;

  int hear = 0;
  Listener listening(hear);
  reg.productAddedSignal_.connect(listening);

  //ProductDescription prod(InEvent, "label", "PROD", "int", "int", "int", md);

  //   reg.addProduct(prod);
  reg.addProduct(*intBranch_);
  CPPUNIT_ASSERT(1 == hear);
}

void testProductRegistry::testWatch() {
  using namespace edm;
  SignallingProductRegistryFiller reg;

  int hear = 0;
  Listener listening(hear);
  reg.watchProductAdditions(listening);
  reg.watchProductAdditions(listening, &Listener::operator());

  Responder one("one", reg);

  //ProductDescription prod(InEvent, "label", "PROD", "int", "int", "int");
  //reg.addProduct(prod);
  reg.addProduct(*intBranch_);

  //ProductDescription prod2(InEvent, "label", "PROD", "float", "float", "float");
  //   reg.addProduct(prod2);
  reg.addProduct(*floatBranch_);

  //Should be 4 products
  // 1 from the 'int' in this routine
  // 1 from 'one' responding to this call
  // 1 from the 'float'
  // 1 from 'one' responding to the original call
  CPPUNIT_ASSERT(4 * 2 == hear);
  CPPUNIT_ASSERT(4 == reg.registry().size());
}
void testProductRegistry::testCircular() {
  using namespace edm;
  SignallingProductRegistryFiller reg;

  int hear = 0;
  Listener listening(hear);
  reg.watchProductAdditions(listening);
  reg.watchProductAdditions(listening, &Listener::operator());

  Responder one("one", reg);
  Responder two("two", reg);

  //ProductDescription prod(InEvent, "label","PROD","int","int","int");
  //reg.addProduct(prod);
  reg.addProduct(*intBranch_);

  //Should be 5 products
  // 1 from the original 'add' in this routine
  // 1 from 'one' responding to this call
  // 1 from 'two' responding to 'one'
  // 1 from 'two' responding to the original call
  // 1 from 'one' responding to 'two'
  CPPUNIT_ASSERT(5 * 2 == hear);
  CPPUNIT_ASSERT(5 == reg.registry().size());
}

void testProductRegistry::testProductRegistration() {
  edm::AssertHandler ah;

  std::string configuration(
      "import FWCore.ParameterSet.Config as cms\n"
      "process = cms.Process('TEST')\n"
      "process.maxEvents = cms.untracked.PSet(\n"
      "  input = cms.untracked.int32(-1))\n"
      "process.source = cms.Source('EmptySource')\n"
      "process.m1 = cms.EDProducer('TestPRegisterModule1')\n"
      "process.m2 = cms.EDProducer('TestPRegisterModule2')\n"
      "process.p = cms.Path(process.m1*process.m2)\n");
  try {
    edm::EventProcessor proc(edm::getPSetFromConfig(configuration));
  } catch (cms::Exception const& iException) {
    std::cout << "caught " << iException.explainSelf() << std::endl;
    throw;
  }
}

void testProductRegistry::testAddAlias() {
  edm::SignallingProductRegistryFiller reg;

  reg.addProduct(*intBranch_);
  reg.addLabelAlias(*intBranch_, "aliasi", "instanceAlias");

  reg.addProduct(*floatBranch_);
  reg.addLabelAlias(*floatBranch_, "aliasf", "instanceAlias");

  reg.addProduct(*intVecBranch_);
  reg.addLabelAlias(*intVecBranch_, "aliasvi", "instanceAlias");

  reg.addProduct(*simpleVecBranch_);
  reg.addLabelAlias(*simpleVecBranch_, "aliasovsimple", "instanceAlias");

  reg.addProduct(*simpleDerivedVecBranch_);
  reg.addLabelAlias(*simpleDerivedVecBranch_, "aliasovsimple", "instanceAlias");

  std::set<edm::TypeID> productTypesConsumed{intBranch_->unwrappedTypeID(),
                                             floatBranch_->unwrappedTypeID(),
                                             intVecBranch_->unwrappedTypeID(),
                                             simpleVecBranch_->unwrappedTypeID(),
                                             simpleDerivedVecBranch_->unwrappedTypeID()};
  std::set<edm::TypeID> elementTypesConsumed{intBranch_->unwrappedTypeID(), edm::TypeID(typeid(edmtest::Simple))};
  reg.setFrozen(productTypesConsumed, elementTypesConsumed, "TEST");
  {
    auto notFound =
        reg.registry().aliasToModules(edm::PRODUCT_TYPE, intBranch_->unwrappedTypeID(), "alias", "instance");
    CPPUNIT_ASSERT(notFound.empty());
  }
  {
    auto found =
        reg.registry().aliasToModules(edm::PRODUCT_TYPE, intBranch_->unwrappedTypeID(), "aliasi", "instanceAlias");
    CPPUNIT_ASSERT(found.size() == 1);
    CPPUNIT_ASSERT(found[0] == "labeli");
  }
  {
    auto found =
        reg.registry().aliasToModules(edm::PRODUCT_TYPE, floatBranch_->unwrappedTypeID(), "aliasf", "instanceAlias");
    CPPUNIT_ASSERT(found.size() == 1);
    CPPUNIT_ASSERT(found[0] == "labelf");
  }
  {
    auto found =
        reg.registry().aliasToModules(edm::PRODUCT_TYPE, intVecBranch_->unwrappedTypeID(), "aliasvi", "instanceAlias");
    CPPUNIT_ASSERT(found.size() == 1);
    CPPUNIT_ASSERT(found[0] == "labelvi");
  }
  {
    auto found =
        reg.registry().aliasToModules(edm::ELEMENT_TYPE, intBranch_->unwrappedTypeID(), "aliasvi", "instanceAlias");
    CPPUNIT_ASSERT(found.size() == 1);
    CPPUNIT_ASSERT(found[0] == "labelvi");
  }
  {
    auto found = reg.registry().aliasToModules(
        edm::ELEMENT_TYPE, edm::TypeID(typeid(edmtest::Simple)), "aliasovsimple", "instanceAlias");
    CPPUNIT_ASSERT(found.size() == 2);
    CPPUNIT_ASSERT(std::find(found.begin(), found.end(), "labelovsimple") != found.end());
    CPPUNIT_ASSERT(std::find(found.begin(), found.end(), "labelovsimplederived") != found.end());
  }
}
