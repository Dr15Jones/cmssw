#ifndef SimG4Core_SensitiveDetector_AttachSD_h
#define SimG4Core_SensitiveDetector_AttachSD_h

#include <vector>

namespace edm {
  class EventSetup;
  class ParameterSet;
}  // namespace edm

namespace sim {
  class CAConsumesCollector;
}

class SensitiveDetectorCatalog;
class SensitiveTkDetector;
class SensitiveCaloDetector;
class SimActivityRegistry;
class SimTrackManager;

namespace attachSD {

  std::pair<std::vector<SensitiveTkDetector *>, std::vector<SensitiveCaloDetector *> > create(
      const edm::EventSetup &,
      const SensitiveDetectorCatalog &,
      edm::ParameterSet const &,
      const SimTrackManager *,
      SimActivityRegistry &reg);

  void consumes(const SensitiveDetectorCatalog &clg, edm::ParameterSet const &, sim::CAConsumesCollector &);
};  // namespace attachSD

#endif
