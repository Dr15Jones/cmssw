//
//

#include "DataFormats/PatCandidates/interface/TriggerObject.h"

#include "DataFormats/Common/interface/RefToBaseToPtr.h"
#include "FWCore/Utilities/interface/EDMException.h"

using namespace pat;

// Constructors and Destructor

// Default constructor
TriggerObject::TriggerObject() : reco::LeafCandidate() { triggerObjectTypes_.clear(); }

// Constructor from trigger::TriggerObject
TriggerObject::TriggerObject(const trigger::TriggerObject& trigObj)
    : reco::LeafCandidate(0, trigObj.particle().p4(), reco::Particle::Point(0., 0., 0.), trigObj.id()), refToOrig_() {
  triggerObjectTypes_.clear();
}

// Constructors from base class object
TriggerObject::TriggerObject(const reco::LeafCandidate& leafCand) : reco::LeafCandidate(leafCand), refToOrig_() {
  triggerObjectTypes_.clear();
}

// Constructors from base candidate reference (for 'l1extra' particles)
TriggerObject::TriggerObject(const reco::CandidateBaseRef& candRef)
    : reco::LeafCandidate(*candRef), refToOrig_(edm::refToBaseToPtr(candRef)) {
  triggerObjectTypes_.clear();
}

// Constructors from Lorentz-vectors and (optional) PDG ID
TriggerObject::TriggerObject(const reco::Particle::LorentzVector& vec, int id)
    : reco::LeafCandidate(0, vec, reco::Particle::Point(0., 0., 0.), id), refToOrig_() {
  triggerObjectTypes_.clear();
}
TriggerObject::TriggerObject(const reco::Particle::PolarLorentzVector& vec, int id)
    : reco::LeafCandidate(0, vec, reco::Particle::Point(0., 0., 0.), id), refToOrig_() {
  triggerObjectTypes_.clear();
}

// Methods

// Get all trigger object type identifiers
std::vector<int> TriggerObject::triggerObjectTypes() const {
  std::vector<int> triggerObjectTypes;
  for (size_t iTo = 0; iTo < triggerObjectTypes_.size(); ++iTo) {
    triggerObjectTypes.push_back(triggerObjectTypes_.at(iTo));
  }
  return triggerObjectTypes;
}

// Checks, if a certain label of original collection is assigned
bool TriggerObject::hasCollection(const std::string& collName) const {
  // True, if collection name is simply fine
  if (collName == collection_)
    return true;
  // Check, if collection name possibly fits in an edm::InputTag approach
  const edm::InputTag collectionTag(collection_);
  const edm::InputTag collTag(collName);
  // If evaluated collection tag contains a process name, it must have been found already by identity check
  if (collTag.process().empty()) {
    // Check instance ...
    if ((collTag.instance().empty() && collectionTag.instance().empty()) ||
        collTag.instance() == collectionTag.instance()) {
      // ... and label
      return (collTag.label() == collectionTag.label());
    }
  }
  return false;
}

// Checks, if a certain trigger object type identifier is assigned
bool TriggerObject::hasTriggerObjectType(trigger::TriggerObjectType triggerObjectType) const {
  for (size_t iF = 0; iF < triggerObjectTypes_.size(); ++iF) {
    if (triggerObjectType == triggerObjectTypes_.at(iF))
      return true;
  }
  return false;
}

