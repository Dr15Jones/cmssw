#ifndef SimG4Core_BeginOfJob_H
#define SimG4Core_BeginOfJob_H

#include "SimG4Core/Notification/interface/ConditionsAccess.h"

namespace edm {
  class EventSetup;
}

class BeginOfJob {
public:
  BeginOfJob(const edm::EventSetup* tJob, const sim::ConditionsAccess* iCA) : es_(tJob), ca_(iCA) {}
  const edm::EventSetup* operator()() const { return es_; }

  template <typename DATA, typename REC>
  DATA const& get(std::string_view iLabel = std::string_view()) const {
    return ca_->get<DATA, REC>(iLabel);
  }

private:
  const edm::EventSetup* es_;
  const sim::ConditionsAccess* ca_;
};

#endif
