// -*- C++ -*-
//
// Package:     Subsystem/Package
// Class  :     TestProcessor
//
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Chris Jones
//         Created:  Mon, 30 Apr 2018 18:51:08 GMT
//

// system include files

// user include files
#include "FWCore/TestProcessor/interface/TestProcessor.h"
#include "FWCore/TestProcessor/interface/EventSetupTestHelper.h"

#include "FWCore/Common/interface/ProcessBlockHelper.h"
#include "FWCore/Concurrency/interface/FinalWaitingTask.h"
#include "FWCore/Concurrency/interface/WaitingTaskHolder.h"
#include "FWCore/Framework/interface/ScheduleItems.h"
#include "FWCore/Framework/interface/EventPrincipal.h"
#include "FWCore/Framework/interface/EventSetupProvider.h"
#include "FWCore/Framework/interface/LuminosityBlockPrincipal.h"
#include "FWCore/Framework/interface/ProcessBlockPrincipal.h"
#include "FWCore/Framework/interface/ExceptionActions.h"
#include "FWCore/Framework/interface/HistoryAppender.h"
#include "FWCore/Framework/interface/RunPrincipal.h"
#include "FWCore/Framework/interface/ESRecordsToProductResolverIndices.h"
#include "FWCore/Framework/interface/EventSetupsController.h"
#include "FWCore/Framework/interface/TransitionInfoTypes.h"
#include "FWCore/Framework/interface/ProductPutterBase.h"
#include "FWCore/Framework/interface/DelayedReader.h"
#include "FWCore/Framework/interface/ensureAvailableAccelerators.h"
#include "FWCore/Framework/interface/makeModuleTypeResolverMaker.h"
#include "FWCore/Framework/interface/FileBlock.h"
#include "FWCore/Framework/interface/MergeableRunProductMetadata.h"
#include "FWCore/Framework/interface/ProductResolversFactory.h"

#include "FWCore/ServiceRegistry/interface/ServiceRegistry.h"
#include "FWCore/ServiceRegistry/interface/SystemBounds.h"

#include "FWCore/ParameterSetReader/interface/ProcessDescImpl.h"
#include "FWCore/ParameterSet/interface/ProcessDesc.h"
#include "FWCore/ParameterSet/interface/validateTopLevelParameterSets.h"

#include "FWCore/Utilities/interface/ExceptionCollector.h"

#include "oneTimeInitialization.h"

#include <mutex>

#define xstr(s) str(s)
#define str(s) #s

namespace edm {
  namespace test {

