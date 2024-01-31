#include "DataFormats/VertexReco/interface/Vertex.h"
#include <Math/GenVector/PxPyPzE4D.h>
#include <Math/GenVector/PxPyPzM4D.h>
#include "DataFormats/Common/interface/RefToPtr.h"

using namespace reco;
using namespace std;

Vertex::Vertex(const Point& p, const Error& err, double chi2, double ndof, size_t size)
    : chi2_(chi2), ndof_(ndof), position_(p), time_(0.) {
  tracks_.reserve(size);
  index idx = 0;
  for (index i = 0; i < dimension4D; ++i) {
    for (index j = 0; j <= i; ++j) {
      if (i == dimension || j == dimension) {
        covariance_[idx++] = 0.0;
      } else {
        covariance_[idx++] = err(i, j);
      }
    }
  }
  validity_ = true;
}

Vertex::Vertex(const Point& p, const Error4D& err, double time, double chi2, double ndof, size_t size)
    : chi2_(chi2), ndof_(ndof), position_(p), time_(time) {
  tracks_.reserve(size4D);
  index idx = 0;
  for (index i = 0; i < dimension4D; ++i)
    for (index j = 0; j <= i; ++j)
      covariance_[idx++] = err(i, j);
  validity_ = true;
}

Vertex::Vertex(const Point& p, const Error& err) : chi2_(0.0), ndof_(0), position_(p), time_(0.) {
  index idx = 0;
  for (index i = 0; i < dimension4D; ++i) {
    for (index j = 0; j <= i; ++j) {
      if (i == dimension || j == dimension) {
        covariance_[idx++] = 0.0;
      } else {
        covariance_[idx++] = err(i, j);
      }
    }
  }
  validity_ = true;
}

Vertex::Vertex(const Point& p, const Error4D& err, double time) : chi2_(0.0), ndof_(0), position_(p), time_(time) {
  index idx = 0;
  for (index i = 0; i < dimension + 1; ++i)
    for (index j = 0; j <= i; ++j)
      covariance_[idx++] = err(i, j);
  validity_ = true;
}

void Vertex::fill(Error& err) const {
  Error4D temp;
  fill(temp);
  err = temp.Sub<Error>(0, 0);
}

void Vertex::fill(Error4D& err) const {
  index idx = 0;
  for (index i = 0; i < dimension4D; ++i)
    for (index j = 0; j <= i; ++j)
      err(i, j) = covariance_[idx++];
}

void Vertex::add(const edm::Ptr<reco::Track>& r, const Track& refTrack, float w) {
  tracks_.push_back(r);
  refittedTracks_.push_back(refTrack);
  weights_.push_back(w * 255);
}

void Vertex::removeTracks() {
  weights_.clear();
  tracks_.clear();
  refittedTracks_.clear();
}

edm::Ptr<reco::Track> Vertex::originalTrack(const Track& refTrack) const {
  if (refittedTracks_.empty())
    throw cms::Exception("Vertex") << "No refitted tracks stored in vertex\n";
  std::vector<Track>::const_iterator it = find_if(refittedTracks_.begin(), refittedTracks_.end(), TrackEqual(refTrack));
  if (it == refittedTracks_.end())
    throw cms::Exception("Vertex") << "Refitted track not found in list.\n pt used for comparison: " << refTrack.pt();
  size_t pos = it - refittedTracks_.begin();
  return tracks_[pos];
}

Track Vertex::refittedTrack(const edm::Ptr<reco::Track>& track) const {
  if (refittedTracks_.empty())
    throw cms::Exception("Vertex") << "No refitted tracks stored in vertex\n";
  trackRef_iterator it = find(tracks_begin(), tracks_end(), track);
  if (it == tracks_end())
    throw cms::Exception("Vertex") << "Track not found in list\n";
  size_t pos = it - tracks_begin();
  return refittedTracks_[pos];
}

Track Vertex::refittedTrack(const TrackRef& track) const { return refittedTrack(edm::refToPtr(track)); }

math::XYZTLorentzVectorD Vertex::p4(float mass, float minWeight) const {
  math::XYZTLorentzVectorD sum;
  ROOT::Math::LorentzVector<ROOT::Math::PxPyPzM4D<double> > vec;

  if (hasRefittedTracks()) {
    for (std::vector<Track>::const_iterator iter = refittedTracks_.begin(); iter != refittedTracks_.end(); ++iter) {
      if (trackWeight(originalTrack(*iter)) >= minWeight) {
        vec.SetPx(iter->px());
        vec.SetPy(iter->py());
        vec.SetPz(iter->pz());
        vec.SetM(mass);
        sum += vec;
      }
    }
  } else {
    for (auto const& track: tracks_) {
      if (trackWeight(track) >= minWeight) {
        vec.SetPx((track)->px());
        vec.SetPy((track)->py());
        vec.SetPz((track)->pz());
        vec.SetM(mass);
        sum += vec;
      }
    }
  }
  return sum;
}

unsigned int Vertex::nTracks(float minWeight) const {
  int n = 0;
  if (hasRefittedTracks()) {
    for (std::vector<Track>::const_iterator iter = refittedTracks_.begin(); iter != refittedTracks_.end(); ++iter)
      if (trackWeight(originalTrack(*iter)) >= minWeight)
        n++;
  } else {
    for (auto const& track: tracks_) {
      if (trackWeight(track) >= minWeight)
        n++;
    }
  }
  return n;
}
