#ifndef HLTrigger_Egamma_HLTEgammaCombMassFilter_h
#define HLTrigger_Egamma_HLTEgammaCombMassFilter_h

//Class: HLTEgammaCombMassFilter
//purpose: the last filter of multi-e/g triggers which have asymetric cuts on the e/g objects
//         this checks that the required number of pair candidate pass a minimum mass cut

#include "HLTrigger/HLTcore/interface/HLTFilter.h"

#include "DataFormats/HLTReco/interface/TriggerFilterObjectWithRefs.h"

#include "DataFormats/Math/interface/LorentzVector.h"

namespace edm {
  class ConfigurationDescriptions;
}

class HLTEgammaCombMassFilter : public HLTFilter {
public:
  explicit HLTEgammaCombMassFilter(const edm::ParameterSet&);
  ~HLTEgammaCombMassFilter() override;
  bool hltFilter(edm::Event&,
                 const edm::EventSetup&,
                 trigger::TriggerFilterObjectWithRefs& filterproduct) const override;
  static void getP4OfLegCands(const edm::Event& iEvent,
                              const edm::EDGetTokenT<trigger::TriggerFilterObjectWithRefs>& filterToken,
                              std::vector<math::XYZTLorentzVector>& p4s);
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  static void addObjectToFilterProduct(const edm::Event& iEvent,
                                       trigger::TriggerFilterObjectWithRefs& filterproduct,
                                       const edm::EDGetTokenT<trigger::TriggerFilterObjectWithRefs>& token,
                                       size_t index);

private:
  edm::InputTag firstLegLastFilterTag_;
  edm::InputTag secondLegLastFilterTag_;
  edm::EDGetTokenT<trigger::TriggerFilterObjectWithRefs> firstLegLastFilterToken_;
  edm::EDGetTokenT<trigger::TriggerFilterObjectWithRefs> secondLegLastFilterToken_;
  double minMass_;
  edm::InputTag l1EGTag_;
  struct LorentzVectorComparator {
    bool operator()(const math::XYZTLorentzVector& lhs, const math::XYZTLorentzVector& rhs) const {
      return lhs.pt() < rhs.pt();
    }
  };
};

#endif