    //
    // constructors and destructor
    //
    TestProcessor::TestProcessor(Config const& iConfig, ServiceToken iToken)
        : globalControl_(oneapi::tbb::global_control::max_allowed_parallelism, 1),
          arena_(1),
          historyAppender_(std::make_unique<HistoryAppender>()),
          moduleRegistry_(std::make_shared<ModuleRegistry>()) {
      //Setup various singletons
      (void)testprocessor::oneTimeInitialization();

      ProcessDescImpl desc(iConfig.pythonConfiguration(), false);

      auto psetPtr = desc.parameterSet();
      moduleTypeResolverMaker_ = makeModuleTypeResolverMaker(*psetPtr);
      espController_ = std::make_unique<eventsetup::EventSetupsController>(moduleTypeResolverMaker_.get());

      validateTopLevelParameterSets(psetPtr.get());

      ensureAvailableAccelerators(*psetPtr);

      labelOfTestModule_ = psetPtr->getParameter<std::string>("@moduleToTest");

      auto procDesc = desc.processDesc();
      // Now do general initialization
      ScheduleItems items;

      //initialize the services
      auto& serviceSets = procDesc->getServicesPSets();
      ServiceToken token = items.initServices(serviceSets, *psetPtr, iToken, serviceregistry::kOverlapIsError);
      serviceToken_ = items.addTNS(*psetPtr, token);

      //make the services available
      ServiceRegistry::Operate operate(serviceToken_);

      // intialize miscellaneous items
      std::shared_ptr<CommonParams> common(items.initMisc(*psetPtr));

      // intialize the event setup provider
      esp_ = espController_->makeProvider(*psetPtr, items.actReg_.get());

      auto nThreads = 1U;
      auto nStreams = 1U;
      auto nConcurrentLumis = 1U;
      auto nConcurrentRuns = 1U;
      preallocations_ = PreallocationConfiguration{nThreads, nStreams, nConcurrentLumis, nConcurrentRuns};

      if (not iConfig.esProduceEntries().empty()) {
        esHelper_ = std::make_unique<EventSetupTestHelper>(iConfig.esProduceEntries());
        esp_->add(std::dynamic_pointer_cast<eventsetup::ESProductResolverProvider>(esHelper_));
        esp_->add(std::dynamic_pointer_cast<EventSetupRecordIntervalFinder>(esHelper_));
      }

      auto tempReg = items.preg();
      processConfiguration_ = items.processConfiguration();

      edm::ParameterSet emptyPSet;
      emptyPSet.registerIt();
      auto psetid = emptyPSet.id();

      for (auto const& p : iConfig.extraProcesses()) {
        processHistory_.emplace_back(p, psetid, xstr(PROJECT_VERSION), HardwareResourcesDescription());
        processHistoryRegistry_.registerProcessHistory(processHistory_);
      }

      //setup the products we will be adding to the event
      for (auto const& produce : iConfig.produceEntries()) {
        auto processName = produce.processName_;
        if (processName.empty()) {
          processName = processConfiguration_->processName();
        }
        edm::TypeWithDict twd(produce.type_.typeInfo());
        edm::ProductDescription product(edm::InEvent,
                                        produce.moduleLabel_,
                                        processName,
                                        twd.userClassName(),
                                        twd.friendlyClassName(),
                                        produce.instanceLabel_,
                                        twd,
                                        true  //force this to come from 'source'
        );
        product.init();
        dataProducts_.emplace_back(product, std::unique_ptr<WrapperBase>());
        tempReg->addProduct(product);
      }

      processBlockHelper_ = std::make_shared<ProcessBlockHelper>();

      schedule_ = items.initSchedule(
          *psetPtr, preallocations_, &processContext_, moduleTypeResolverMaker_.get(), *processBlockHelper_);
      // set the data members
      act_table_ = std::move(items.act_table_);
      actReg_ = items.actReg_;
      branchIDListHelper_ = items.branchIDListHelper();
      thinnedAssociationsHelper_ = items.thinnedAssociationsHelper();
      processContext_.setProcessConfiguration(processConfiguration_.get());

      principalCache_.setNumberOfConcurrentPrincipals(preallocations_);

      tempReg->setFrozen();
      preg_ = std::make_shared<edm::ProductRegistry>(tempReg->moveTo());
      mergeableRunProductProcesses_.setProcessesWithMergeableRunProducts(*preg_);

      for (unsigned int index = 0; index < preallocations_.numberOfStreams(); ++index) {
        // Reusable event principal
        auto ep = std::make_shared<EventPrincipal>(preg_,
                                                   edm::productResolversFactory::makePrimary,
                                                   branchIDListHelper_,
                                                   thinnedAssociationsHelper_,
                                                   *processConfiguration_,
                                                   historyAppender_.get(),
                                                   index);
        principalCache_.insert(std::move(ep));
      }
      for (unsigned int index = 0; index < preallocations_.numberOfRuns(); ++index) {
        auto rp = std::make_unique<RunPrincipal>(preg_,
                                                 edm::productResolversFactory::makePrimary,
                                                 *processConfiguration_,
                                                 historyAppender_.get(),
                                                 index,
                                                 &mergeableRunProductProcesses_);
        principalCache_.insert(std::move(rp));
      }
      for (unsigned int index = 0; index < preallocations_.numberOfLuminosityBlocks(); ++index) {
        auto lp = std::make_unique<LuminosityBlockPrincipal>(
            preg_, edm::productResolversFactory::makePrimary, *processConfiguration_, historyAppender_.get(), index);
        principalCache_.insert(std::move(lp));
      }
      {
        auto pb = std::make_unique<ProcessBlockPrincipal>(
            preg_, edm::productResolversFactory::makePrimary, *processConfiguration_);
        principalCache_.insert(std::move(pb));
      }
    }

