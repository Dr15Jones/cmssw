#ifndef MuonReco_MuonChamberMatch_h
#define MuonReco_MuonChamberMatch_h

#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/MuonReco/interface/MuonSegmentMatch.h"
#include "DataFormats/MuonReco/interface/MuonRPCHitMatch.h"
#include "DataFormats/MuonReco/interface/MuonGEMHitMatch.h"
#include <vector>

//#if defined(__CLING__)
//#define CMS_HIDE_INLINE_NAMESPACE
//#endif
//#if defined(CMS_HIDE_INLINE_NAMESPACE)
//#define CMS_INLINE_NAMESPACE namespace
//#else
//#define CMS_INLINE_NAMESPACE inline namespace
//#endif

namespace reco {
  inline namespace v2 {
  class MuonChamberMatch {
  public:
    std::vector<reco::v2::MuonSegmentMatch> segmentMatches;  // segments matching propagated track trajectory
    std::vector<reco::v2::MuonSegmentMatch> gemMatches;      // segments matching propagated track trajectory
    std::vector<reco::MuonGEMHitMatch> gemHitMatches;    // segments matching propagated track trajectory
    std::vector<reco::v2::MuonSegmentMatch> me0Matches;      // segments matching propagated track trajectory
    std::vector<reco::v2::MuonSegmentMatch> truthMatches;    // SimHit projection matching propagated track trajectory
    std::vector<reco::MuonRPCHitMatch> rpcMatches;       // rpc hits matching propagated track trajectory
    float edgeX;    // distance to closest edge in X (negative - inside, positive - outside)
    float edgeY;    // distance to closest edge in Y (negative - inside, positive - outside)
    float x;        // X position of the track
    float y;        // Y position of the track
    float xErr;     // propagation uncertainty in X
    float yErr;     // propagation uncertainty in Y
    float dXdZ;     // dX/dZ of the track
    float dYdZ;     // dY/dZ of the track
    float dXdZErr;  // propagation uncertainty in dX/dZ
    float dYdZErr;  // propagation uncertainty in dY/dZ
    DetId id;       // chamber ID

    int nDigisInRange;  // # of DT/CSC digis in the chamber close-by to the propagated track

    int detector() const { return id.subdetId(); }
    int station() const;

    std::pair<float, float> getDistancePair(float edgeX, float edgeY, float xErr, float yErr) const;
    float dist() const { return getDistancePair(edgeX, edgeY, xErr, yErr).first; }  // distance to absolute closest edge
    float distErr() const {
      return getDistancePair(edgeX, edgeY, xErr, yErr).second;
    }  // propagation uncertainty in above distance
  };
  }
}  // namespace reco

#endif
