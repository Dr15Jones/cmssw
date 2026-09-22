#ifndef FWCore_Framework_maker_TransitionWorker_h
#define FWCore_Framework_maker_TransitionWorker_h
/*----------------------------------------------------------------------    
*/

#include "FWCore/Framework/interface/maker/Worker.h"
namespace edm {
  class EventTransitionInfo;
  class RunTransitionInfo;
  class LumiTransitionInfo;
  class TransitionPhaseGlobal;
  class TransitionPhaseStream;
  template <typename TI, typename TP>
  class TransitionWorker : public Worker {
  public:
    TransitionWorker(ModuleDescription const& iMD, ExceptionToActionTable const* iActions) : Worker(iMD, iActions) {}
    ~TransitionWorker() override = default;
  };

  using StreamRunWorker = TransitionWorker<RunTransitionInfo, TransitionPhaseStream>;
  using StreamLumiWorker = TransitionWorker<LumiTransitionInfo, TransitionPhaseStream>;
  using GlobalRunWorker = TransitionWorker<RunTransitionInfo, TransitionPhaseGlobal>;
  using GlobalLumiWorker = TransitionWorker<LumiTransitionInfo, TransitionPhaseGlobal>;
  using GlobalEventWorker = TransitionWorker<EventTransitionInfo, TransitionPhaseGlobal>;
}  // namespace edm
#endif
