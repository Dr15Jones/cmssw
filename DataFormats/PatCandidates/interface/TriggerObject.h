#ifndef DataFormats_PatCandidates_TriggerObject_h
#define DataFormats_PatCandidates_TriggerObject_h

// -*- C++ -*-
//
// Package:    PatCandidates
// Class:      pat::TriggerObject
//
//
/**
  \class    pat::TriggerObject TriggerObject.h "DataFormats/PatCandidates/interface/TriggerObject.h"
  \brief    Analysis-level trigger object class

   TriggerObject implements a container for trigger objects' information within the 'pat' namespace.
   For detailed information, consult
   https://twiki.cern.ch/twiki/bin/view/CMS/SWGuidePATTrigger#TriggerObject

  \author   Volker Adler
*/

#include "DataFormats/Candidate/interface/LeafCandidate.h"

#include <map>
#include <string>
#include <vector>
#include <algorithm>

#include "DataFormats/L1Trigger/interface/L1EmParticle.h"
#include "DataFormats/L1Trigger/interface/L1EmParticleFwd.h"
#include "DataFormats/L1Trigger/interface/L1EtMissParticle.h"
#include "DataFormats/L1Trigger/interface/L1EtMissParticleFwd.h"
#include "DataFormats/L1Trigger/interface/L1JetParticle.h"
#include "DataFormats/L1Trigger/interface/L1JetParticleFwd.h"
#include "DataFormats/L1Trigger/interface/L1MuonParticle.h"
#include "DataFormats/L1Trigger/interface/L1MuonParticleFwd.h"
#include "DataFormats/HLTReco/interface/TriggerTypeDefs.h"
#include "DataFormats/HLTReco/interface/TriggerObject.h"
#include "DataFormats/Common/interface/RefVectorIterator.h"
#include "DataFormats/Common/interface/Association.h"
#include "FWCore/Utilities/interface/InputTag.h"

namespace pat {

  class TriggerObject : public reco::LeafCandidate {
    /// Data Members

    /// Label of the collection the trigger object originates from
    std::string collection_;
    /// Vector of special identifiers for the trigger object type as defined in
    /// trigger::TriggerObjectType (DataFormats/HLTReco/interface/TriggerTypeDefs.h),
    /// possibly empty
    std::vector<trigger::TriggerObjectType> triggerObjectTypes_;
    /// Reference to trigger object,
    /// meant for 'l1extra' particles to access their additional functionalities,
    /// empty otherwise
    reco::CandidatePtr refToOrig_;

  public:
    /// Constructors and Destructor

    /// Default constructor
    TriggerObject();
    /// Constructor from trigger::TriggerObject
    TriggerObject(const trigger::TriggerObject& trigObj);
    /// Constructors from base class object
    TriggerObject(const reco::LeafCandidate& leafCand);
    /// Constructors from base candidate reference (for 'l1extra' particles)
    TriggerObject(const reco::CandidateBaseRef& candRef);
    /// Constructors from Lorentz-vectors and (optional) PDG ID
    TriggerObject(const reco::Particle::LorentzVector& vec, int id = 0);
    TriggerObject(const reco::Particle::PolarLorentzVector& vec, int id = 0);

    /// Destructor
    ~TriggerObject() override{};

    /// Methods

    /// Set the label of the collection the trigger object originates from
    void setCollection(const std::string& collName) { collection_ = collName; };
    void setCollection(const edm::InputTag& collName) { collection_ = collName.encode(); };
    /// Add a new trigger object type identifier
    void addTriggerObjectType(trigger::TriggerObjectType triggerObjectType) {
      if (!hasTriggerObjectType(triggerObjectType))
        triggerObjectTypes_.push_back(triggerObjectType);
    };
    void addTriggerObjectType(int triggerObjectType) {
      addTriggerObjectType(trigger::TriggerObjectType(triggerObjectType));
    };
    void addFilterId(trigger::TriggerObjectType triggerObjectType) {
      addTriggerObjectType(triggerObjectType);
    };  // for backward compatibility
    void addFilterId(int triggerObjectType) {
      addTriggerObjectType(trigger::TriggerObjectType(triggerObjectType));
    };  // for backward compatibility
    /// Get the label of the collection the trigger object originates from
    const std::string& collection() const { return collection_; };
    /// Get all trigger object type identifiers
    //       std::vector< trigger::TriggerObjectType > triggerObjectTypes() const { return triggerObjectTypes_; };
    //       std::vector< trigger::TriggerObjectType > filterIds()          const { return triggerObjectTypes(); }; // for backward compatibility
    std::vector<int> triggerObjectTypes() const;                          // for backward compatibility
    std::vector<int> filterIds() const { return triggerObjectTypes(); };  // for double backward compatibility
    /// Checks, if a certain label of original collection is assigned
    virtual bool hasCollection(const std::string& collName) const;
    virtual bool hasCollection(const edm::InputTag& collName) const { return hasCollection(collName.encode()); };
    /// Checks, if a certain trigger object type identifier is assigned
    bool hasTriggerObjectType(trigger::TriggerObjectType triggerObjectType) const;
    bool hasTriggerObjectType(int triggerObjectType) const {
      return hasTriggerObjectType(trigger::TriggerObjectType(triggerObjectType));
    };
    bool hasFilterId(trigger::TriggerObjectType triggerObjectType) const {
      return hasTriggerObjectType(triggerObjectType);
    };  // for backward compatibility
    bool hasFilterId(int triggerObjectType) const {
      return hasTriggerObjectType(trigger::TriggerObjectType(triggerObjectType));
    };  // for backward compatibility

