#ifndef _PFDisplacedTrackerVertex_H
#define _PFDisplacedTrackerVertex_H

// class which contains the secondary PFRecTracks
// this dataformat will be used to create PFBlockElementNuclTrack

// \author Maxime Gouzevitch

#include "DataFormats/ParticleFlowReco/interface/PFDisplacedVertex.h"
#include "DataFormats/ParticleFlowReco/interface/PFDisplacedVertexFwd.h"
#include "DataFormats/ParticleFlowReco/interface/PFRecTrack.h"
#include "DataFormats/ParticleFlowReco/interface/PFRecTrackFwd.h"
#include "DataFormats/Common/interface/RefToPtr.h"

namespace reco {
  class PFDisplacedTrackerVertex {
  public:
    PFDisplacedTrackerVertex() {}
    PFDisplacedTrackerVertex(const PFDisplacedVertexRef& nuclref, const PFRecTrackRefVector& pfRecTracks)
        : displacedVertexRef_(nuclref), pfRecTracks_(pfRecTracks) {}

    const PFRecTrackRefVector& pfRecTracks() const { return pfRecTracks_; }

    const bool isIncomingTrack(const reco::PFRecTrackRef originalTrack) const {
      return displacedVertexRef_->isIncomingTrack(edm::refToPtr(originalTrack->trackRef()));
    }

    const bool isOutgoingTrack(const reco::PFRecTrackRef originalTrack) const {
      return displacedVertexRef_->isOutgoingTrack(edm::refToPtr(originalTrack->trackRef()));
    }

    const PFDisplacedVertexRef& displacedVertexRef() const { return displacedVertexRef_; }

  private:
    // Reference to the initial DisplacedTrackerVertex
    PFDisplacedVertexRef displacedVertexRef_;

    // Collection of the secondary PFRecTracks
    PFRecTrackRefVector pfRecTracks_;
  };

  /// collection of DisplacedTrackerVertexs
  typedef std::vector<PFDisplacedTrackerVertex> PFDisplacedTrackerVertexCollection;
  /// persistent reference to a DisplacedTrackerVertex
  typedef edm::Ref<PFDisplacedTrackerVertexCollection> PFDisplacedTrackerVertexRef;
  /// vector of reference to Track in the same collection
  typedef edm::RefVector<PFDisplacedTrackerVertexCollection> PFDisplacedTrackerVertexRefVector;
}  // namespace reco
#endif