    TestProcessor::~TestProcessor() noexcept(false) { teardownProcessing(); }
    //
    // member functions
    //

    void TestProcessor::put(unsigned int index, std::unique_ptr<WrapperBase> iWrapper) {
      if (index >= dataProducts_.size()) {
        throw cms::Exception("LogicError") << "Products must be declared to the TestProcessor::Config object\n"
                                              "with a call to the function \'produces\' BEFORE passing the\n"
                                              "TestProcessor::Config object to the TestProcessor constructor";
      }
      dataProducts_[index].second = std::move(iWrapper);
    }

    edm::test::Event TestProcessor::testImpl() {
      bool result = arena_.execute([this]() {
        setupProcessing();
        event();

        return schedule_->totalEventsPassed() > 0;
      });
      schedule_->clearCounters();
      if (esHelper_) {
        //We want each test to have its own ES data products
        esHelper_->resetAllResolvers();
      }
      return edm::test::Event(
          principalCache_.eventPrincipal(0), labelOfTestModule_, processConfiguration_->processName(), result);
    }

    edm::test::LuminosityBlock TestProcessor::testBeginLuminosityBlockImpl(edm::LuminosityBlockNumber_t iNum) {
      arena_.execute([this, iNum]() {
        if (not beginJobCalled_) {
          beginJob();
        }
        if (not respondToOpenInputFileCalled_) {
          respondToOpenInputFile();
        }
        if (not beginProcessBlockCalled_) {
          beginProcessBlock();
        }
        if (not openOutputFilesCalled_) {
          openOutputFiles();
        }

        if (not beginRunCalled_) {
          beginRun();
        }
        if (beginLumiCalled_) {
          endLuminosityBlock();
          assert(lumiNumber_ != iNum);
        }
        lumiNumber_ = iNum;
        beginLuminosityBlock();
      });

      if (esHelper_) {
        //We want each test to have its own ES data products
        esHelper_->resetAllResolvers();
      }
      return edm::test::LuminosityBlock(lumiPrincipal_, labelOfTestModule_, processConfiguration_->processName());
    }

    edm::test::LuminosityBlock TestProcessor::testEndLuminosityBlockImpl() {
      //using a return value from arena_.execute lead to double delete of shared_ptr
      // based on valgrind output when exception occurred. Use lambda capture instead.
      std::shared_ptr<edm::LuminosityBlockPrincipal> lumi;
      arena_.execute([this, &lumi]() {
        if (not beginJobCalled_) {
          beginJob();
        }
        if (not respondToOpenInputFileCalled_) {
          respondToOpenInputFile();
        }
        if (not beginProcessBlockCalled_) {
          beginProcessBlock();
        }
        if (not openOutputFilesCalled_) {
          openOutputFiles();
        }
        if (not beginRunCalled_) {
          beginRun();
        }
        if (not beginLumiCalled_) {
          beginLuminosityBlock();
        }
        lumi = endLuminosityBlock();
      });
      if (esHelper_) {
        //We want each test to have its own ES data products
        esHelper_->resetAllResolvers();
      }

      return edm::test::LuminosityBlock(std::move(lumi), labelOfTestModule_, processConfiguration_->processName());
    }

