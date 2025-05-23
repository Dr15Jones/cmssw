#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Framework/interface/ProducesCollector.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "SimGeneral/MixingModule/interface/PileUpEventPrincipal.h"

#include "FWCore/AbstractServices/interface/RandomNumberGenerator.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/FTLDigi/interface/PMTDSimAccumulator.h"
#include "SimFastTiming/FastTimingCommon/interface/MTDDigitizerBase.h"

#include "SimGeneral/PreMixingModule/interface/PreMixingWorker.h"
#include "SimGeneral/PreMixingModule/interface/PreMixingWorkerFactory.h"

class PreMixingMTDWorker : public PreMixingWorker {
public:
  PreMixingMTDWorker(const edm::ParameterSet& ps, edm::ProducesCollector, edm::ConsumesCollector&& iC);
  ~PreMixingMTDWorker() override = default;

  PreMixingMTDWorker(const PreMixingMTDWorker&) = delete;
  PreMixingMTDWorker& operator=(const PreMixingMTDWorker&) = delete;

  void initializeEvent(const edm::Event& e, const edm::EventSetup& ES) override;
  void addSignals(const edm::Event& e, const edm::EventSetup& ES) override;
  void addPileups(const PileUpEventPrincipal&, const edm::EventSetup& ES) override;
  void put(edm::Event& e, const edm::EventSetup& ES, std::vector<PileupSummaryInfo> const& ps, int bs) override;

private:
  edm::EDGetTokenT<PMTDSimAccumulator> signalToken_;

  edm::InputTag pileInputTag_;

  std::unique_ptr<MTDDigitizerBase> digitizer_;
};

PreMixingMTDWorker::PreMixingMTDWorker(const edm::ParameterSet& ps,
                                       edm::ProducesCollector producesCollector,
                                       edm::ConsumesCollector&& iC)
    : signalToken_(iC.consumes<PMTDSimAccumulator>(ps.getParameter<edm::InputTag>("digiTagSig"))),
      pileInputTag_(ps.getParameter<edm::InputTag>("pileInputTag")),
      digitizer_(MTDDigitizerFactory::get()->create(
          ps.getParameter<std::string>("digitizerName"), ps, producesCollector, iC)) {}

void PreMixingMTDWorker::initializeEvent(const edm::Event& e, const edm::EventSetup& ES) {
  digitizer_->initializeEvent(e, ES);
}

void PreMixingMTDWorker::addSignals(const edm::Event& e, const edm::EventSetup& ES) {
  edm::Handle<PMTDSimAccumulator> handle;
  e.getByToken(signalToken_, handle);
  if (handle.isValid())
    digitizer_->accumulate(*handle);
  else
    edm::LogWarning("MtdMix") << "PreMixingMTDWorker::Cannot find Signal collection";
}

void PreMixingMTDWorker::addPileups(const PileUpEventPrincipal& pep, const edm::EventSetup& ES) {
  edm::Handle<PMTDSimAccumulator> handle;
  pep.getByLabel(pileInputTag_, handle);
  if (handle.isValid())
    digitizer_->accumulate(*handle);
  else
    edm::LogWarning("MtdMix") << "PreMixingMTDWorker::Cannot find PileUp collection";
}

void PreMixingMTDWorker::put(edm::Event& e,
                             const edm::EventSetup& ES,
                             std::vector<PileupSummaryInfo> const& ps,
                             int bs) {
  edm::Service<edm::RandomNumberGenerator> rng;
  digitizer_->finalizeEvent(e, ES, &rng->getEngine(e.streamID()));
}

DEFINE_PREMIXING_WORKER(PreMixingMTDWorker);
