/**
  \class    pat::OutsideInMuonSeeder MuonReSeeder.h "MuonAnalysis/MuonAssociators/interface/MuonReSeeder.h"
  \brief    Matcher of reconstructed objects to other reconstructed objects using the tracks inside them

  \author   Giovanni Petrucciani
*/

#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "Geometry/CommonDetUnit/interface/GlobalTrackingGeometry.h"
#include "Geometry/Records/interface/GlobalTrackingGeometryRecord.h"
#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"
#include "RecoTracker/MeasurementDet/interface/MeasurementTracker.h"
#include "RecoTracker/MeasurementDet/interface/MeasurementTrackerEvent.h"
#include "RecoTracker/TkDetLayers/interface/GeometricSearchTracker.h"
#include "TrackingTools/GeomPropagators/interface/Propagator.h"
#include "TrackingTools/KalmanUpdators/interface/Chi2MeasurementEstimatorBase.h"
#include "TrackingTools/KalmanUpdators/interface/KFUpdator.h"
#include "TrackingTools/MeasurementDet/interface/MeasurementDet.h"
#include "TrackingTools/PatternTools/interface/TrajMeasLessEstim.h"
#include "TrackingTools/PatternTools/interface/TrajectoryMeasurement.h"
#include "TrackingTools/PatternTools/interface/TrajectoryStateUpdator.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"
#include "TrackingTools/TrackRefitter/interface/TrackTransformer.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateOnSurface.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"

class OutsideInMuonSeeder final : public edm::stream::EDProducer<> {
public:
  explicit OutsideInMuonSeeder(const edm::ParameterSet &iConfig);
  ~OutsideInMuonSeeder() override = default;

  void produce(edm::Event &iEvent, const edm::EventSetup &iSetup) override;

  static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
  /// Labels for input collections
  edm::EDGetTokenT<edm::View<reco::Muon>> src_;

  /// Muon selection
  StringCutObjectSelector<reco::Muon> selector_;

  /// How many layers to try
  const int layersToTry_;

  /// How many hits to try on same layer
  const int hitsToTry_;

  /// Do inside-out
  const bool fromVertex_;

  /// How much to rescale errors from STA
  const double errorRescaling_;

  const edm::ESGetToken<Propagator, TrackingComponentsRecord> trackerPropagatorToken_;
  const edm::ESGetToken<Propagator, TrackingComponentsRecord> muonPropagatorToken_;
  const edm::EDGetTokenT<MeasurementTrackerEvent> measurementTrackerTag_;
  const edm::ESGetToken<Chi2MeasurementEstimatorBase, TrackingComponentsRecord> estimatorToken_;
  const edm::ESGetToken<TrajectoryStateUpdator, TrackingComponentsRecord> updatorToken_;
  const edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> magfieldToken_;
  const edm::ESGetToken<GlobalTrackingGeometry, GlobalTrackingGeometryRecord> geometryToken_;
  const edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> tkGeometryToken_;

  float const minEtaForTEC_;
  float const maxEtaForTOB_;

  const MagneticField *magfield_;
  const Propagator *muonPropagator_;
  const GlobalTrackingGeometry *geometry_;
  const Chi2MeasurementEstimatorBase *estimator_;
  const TrajectoryStateUpdator *updator_;

  /// Dump deug information
  const bool debug_;

  int doLayer(const GeometricSearchDet &layer,
              const TrajectoryStateOnSurface &state,
              std::vector<TrajectorySeed> &out,
              const Propagator &muon_propagator,
              const Propagator &tracker_propagator,
              const MeasurementTrackerEvent &mte) const;
  void doDebug(const reco::Track &tk) const;
};