    edm::test::Run TestProcessor::testBeginRunImpl(edm::RunNumber_t iNum) {
      arena_.execute([this, iNum]() {
        if (not beginJobCalled_) {
          beginJob();
        }
        if (not respondToOpenInputFileCalled_) {
          respondToOpenInputFile();
        }
        if (not beginProcessBlockCalled_) {
          beginProcessBlock();
        }
        if (not openOutputFilesCalled_) {
          openOutputFiles();
        }
        if (beginRunCalled_) {
          assert(runNumber_ != iNum);
          endRun();
        }
        runNumber_ = iNum;
        beginRun();
      });
      if (esHelper_) {
        //We want each test to have its own ES data products
        esHelper_->resetAllResolvers();
      }
      return edm::test::Run(runPrincipal_, labelOfTestModule_, processConfiguration_->processName());
    }
    edm::test::Run TestProcessor::testEndRunImpl() {
      //using a return value from arena_.execute lead to double delete of shared_ptr
      // based on valgrind output when exception occurred. Use lambda capture instead.
      std::shared_ptr<edm::RunPrincipal> rp;
      arena_.execute([this, &rp]() {
        if (not beginJobCalled_) {
          beginJob();
        }
        if (not respondToOpenInputFileCalled_) {
          respondToOpenInputFile();
        }
        if (not beginProcessBlockCalled_) {
          beginProcessBlock();
        }
        if (not openOutputFilesCalled_) {
          openOutputFiles();
        }
        if (not beginRunCalled_) {
          beginRun();
        }
        rp = endRun();
      });
      if (esHelper_) {
        //We want each test to have its own ES data products
        esHelper_->resetAllResolvers();
      }

      return edm::test::Run(rp, labelOfTestModule_, processConfiguration_->processName());
    }

    edm::test::ProcessBlock TestProcessor::testBeginProcessBlockImpl() {
      arena_.execute([this]() {
        if (not beginJobCalled_) {
          beginJob();
        }
        beginProcessBlock();
      });
      return edm::test::ProcessBlock(
          &principalCache_.processBlockPrincipal(), labelOfTestModule_, processConfiguration_->processName());
    }
    edm::test::ProcessBlock TestProcessor::testEndProcessBlockImpl() {
      auto pbp = arena_.execute([this]() {
        if (not beginJobCalled_) {
          beginJob();
        }
        if (not beginProcessBlockCalled_) {
          beginProcessBlock();
        }
        return endProcessBlock();
      });
      return edm::test::ProcessBlock(pbp, labelOfTestModule_, processConfiguration_->processName());
    }

    void TestProcessor::setupProcessing() {
      if (not beginJobCalled_) {
        beginJob();
      }
      if (not respondToOpenInputFileCalled_) {
        respondToOpenInputFile();
      }
      if (not beginProcessBlockCalled_) {
        beginProcessBlock();
      }
      if (not openOutputFilesCalled_) {
        openOutputFiles();
      }
      if (not beginRunCalled_) {
        beginRun();
      }
      if (not beginLumiCalled_) {
        beginLuminosityBlock();
      }
    }

    void TestProcessor::teardownProcessing() {
      arena_.execute([this]() {
        if (beginLumiCalled_) {
          endLuminosityBlock();
          beginLumiCalled_ = false;
        }
        if (beginRunCalled_) {
          endRun();
          beginRunCalled_ = false;
        }
        if (respondToOpenInputFileCalled_) {
          respondToCloseInputFile();
        }
        if (beginProcessBlockCalled_) {
          endProcessBlock();
          beginProcessBlockCalled_ = false;
        }
        if (openOutputFilesCalled_) {
          closeOutputFiles();
          openOutputFilesCalled_ = false;
        }
        if (beginJobCalled_) {
          endJob();
        }
        edm::FinalWaitingTask task{taskGroup_};
        espController_->endIOVsAsync(edm::WaitingTaskHolder(taskGroup_, &task));
        task.waitNoThrow();
      });
    }

    void TestProcessor::beginJob() {
      ServiceRegistry::Operate operate(serviceToken_);

      service::SystemBounds bounds(preallocations_.numberOfStreams(),
                                   preallocations_.numberOfLuminosityBlocks(),
                                   preallocations_.numberOfRuns(),
                                   preallocations_.numberOfThreads());
      actReg_->preallocateSignal_(bounds);
      schedule_->convertCurrentProcessAlias(processConfiguration_->processName());

      espController_->finishConfiguration();
      actReg_->eventSetupConfigurationSignal_(esp_->recordsToResolverIndices(), processContext_);

      schedule_->beginJob(
          *preg_, esp_->recordsToResolverIndices(), *processBlockHelper_, processContext_.processName());

      for (unsigned int i = 0; i < preallocations_.numberOfStreams(); ++i) {
        schedule_->beginStream(i);
      }
      beginJobCalled_ = true;
    }