    /// Special methods for 'l1extra' particles

    /// General getters
    const reco::Candidate* origObjCand() const { return refToOrig_.get(); };
    /// Getters specific to the 'l1extra' particle type for
    /// - EM
    const L1GctEmCand* origL1GctEmCand() const {
      auto* cand = dynamic_cast<const l1extra::L1EmParticle*>(origObjCand());
      return cand ? cand->gctEmCand() : nullptr;
    };
    /// - EtMiss
    const L1GctEtMiss* origL1GctEtMiss() const {
      auto* cand = dynamic_cast<const l1extra::L1EtMissParticle*>(origObjCand());
      return cand ? cand->gctEtMiss() : nullptr;
    };
    const L1GctEtTotal* origL1GctEtTotal() const {
      auto* cand = dynamic_cast<const l1extra::L1EtMissParticle*>(origObjCand());
      return cand ? cand->gctEtTotal() : nullptr;
    };
    const L1GctHtMiss* origL1GctHtMiss() const {
      auto* cand = dynamic_cast<const l1extra::L1EtMissParticle*>(origObjCand());
      return cand ? cand->gctHtMiss() : nullptr;
    };
    const L1GctEtHad* origL1GctEtHad() const {
      auto* cand = dynamic_cast<const l1extra::L1EtMissParticle*>(origObjCand());
      return cand ? cand->gctEtHad() : nullptr;
    };
    /// - Jet
    const L1GctJetCand* origL1GctJetCand() const {
      auto* cand = dynamic_cast<const l1extra::L1JetParticle*>(origObjCand());
      return cand ? cand->gctJetCand() : nullptr;
    };
    /// - Muon
    const L1MuGMTExtendedCand* origL1GmtMuonCand() const {
      auto* cand = dynamic_cast<const l1extra::L1MuonParticle*>(origObjCand());
      return cand ? &(cand->gmtMuonCand()) : nullptr;
    };

    /// Special methods for the cut string parser
    /// - argument types usable in the cut string parser
    /// - short names for readable configuration files

    /// Calls 'hasCollection(...)'
    virtual bool coll(const std::string& collName) const { return hasCollection(collName); };
    /// Calls 'hasTriggerObjectType(...)'
    bool type(trigger::TriggerObjectType triggerObjectType) const { return hasTriggerObjectType(triggerObjectType); };
    bool type(int triggerObjectType) const {
      return hasTriggerObjectType(trigger::TriggerObjectType(triggerObjectType));
    };
    bool id(trigger::TriggerObjectType triggerObjectType) const {
      return hasTriggerObjectType(triggerObjectType);
    };  // for backward compatibility
    bool id(int triggerObjectType) const {
      return hasTriggerObjectType(trigger::TriggerObjectType(triggerObjectType));
    };  // for backward compatibility
  };

  /// Collection of TriggerObject
  typedef std::vector<TriggerObject> TriggerObjectCollection;
  /// Persistent reference to an item in a TriggerObjectCollection
  typedef edm::Ref<TriggerObjectCollection> TriggerObjectRef;
  /// Container to store match references from different producers (for one PAT object)
  typedef std::map<std::string, TriggerObjectRef> TriggerObjectMatchMap;
  /// Persistent reference to a TriggerObjectCollection product
  typedef edm::RefProd<TriggerObjectCollection> TriggerObjectRefProd;
  /// Vector of persistent references to items in the same TriggerObjectCollection
  typedef edm::RefVector<TriggerObjectCollection> TriggerObjectRefVector;
  /// Const iterator over vector of persistent references to items in the same TriggerObjectCollection
  typedef edm::RefVectorIterator<TriggerObjectCollection> TriggerObjectRefVectorIterator;
  /// Association of TriggerObjects to store matches to Candidates
  typedef edm::Association<TriggerObjectCollection> TriggerObjectMatch;
  /// Persistent reference to a TriggerObjectMatch product
  typedef edm::RefProd<TriggerObjectMatch> TriggerObjectMatchRefProd;
  /// Container to store references to matches from different producers in the trigger event
  typedef std::map<std::string, TriggerObjectMatchRefProd> TriggerObjectMatchContainer;

}  // namespace pat

#endif