OutsideInMuonSeeder::OutsideInMuonSeeder(const edm::ParameterSet &iConfig)
    : src_(consumes<edm::View<reco::Muon>>(iConfig.getParameter<edm::InputTag>("src"))),
      selector_(iConfig.existsAs<std::string>("cut") ? iConfig.getParameter<std::string>("cut") : "", true),
      layersToTry_(iConfig.getParameter<int32_t>("layersToTry")),
      hitsToTry_(iConfig.getParameter<int32_t>("hitsToTry")),
      fromVertex_(iConfig.getParameter<bool>("fromVertex")),
      errorRescaling_(iConfig.getParameter<double>("errorRescaleFactor")),
      trackerPropagatorToken_(esConsumes(edm::ESInputTag("", iConfig.getParameter<std::string>("trackerPropagator")))),
      muonPropagatorToken_(esConsumes(edm::ESInputTag("", iConfig.getParameter<std::string>("muonPropagator")))),
      measurementTrackerTag_(
          consumes<MeasurementTrackerEvent>(iConfig.getParameter<edm::InputTag>("measurementTkEvent"))),
      estimatorToken_(esConsumes(edm::ESInputTag("", iConfig.getParameter<std::string>("hitCollector")))),
      updatorToken_(esConsumes(edm::ESInputTag("", iConfig.getParameter<std::string>("updatorLabel")))),
      magfieldToken_(esConsumes()),
      geometryToken_(esConsumes()),
      tkGeometryToken_(esConsumes()),
      minEtaForTEC_(iConfig.getParameter<double>("minEtaForTEC")),
      maxEtaForTOB_(iConfig.getParameter<double>("maxEtaForTOB")),
      debug_(iConfig.getUntrackedParameter<bool>("debug", false)) {
  produces<std::vector<TrajectorySeed>>();
}

void OutsideInMuonSeeder::produce(edm::Event &iEvent, const edm::EventSetup &iSetup) {
  using namespace edm;
  using namespace std;

  magfield_ = &iSetup.getData(magfieldToken_);
  auto const &trackerPropagator = iSetup.getData(trackerPropagatorToken_);
  muonPropagator_ = &iSetup.getData(muonPropagatorToken_);
  geometry_ = &iSetup.getData(geometryToken_);
  estimator_ = &iSetup.getData(estimatorToken_);
  updator_ = &iSetup.getData(updatorToken_);

  Handle<MeasurementTrackerEvent> measurementTracker;
  iEvent.getByToken(measurementTrackerTag_, measurementTracker);

  const auto &tmpTkGeometry = iSetup.getData(tkGeometryToken_);

  Handle<View<reco::Muon>> src;
  iEvent.getByToken(src_, src);

  auto out = std::make_unique<std::vector<TrajectorySeed>>();

  for (auto const &mu : *src) {
    if (mu.outerTrack().isNull() || !selector_(mu))
      continue;
    if (debug_ && mu.innerTrack().isNonnull())
      doDebug(*mu.innerTrack());

    // Better clone here and not directly into doLayer to avoid
    // useless clone/destroy operations to set, in the end, the
    // very same direction every single time.
    std::unique_ptr<Propagator> pmuon_cloned =
        SetPropagationDirection(*muonPropagator_, fromVertex_ ? alongMomentum : oppositeToMomentum);
    std::unique_ptr<Propagator> ptracker_cloned = SetPropagationDirection(trackerPropagator, alongMomentum);

    int sizeBefore = out->size();
    if (debug_)
      LogDebug("OutsideInMuonSeeder") << "\n\n\nSeeding for muon of pt " << mu.pt() << ", eta " << mu.eta() << ", phi "
                                      << mu.phi() << std::endl;
    const reco::Track &tk = *mu.outerTrack();

    TrajectoryStateOnSurface state =
        fromVertex_ ? TrajectoryStateOnSurface(trajectoryStateTransform::initialFreeState(tk, magfield_))
                    : trajectoryStateTransform::innerStateOnSurface(tk, *geometry_, magfield_);

    if (std::abs(tk.eta()) < maxEtaForTOB_) {
      std::vector<BarrelDetLayer const *> const &tob = measurementTracker->geometricSearchTracker()->tobLayers();
      int found = 0;
      int iLayer = tob.size();
      if (iLayer == 0)
        LogError("OutsideInMuonSeeder") << "TOB has no layers.";

      for (auto it = tob.rbegin(), ed = tob.rend(); it != ed; ++it, --iLayer) {
        if (debug_)
          LogDebug("OutsideInMuonSeeder") << "\n ==== Trying TOB " << iLayer << " ====" << std::endl;
        if (doLayer(**it, state, *out, *(pmuon_cloned.get()), *(ptracker_cloned.get()), *measurementTracker)) {
          if (++found == layersToTry_)
            break;
        }
      }
    }
    if (tk.eta() > minEtaForTEC_) {
      const auto &forwLayers = tmpTkGeometry.isThere(GeomDetEnumerators::P2OTEC)
                                   ? measurementTracker->geometricSearchTracker()->posTidLayers()
                                   : measurementTracker->geometricSearchTracker()->posTecLayers();
      if (tmpTkGeometry.isThere(GeomDetEnumerators::P2OTEC)) {
        LogDebug("OutsideInMuonSeeder") << "\n We are using the Phase2 Outer Tracker (defined as a TID+). ";
      }
      LogTrace("OutsideInMuonSeeder") << "\n ==== TEC+ tot layers " << forwLayers.size() << " ====" << std::endl;
      int found = 0;
      int iLayer = forwLayers.size();
      if (iLayer == 0)
        LogError("OutsideInMuonSeeder") << "TEC+ has no layers.";

      if (debug_)
        LogDebug("OutsideInMuonSeeder") << "\n ==== Tot layers " << forwLayers.size() << " ====" << std::endl;
      for (auto it = forwLayers.rbegin(), ed = forwLayers.rend(); it != ed; ++it, --iLayer) {
        if (debug_)
          LogDebug("OutsideInMuonSeeder") << "\n ==== Trying Forward Layer +" << +iLayer << " ====" << std::endl;
        if (doLayer(**it, state, *out, *(pmuon_cloned.get()), *(ptracker_cloned.get()), *measurementTracker)) {
          if (++found == layersToTry_)
            break;
        }
      }
    }
    if (tk.eta() < -minEtaForTEC_) {
      const auto &forwLayers = tmpTkGeometry.isThere(GeomDetEnumerators::P2OTEC)
                                   ? measurementTracker->geometricSearchTracker()->negTidLayers()
                                   : measurementTracker->geometricSearchTracker()->negTecLayers();
      if (tmpTkGeometry.isThere(GeomDetEnumerators::P2OTEC)) {
        LogDebug("OutsideInMuonSeeder") << "\n We are using the Phase2 Outer Tracker (defined as a TID-). ";
      }
      LogTrace("OutsideInMuonSeeder") << "\n ==== TEC- tot layers " << forwLayers.size() << " ====" << std::endl;
      int found = 0;
      int iLayer = forwLayers.size();
      if (iLayer == 0)
        LogError("OutsideInMuonSeeder") << "TEC- has no layers.";

      if (debug_)
        LogDebug("OutsideInMuonSeeder") << "\n ==== Tot layers " << forwLayers.size() << " ====" << std::endl;
      for (auto it = forwLayers.rbegin(), ed = forwLayers.rend(); it != ed; ++it, --iLayer) {
        if (debug_)
          LogDebug("OutsideInMuonSeeder") << "\n ==== Trying Forward Layer -" << -iLayer << " ====" << std::endl;
        if (doLayer(**it, state, *out, *(pmuon_cloned.get()), *(ptracker_cloned.get()), *measurementTracker)) {
          if (++found == layersToTry_)
            break;
        }
      }
    }
    if (debug_)
      LogDebug("OutsideInMuonSeeder") << "Outcome of seeding for muon of pt " << mu.pt() << ", eta " << mu.eta()
                                      << ", phi " << mu.phi() << ": found " << (out->size() - sizeBefore) << " seeds."
                                      << std::endl;
  }

  iEvent.put(std::move(out));
}