    void TestProcessor::beginProcessBlock() {
      ProcessBlockPrincipal& processBlockPrincipal = principalCache_.processBlockPrincipal();
      processBlockPrincipal.fillProcessBlockPrincipal(processConfiguration_->processName());

      ProcessBlockTransitionInfo transitionInfo(processBlockPrincipal);
      using Traits = OccurrenceTraits<ProcessBlockPrincipal, BranchActionGlobalBegin>;
      processGlobalTransition<Traits>(transitionInfo);

      beginProcessBlockCalled_ = true;
    }

    void TestProcessor::openOutputFiles() {
      //make the services available
      ServiceRegistry::Operate operate(serviceToken_);

      edm::FileBlock fb;
      schedule_->openOutputFiles(fb);
      openOutputFilesCalled_ = true;
    }

    void TestProcessor::closeOutputFiles() {
      if (openOutputFilesCalled_) {
        //make the services available
        ServiceRegistry::Operate operate(serviceToken_);
        schedule_->closeOutputFiles();

        openOutputFilesCalled_ = false;
      }
    }

    void TestProcessor::respondToOpenInputFile() {
      respondToOpenInputFileCalled_ = true;
      edm::FileBlock fb;
      //make the services available
      ServiceRegistry::Operate operate(serviceToken_);
      schedule_->respondToOpenInputFile(fb);
    }

    void TestProcessor::respondToCloseInputFile() {
      if (respondToOpenInputFileCalled_) {
        edm::FileBlock fb;
        //make the services available
        ServiceRegistry::Operate operate(serviceToken_);

        schedule_->respondToCloseInputFile(fb);
        respondToOpenInputFileCalled_ = false;
      }
    }

    void TestProcessor::beginRun() {
      runPrincipal_ = principalCache_.getAvailableRunPrincipalPtr();
      runPrincipal_->clearPrincipal();
      assert(runPrincipal_);
      edm::RunAuxiliary aux(runNumber_, Timestamp(), Timestamp());
      aux.setProcessHistoryID(processHistory_.id());
      runPrincipal_->setAux(aux);

      runPrincipal_->fillRunPrincipal(processHistoryRegistry_);

      IOVSyncValue ts(EventID(runPrincipal_->run(), 0, 0), runPrincipal_->beginTime());
      eventsetup::synchronousEventSetupForInstance(ts, taskGroup_, *espController_);

      auto const& es = esp_->eventSetupImpl();

      RunTransitionInfo transitionInfo(*runPrincipal_, es);
      {
        using Traits = OccurrenceTraits<RunPrincipal, BranchActionGlobalBegin>;
        processGlobalTransition<Traits>(transitionInfo);
      }
      {
        using Traits = OccurrenceTraits<RunPrincipal, BranchActionStreamBegin>;
        processTransitionForAllStreams<Traits>(transitionInfo);
      }
      beginRunCalled_ = true;
    }

    void TestProcessor::beginLuminosityBlock() {
      LuminosityBlockAuxiliary aux(runNumber_, lumiNumber_, Timestamp(), Timestamp());
      aux.setProcessHistoryID(processHistory_.id());
      lumiPrincipal_ = principalCache_.getAvailableLumiPrincipalPtr();
      lumiPrincipal_->clearPrincipal();
      assert(lumiPrincipal_);
      lumiPrincipal_->setAux(aux);

      lumiPrincipal_->setRunPrincipal(runPrincipal_);
      lumiPrincipal_->fillLuminosityBlockPrincipal(&processHistory_);

      IOVSyncValue ts(EventID(runNumber_, lumiNumber_, eventNumber_), lumiPrincipal_->beginTime());
      eventsetup::synchronousEventSetupForInstance(ts, taskGroup_, *espController_);

      auto const& es = esp_->eventSetupImpl();

      LumiTransitionInfo transitionInfo(*lumiPrincipal_, es);

      {
        using Traits = OccurrenceTraits<LuminosityBlockPrincipal, BranchActionGlobalBegin>;
        processGlobalTransition<Traits>(transitionInfo);
      }
      {
        using Traits = OccurrenceTraits<LuminosityBlockPrincipal, BranchActionStreamBegin>;
        processTransitionForAllStreams<Traits>(transitionInfo);
      }
      beginLumiCalled_ = true;
    }

