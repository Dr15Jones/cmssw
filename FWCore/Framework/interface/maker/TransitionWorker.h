#ifndef FWCore_Framework_maker_TransitionWorker_h
#define FWCore_Framework_maker_TransitionWorker_h
/*----------------------------------------------------------------------    
*/

#include "FWCore/Framework/interface/maker/Worker.h"
namespace edm {
  template <typename TI, typename TP>
  class TransitionWorker : public Worker {
  public:
    TransitionWorker(ModuleDescription const& iMD, ExceptionToActionTable const* iActions) : Worker(iMD, iActions) {}
    ~TransitionWorker() override = default;
  };

}  // namespace edm
#endif
