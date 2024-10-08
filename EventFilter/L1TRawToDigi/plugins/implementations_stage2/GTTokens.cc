#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "GTTokens.h"

namespace l1t {
  namespace stage2 {
    GTTokens::GTTokens(const edm::ParameterSet& cfg, edm::ConsumesCollector& cc) {
      auto gttag = cfg.getParameter<edm::InputTag>("GtInputTag");
      auto exttag = cfg.getParameter<edm::InputTag>("ExtInputTag");
      auto egammatag = cfg.getParameter<edm::InputTag>("EGammaInputTag");
      auto jettag = cfg.getParameter<edm::InputTag>("JetInputTag");
      auto tautag = cfg.getParameter<edm::InputTag>("TauInputTag");
      auto etsumtag = cfg.getParameter<edm::InputTag>("EtSumInputTag");
      auto etsumzdctag = cfg.getParameter<edm::InputTag>("EtSumZDCInputTag");
      auto muontag = cfg.getParameter<edm::InputTag>("MuonInputTag");
      auto muonshowertag = cfg.getParameter<edm::InputTag>("ShowerInputLabel");
      auto cicadascoretag = cfg.getParameter<edm::InputTag>("CICADAScoreInputTag");

      //cout << "DEBUG:  GmtInputTag" <<  muontag << "\n";

      muonToken_ = cc.consumes<MuonBxCollection>(muontag);
      muonShowerToken_ = cc.consumes<MuonShowerBxCollection>(muonshowertag);
      egammaToken_ = cc.consumes<EGammaBxCollection>(egammatag);
      etSumToken_ = cc.consumes<EtSumBxCollection>(etsumtag);
      etSumZDCToken_ = cc.consumes<EtSumBxCollection>(etsumzdctag);
      jetToken_ = cc.consumes<JetBxCollection>(jettag);
      tauToken_ = cc.consumes<TauBxCollection>(tautag);
      algToken_ = cc.consumes<GlobalAlgBlkBxCollection>(gttag);
      extToken_ = cc.consumes<GlobalExtBlkBxCollection>(exttag);
      cicadaToken_ = cc.consumes<CICADABxCollection>(cicadascoretag);
    }
  }  // namespace stage2
}  // namespace l1t