    void TestProcessor::event() {
      auto pep = &(principalCache_.eventPrincipal(0));

      //this resets the EventPrincipal (if it had been used before)
      pep->clearEventPrincipal();
      edm::EventAuxiliary aux(EventID(runNumber_, lumiNumber_, eventNumber_), "", Timestamp(), false);
      aux.setProcessHistoryID(processHistory_.id());
      pep->fillEventPrincipal(aux, nullptr, nullptr);
      assert(lumiPrincipal_.get() != nullptr);
      pep->setLuminosityBlockPrincipal(lumiPrincipal_.get());

      for (auto& p : dataProducts_) {
        if (p.second) {
          pep->put(p.first, std::move(p.second), ProductProvenance(p.first.branchID()));
        } else {
          //The data product was not set so we need to
          // tell the ProductResolver not to wait
          auto r = pep->getProductResolver(p.first.branchID());
          dynamic_cast<ProductPutterBase const*>(r)->putProduct(std::unique_ptr<WrapperBase>());
        }
      }

      ServiceRegistry::Operate operate(serviceToken_);

      FinalWaitingTask waitTask{taskGroup_};

      EventTransitionInfo info(*pep, esp_->eventSetupImpl());
      schedule_->processOneEventAsync(edm::WaitingTaskHolder(taskGroup_, &waitTask), 0, info, serviceToken_);

      waitTask.wait();
      ++eventNumber_;
    }

    std::shared_ptr<LuminosityBlockPrincipal> TestProcessor::endLuminosityBlock() {
      auto lumiPrincipal = lumiPrincipal_;
      if (beginLumiCalled_) {
        //make the services available
        ServiceRegistry::Operate operate(serviceToken_);

        beginLumiCalled_ = false;
        lumiPrincipal_.reset();

        IOVSyncValue ts(EventID(runNumber_, lumiNumber_, eventNumber_), lumiPrincipal->endTime());
        eventsetup::synchronousEventSetupForInstance(ts, taskGroup_, *espController_);

        auto const& es = esp_->eventSetupImpl();

        LumiTransitionInfo transitionInfo(*lumiPrincipal, es);

        {
          using Traits = OccurrenceTraits<LuminosityBlockPrincipal, BranchActionStreamEnd>;
          processTransitionForAllStreams<Traits>(transitionInfo);
        }
        {
          using Traits = OccurrenceTraits<LuminosityBlockPrincipal, BranchActionGlobalEnd>;
          processGlobalTransition<Traits>(transitionInfo);
        }
        {
          FinalWaitingTask globalWaitTask{taskGroup_};
          schedule_->writeLumiAsync(
              WaitingTaskHolder(taskGroup_, &globalWaitTask), *lumiPrincipal, &processContext_, actReg_.get());
          globalWaitTask.wait();
        }
      }
      lumiPrincipal->setRunPrincipal(std::shared_ptr<RunPrincipal>());
      return lumiPrincipal;
    }