int OutsideInMuonSeeder::doLayer(const GeometricSearchDet &layer,
                                 const TrajectoryStateOnSurface &state,
                                 std::vector<TrajectorySeed> &out,
                                 const Propagator &muon_propagator,
                                 const Propagator &tracker_propagator,
                                 const MeasurementTrackerEvent &measurementTracker) const {
  TrajectoryStateOnSurface onLayer(state);
  onLayer.rescaleError(errorRescaling_);
  std::vector<GeometricSearchDet::DetWithState> dets;
  layer.compatibleDetsV(onLayer, muon_propagator, *estimator_, dets);

  if (debug_) {
    LogDebug("OutsideInMuonSeeder") << "Query on layer around x = " << onLayer.globalPosition()
                                    << " with local pos error " << sqrt(onLayer.localError().positionError().xx())
                                    << " ,  " << sqrt(onLayer.localError().positionError().yy()) << " ,  "
                                    << " returned " << dets.size() << " compatible detectors" << std::endl;
  }

  std::vector<TrajectoryMeasurement> meas;
  for (std::vector<GeometricSearchDet::DetWithState>::const_iterator it = dets.begin(), ed = dets.end(); it != ed;
       ++it) {
    MeasurementDetWithData det = measurementTracker.idToDet(it->first->geographicalId());
    if (det.isNull()) {
      std::cerr << "BOGUS detid " << it->first->geographicalId().rawId() << std::endl;
      continue;
    }
    if (!it->second.isValid())
      continue;
    std::vector<TrajectoryMeasurement> mymeas =
        det.fastMeasurements(it->second, state, tracker_propagator, *estimator_);
    if (debug_)
      LogDebug("OutsideInMuonSeeder") << "Query on detector " << it->first->geographicalId().rawId() << " returned "
                                      << mymeas.size() << " measurements." << std::endl;
    for (std::vector<TrajectoryMeasurement>::const_iterator it2 = mymeas.begin(), ed2 = mymeas.end(); it2 != ed2;
         ++it2) {
      if (it2->recHit()->isValid())
        meas.push_back(*it2);
    }
  }
  int found = 0;
  std::sort(meas.begin(), meas.end(), TrajMeasLessEstim());
  for (std::vector<TrajectoryMeasurement>::const_iterator it2 = meas.begin(), ed2 = meas.end(); it2 != ed2; ++it2) {
    if (debug_) {
      LogDebug("OutsideInMuonSeeder") << "  inspecting Hit with chi2 = " << it2->estimate() << std::endl;
      LogDebug("OutsideInMuonSeeder") << "        track state     " << it2->forwardPredictedState().globalPosition()
                                      << std::endl;
      LogDebug("OutsideInMuonSeeder") << "        rechit position " << it2->recHit()->globalPosition() << std::endl;
    }
    TrajectoryStateOnSurface updated = updator_->update(it2->forwardPredictedState(), *it2->recHit());
    if (updated.isValid()) {
      if (debug_)
        LogDebug("OutsideInMuonSeeder") << "          --> updated state: x = " << updated.globalPosition()
                                        << ", p = " << updated.globalMomentum() << std::endl;
      edm::OwnVector<TrackingRecHit> seedHits;
      seedHits.push_back(*it2->recHit()->hit());
      PTrajectoryStateOnDet const &pstate =
          trajectoryStateTransform::persistentState(updated, it2->recHit()->geographicalId().rawId());
      TrajectorySeed seed(pstate, std::move(seedHits), oppositeToMomentum);
      out.push_back(seed);
      found++;
      if (found == hitsToTry_)
        break;
    }
  }
  return found;
}

