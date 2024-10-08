#ifndef SiStripCommissioningSource_SiStripFineDelayHit_h
#define SiStripCommissioningSource_SiStripFineDelayHit_h

// system include files
#include <memory>
#include <utility>

// user include files
#include "CondFormats/DataRecord/interface/SiStripFedCablingRcd.h"
#include "CondFormats/DataRecord/interface/SiStripNoisesRcd.h"
#include "CondFormats/SiStripObjects/interface/SiStripFedCabling.h"
#include "CondFormats/SiStripObjects/interface/SiStripNoises.h"
#include "DQM/SiStripCommissioningSources/plugins/tracking/SiStripFineDelayTLA.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/Common/interface/Ref.h"
#include "DataFormats/SiStripCluster/interface/SiStripCluster.h"
#include "DataFormats/SiStripCluster/interface/SiStripCluster.h"
#include "DataFormats/SiStripCluster/interface/SiStripClusterCollection.h"
#include "DataFormats/SiStripCluster/interface/SiStripClusterCollection.h"
#include "DataFormats/SiStripCommon/interface/SiStripEventSummary.h"
#include "DataFormats/SiStripDigi/interface/SiStripDigi.h"
#include "DataFormats/SiStripDigi/interface/SiStripRawDigi.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "DataFormats/TrajectorySeed/interface/TrajectorySeedCollection.h"
#include "FWCore/Framework/interface/one/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "TrackingTools/PatternTools/interface/Trajectory.h"

//
// class decleration
//

class SiStripFineDelayHit : public edm::one::EDProducer<edm::one::WatchRuns> {
public:
  explicit SiStripFineDelayHit(const edm::ParameterSet&);
  ~SiStripFineDelayHit() override;

private:
  void beginRun(const edm::Run&, const edm::EventSetup&) override;
  void produce(edm::Event&, const edm::EventSetup&) override;
  void endRun(const edm::Run&, const edm::EventSetup&) override {}
  virtual void produceNoTracking(edm::Event&, const edm::EventSetup&);
  using DeviceMask = std::pair<uint32_t, uint32_t>;
  DeviceMask deviceMask(const StripSubdetector::SubDetector subdet,
                        const int substructure,
                        const TrackerTopology* tkrTopo);
  std::vector<std::pair<uint32_t, std::pair<double, double> > > detId(
      const TrackerGeometry& tracker,
      const TrackerTopology* tkrTopo,
      const reco::Track* tk,
      const std::vector<Trajectory>& trajVec,
      const StripSubdetector::SubDetector subdet = StripSubdetector::TIB,
      const int substructure = 0xff);
  std::vector<std::pair<uint32_t, std::pair<double, double> > > detId(const TrackerGeometry& tracker,
                                                                      const TrackerTopology* tkrTopo,
                                                                      const reco::Track* tk,
                                                                      const std::vector<Trajectory>& trajVec,
                                                                      const uint32_t& maskDetId,
                                                                      const uint32_t& rootDetId);
  std::pair<const SiStripCluster*, double> closestCluster(const TrackerGeometry& tracker,
                                                          const reco::Track* tk,
                                                          const uint32_t& detId,
                                                          const edmNew::DetSetVector<SiStripCluster>& clusters,
                                                          const edm::DetSetVector<SiStripDigi>& hits);
  bool rechit(reco::Track* tk, uint32_t detId);

  // ----------member data ---------------------------
  SiStripFineDelayTLA* anglefinder_;
  const edm::Event* event_;
  bool cosmic_, field_, homeMadeClusters_, noTracking_;
  double maxAngle_, minTrackP2_, maxClusterDistance_;
  int mode_;  // 1=delayScan 2=latencyScan
  int explorationWindow_;
  //      edm::InputTag digiLabel_, clusterLabel_, trackLabel_, seedLabel_, inputModuleLabel_;
  edm::EDGetTokenT<TrajectorySeedCollection> seedcollToken_;
  edm::EDGetTokenT<SiStripEventSummary> inputModuleToken_;
  edm::EDGetTokenT<reco::TrackCollection> trackCollectionToken_;
  edm::EDGetTokenT<edm::DetSetVector<SiStripDigi> > digiToken_;
  edm::EDGetTokenT<edmNew::DetSetVector<SiStripCluster> > clustersToken_;
  edm::EDGetTokenT<std::vector<Trajectory> > trackToken_;
  std::map<uint32_t, uint32_t> connectionMap_;

  edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> tkGeomToken_;
  edm::ESGetToken<TrackerTopology, TrackerTopologyRcd> tTopoToken_;
  edm::ESGetToken<SiStripFedCabling, SiStripFedCablingRcd> fedCablingToken_;
  edm::ESGetToken<SiStripNoises, SiStripNoisesRcd> noiseToken_;
};

#endif