    std::shared_ptr<edm::RunPrincipal> TestProcessor::endRun() {
      auto runPrincipal = runPrincipal_;
      runPrincipal_.reset();
      if (beginRunCalled_) {
        beginRunCalled_ = false;

        //make the services available
        ServiceRegistry::Operate operate(serviceToken_);

        IOVSyncValue ts(
            EventID(runPrincipal->run(), LuminosityBlockID::maxLuminosityBlockNumber(), EventID::maxEventNumber()),
            runPrincipal->endTime());
        eventsetup::synchronousEventSetupForInstance(ts, taskGroup_, *espController_);

        auto const& es = esp_->eventSetupImpl();

        RunTransitionInfo transitionInfo(*runPrincipal, es);

        {
          using Traits = OccurrenceTraits<RunPrincipal, BranchActionStreamEnd>;
          processTransitionForAllStreams<Traits>(transitionInfo);
        }
        {
          using Traits = OccurrenceTraits<RunPrincipal, BranchActionGlobalEnd>;
          processGlobalTransition<Traits>(transitionInfo);
        }
        {
          FinalWaitingTask globalWaitTask{taskGroup_};
          schedule_->writeRunAsync(WaitingTaskHolder(taskGroup_, &globalWaitTask),
                                   *runPrincipal,
                                   &processContext_,
                                   actReg_.get(),
                                   runPrincipal->mergeableRunProductMetadata());
          globalWaitTask.wait();
        }
      }
      return runPrincipal;
    }

    ProcessBlockPrincipal const* TestProcessor::endProcessBlock() {
      ProcessBlockPrincipal& processBlockPrincipal = principalCache_.processBlockPrincipal();
      if (beginProcessBlockCalled_) {
        beginProcessBlockCalled_ = false;

        ProcessBlockTransitionInfo transitionInfo(processBlockPrincipal);
        using Traits = OccurrenceTraits<ProcessBlockPrincipal, BranchActionGlobalEnd>;
        processGlobalTransition<Traits>(transitionInfo);
      }
      return &processBlockPrincipal;
    }

    void TestProcessor::endJob() {
      if (!beginJobCalled_) {
        return;
      }
      beginJobCalled_ = false;

      // Collects exceptions, so we don't throw before all operations are performed.
      ExceptionCollector c(
          "Multiple exceptions were thrown while executing endJob. An exception message follows for each.\n");
      std::mutex collectorMutex;

      //make the services available
      ServiceRegistry::Operate operate(serviceToken_);

      //NOTE: this really should go elsewhere in the future
      for (unsigned int i = 0; i < preallocations_.numberOfStreams(); ++i) {
        schedule_->endStream(i, c, collectorMutex);
      }
      auto actReg = actReg_.get();
      c.call([actReg]() { actReg->preEndJobSignal_(); });
      schedule_->endJob(c);
      c.call([actReg]() { actReg->postEndJobSignal_(); });
      if (c.hasThrown()) {
        c.rethrow();
      }
    }

    void TestProcessor::setRunNumber(edm::RunNumber_t iRun) {
      if (beginRunCalled_) {
        endLuminosityBlock();
        endRun();
      }
      runNumber_ = iRun;
    }
    void TestProcessor::setLuminosityBlockNumber(edm::LuminosityBlockNumber_t iLumi) {
      endLuminosityBlock();
      lumiNumber_ = iLumi;
    }

    void TestProcessor::setEventNumber(edm::EventNumber_t iEv) { eventNumber_ = iEv; }

    template <typename Traits>
    void TestProcessor::processTransitionForAllStreams(typename Traits::TransitionInfoType& transitionInfo) {
      FinalWaitingTask finalWaitTask{taskGroup_};
      {
        WaitingTaskHolder holder(taskGroup_, &finalWaitTask);
        // Currently numberOfStreams is always one in TestProcessor and this for loop is unnecessary...
        for (unsigned int i = 0; i < preallocations_.numberOfStreams(); ++i) {
          schedule_->processOneStreamAsync<Traits>(holder, i, transitionInfo, serviceToken_);
        }
      }
      finalWaitTask.wait();
    }

    template <typename Traits>
    void TestProcessor::processGlobalTransition(typename Traits::TransitionInfoType& transitionInfo) {
      FinalWaitingTask finalWaitTask{taskGroup_};
      schedule_->processOneGlobalAsync<Traits>(
          WaitingTaskHolder(taskGroup_, &finalWaitTask), transitionInfo, serviceToken_);
      finalWaitTask.wait();
    }

  }  // namespace test
}  // namespace edm
