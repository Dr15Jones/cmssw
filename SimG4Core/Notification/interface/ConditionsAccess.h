#ifndef SimG4Core_Notification_ConditionsAccess_h
#define SimG4Core_Notification_ConditionsAccess_h
// -*- C++ -*-
//
// Package:     SimG4Core/Notification
// Class  :     ConditionsAccess
//
/**\class ConditionsAccess ConditionsAccess.h "SimG4Core/Notification/interface/ConditionsAccess.h"

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Christopher Jones
//         Created:  Fri, 05 Mar 2021 14:00:07 GMT
//

// system include files
#include <tuple>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <typeindex>

// user include files
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Utilities/interface/ESGetToken.h"
#include "FWCore/Utilities/interface/Exception.h"

// forward declarations

namespace sim {
  class ConditionsAccess {
  public:
    ConditionsAccess() = default;
    ~ConditionsAccess() = default;

    struct RetrieverBase {
      virtual ~RetrieverBase() = default;
      virtual void const* retrieve(edm::EventSetup const&) const = 0;
    };

    template <typename REC, typename DATA>
    struct Retriever : public RetrieverBase {
      Retriever(edm::ESGetToken<DATA, REC> iToken) : token_(iToken) {}

      void const* retrieve(edm::EventSetup const& iSetup) const final { return &(iSetup.getData(token_)); }

    private:
      edm::ESGetToken<DATA, REC> token_;
    };

    ConditionsAccess(const ConditionsAccess&) = delete;                   // stop default
    const ConditionsAccess& operator=(const ConditionsAccess&) = delete;  // stop default

    // ---------- const member functions ---------------------
    template <typename REC, typename DATA>
    DATA const& get(std::string_view iLabel = std::string_view()) const {
      auto itFound = retrievers_.find(makeKey<REC, DATA>(iLabel));
      if (itFound == retrievers_.end()) {
        throw cms::Exception("UnregisteredDataRequest")
            << "A sim plugin requested data that was not prefetched based on consumes request.";
      }
      return *static_cast<DATA const*>(itFound->second->retrieve(*eventSetup_));
    }

    template <typename REC, typename DATA>
    bool hasRetriever(std::string_view iLabel) const {
      auto key = makeKey<REC, DATA>(iLabel);
      return retrievers_.end() != retrievers_.find(key);
    }

    // ---------- static member functions --------------------

    // ---------- member functions ---------------------------
    void set(edm::EventSetup const& iSetup) { eventSetup_ = &iSetup; }

    template <typename REC, typename DATA>
    void insertRetriever(std::string_view iLabel, edm::ESGetToken<DATA, REC> iToken) {
      if (not hasRetriever<REC, DATA>(iLabel)) {
        auto it = labels_.emplace(iLabel);
        std::string_view storedLabel = *it.first;
        retrievers_.emplace(makeKey<REC, DATA>(storedLabel), std::make_unique<Retriever<REC, DATA>>(iToken));
      }
    }

  private:
    // ---------- member data --------------------------------
    using Key = std::tuple<std::type_index, std::type_index, std::string_view>;
    template <typename REC, typename DATA>
    Key makeKey(std::string_view iLabel) const {
      return Key(std::type_index(typeid(REC)), std::type_index(typeid(DATA)), iLabel);
    }

    std::set<std::string> labels_;
    std::map<Key, std::unique_ptr<RetrieverBase>> retrievers_;
    edm::EventSetup const* eventSetup_ = nullptr;
  };
}  // namespace sim
#endif