void OutsideInMuonSeeder::doDebug(const reco::Track &tk) const {
  TrajectoryStateOnSurface tsos = trajectoryStateTransform::innerStateOnSurface(tk, *geometry_, magfield_);
  std::unique_ptr<Propagator> pmuon_cloned = SetPropagationDirection(*muonPropagator_, alongMomentum);
  for (unsigned int i = 0; i < tk.recHitsSize(); ++i) {
    const TrackingRecHit *hit = &*tk.recHit(i);
    const GeomDet *det = geometry_->idToDet(hit->geographicalId());
    if (det == nullptr)
      continue;
    if (i != 0)
      tsos = pmuon_cloned->propagate(tsos, det->surface());
    if (!tsos.isValid())
      continue;
    LogDebug("OutsideInMuonSeeder") << "  state " << i << " at x = " << tsos.globalPosition()
                                    << ", p = " << tsos.globalMomentum() << std::endl;
    if (hit->isValid()) {
      LogDebug("OutsideInMuonSeeder") << "         valid   rechit on detid " << hit->geographicalId().rawId()
                                      << std::endl;
    } else {
      LogDebug("OutsideInMuonSeeder") << "         invalid rechit on detid " << hit->geographicalId().rawId()
                                      << std::endl;
    }
  }
}

void OutsideInMuonSeeder::fillDescriptions(edm::ConfigurationDescriptions &descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("src", edm::InputTag("muons"));
  desc.add<std::string>("cut", {});
  desc.add<int32_t>("layersToTry", 3);
  desc.add<int32_t>("hitsToTry", 3);
  desc.add<bool>("fromVertex", true);
  desc.add<double>("errorRescaleFactor", 2.0);
  desc.add<std::string>("trackerPropagator", {});
  desc.add<std::string>("muonPropagator", {});
  desc.add<edm::InputTag>("measurementTkEvent", edm::InputTag("MeasurementTrackerEvent"));
  desc.add<std::string>("hitCollector", {});
  desc.add<std::string>("updatorLabel", "KFUpdator");
  desc.add<double>("minEtaForTEC", 0.7);
  desc.add<double>("maxEtaForTOB", 1.8);
  desc.addUntracked<bool>("debug", false);
  descriptions.addWithDefaultLabel(desc);
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(OutsideInMuonSeeder);
