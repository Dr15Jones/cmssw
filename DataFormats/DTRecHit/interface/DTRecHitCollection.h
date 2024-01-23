#ifndef DataFormats_DTRecHitCollection_H
#define DataFormats_DTRecHitCollection_H

/** \class DTRecHitCollection
 *  Collection of 1DDTRecHitPair for storage in the event. See \ref DTRecHitCollection.h for details
 *
 *  \author G. Cerminara - INFN Torino
 */

#include "DataFormats/MuonDetId/interface/DTLayerId.h"
#include "DataFormats/DTRecHit/interface/DTRecHit1DPair.h"
#include "DataFormats/Common/interface/RangeMap.h"
#include "DataFormats/Common/interface/ClonePolicy.h"
#include "DataFormats/Common/interface/OwnVector.h"
#include <functional>
#include <vector>

namespace edm::detail {
  template<>
  struct RangeMapContainer<edm::OwnVector<DTRecHit1DPair>> {
    using type = std::vector<DTRecHit1DPair>;
    };
}

typedef edm::RangeMap<DTLayerId, edm::OwnVector<DTRecHit1DPair> > DTRecHitCollection;

#endif
