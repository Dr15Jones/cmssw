#ifndef SimG4Core_Notification_CAConsumesCollector_h
#define SimG4Core_Notification_CAConsumesCollector_h
// -*- C++ -*-
//
// Package:     SimG4Core/Notification
// Class  :     CAConsumesCollector
//
/**\class CAConsumesCollector CAConsumesCollector.h "SimG4Core/Notification/interface/CAConsumesCollector.h"

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Christopher Jones
//         Created:  Fri, 05 Mar 2021 14:01:05 GMT
//

// system include files
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "SimG4Core/Notification/interface/ConditionsAccess.h"

// user include files

// forward declarations

namespace sim {
  class CAConsumesCollector {
  public:
    explicit CAConsumesCollector(edm::ConsumesCollector iCC, ConditionsAccess& iCA) : collector_(iCC), access_(&iCA) {}
    ~CAConsumesCollector() = default;

    CAConsumesCollector(const CAConsumesCollector&) = delete;                   // stop default
    const CAConsumesCollector& operator=(const CAConsumesCollector&) = delete;  // stop default

    // ---------- member functions ---------------------
    template <typename DATA, typename REC>
    void consume(std::string_view iLabel = std::string_view()) {
      access_->insertRetriever<REC, DATA>(iLabel, collector_->esConsumes<DATA, REC>(edm::ESInputTag("", iLabel)));
    }

  private:
    edm::ConsumesCollector collector_;
    ConditionsAccess* access_;
  };
}  // namespace sim
#endif
