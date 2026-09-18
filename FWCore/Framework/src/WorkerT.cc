#include "FWCore/Framework/interface/maker/WorkerT.h"
#include "FWCore/Framework/interface/EventPrincipal.h"

#include "FWCore/Framework/interface/one/EDProducerBase.h"
#include "FWCore/Framework/interface/one/EDFilterBase.h"
#include "FWCore/Framework/interface/one/EDAnalyzerBase.h"
#include "FWCore/Framework/interface/one/OutputModuleBase.h"
#include "FWCore/Framework/interface/global/EDProducerBase.h"
#include "FWCore/Framework/interface/global/EDFilterBase.h"
#include "FWCore/Framework/interface/global/EDAnalyzerBase.h"
#include "FWCore/Framework/interface/global/OutputModuleBase.h"

#include "FWCore/Framework/interface/stream/EDProducerAdaptorBase.h"
#include "FWCore/Framework/interface/stream/EDFilterAdaptorBase.h"
#include "FWCore/Framework/interface/stream/EDAnalyzerAdaptorBase.h"

#include "FWCore/Framework/interface/limited/EDProducerBase.h"
#include "FWCore/Framework/interface/limited/EDFilterBase.h"
#include "FWCore/Framework/interface/limited/EDAnalyzerBase.h"
#include "FWCore/Framework/interface/limited/OutputModuleBase.h"

#include "FWCore/ServiceRegistry/interface/ModuleConsumesInfo.h"

#include <type_traits>

namespace edm {
  namespace workerimpl {
    template <typename T>
    struct has_stream_functions {
      static bool constexpr value = false;
    };

    template <>
    struct has_stream_functions<edm::global::EDProducerBase> {
      static bool constexpr value = true;
    };

    template <>
    struct has_stream_functions<edm::global::EDFilterBase> {
      static bool constexpr value = true;
    };

    template <>
    struct has_stream_functions<edm::global::EDAnalyzerBase> {
      static bool constexpr value = true;
    };

    template <>
    struct has_stream_functions<edm::limited::EDProducerBase> {
      static bool constexpr value = true;
    };

    template <>
    struct has_stream_functions<edm::limited::EDFilterBase> {
      static bool constexpr value = true;
    };

    template <>
    struct has_stream_functions<edm::limited::EDAnalyzerBase> {
      static bool constexpr value = true;
    };

    template <>
    struct has_stream_functions<edm::stream::EDProducerAdaptorBase> {
      static bool constexpr value = true;
    };

    template <>
    struct has_stream_functions<edm::stream::EDFilterAdaptorBase> {
      static bool constexpr value = true;
    };

    template <>
    struct has_stream_functions<edm::stream::EDAnalyzerAdaptorBase> {
      static bool constexpr value = true;
    };

    template <typename T>
    struct has_only_stream_transition_functions {
      static bool constexpr value = false;
    };

    template <>
    struct has_only_stream_transition_functions<edm::global::OutputModuleBase> {
      static bool constexpr value = true;
    };

    struct DoNothing {
      template <typename... T>
      inline void operator()(const T&...) {}
    };

    template <typename T, typename TI, typename TP>
    struct DoBeginStream {
      inline void operator()(WorkerT<T, TI, TP>* iWorker, StreamID id) { iWorker->callWorkerBeginStream(0, id); }
    };

    template <typename T, typename TI, typename TP>
    struct DoEndStream {
      inline void operator()(WorkerT<T, TI, TP>* iWorker, StreamID id) { iWorker->callWorkerEndStream(0, id); }
    };

    template <typename T, typename TI, typename TP, typename INFOTYPE>
    struct DoStreamBeginTrans {
      inline void operator()(WorkerT<T, TI, TP>* iWorker,
                             StreamID id,
                             INFOTYPE const& info,
                             ModuleCallingContext const* mcc) {
        iWorker->callWorkerStreamBegin(0, id, info, mcc);
      }
    };

    template <typename T, typename TI, typename TP, typename INFOTYPE>
    struct DoStreamEndTrans {
      inline void operator()(WorkerT<T, TI, TP>* iWorker,
                             StreamID id,
                             INFOTYPE const& info,
                             ModuleCallingContext const* mcc) {
        iWorker->callWorkerStreamEnd(0, id, info, mcc);
      }
    };
  }  // namespace workerimpl

  template <typename T, typename TI, typename TP>
  inline WorkerT<T, TI, TP>::WorkerT(std::shared_ptr<T> ed,
                                     ModuleDescription const& md,
                                     ExceptionToActionTable const* actions)
      : TransitionWorker<TI, TP>(md, actions), module_(ed) {
    assert(module_ != nullptr);
  }

  template <typename T, typename TI, typename TP>
  WorkerT<T, TI, TP>::~WorkerT() {}

  template <typename T, typename TI, typename TP>
  bool WorkerT<T, TI, TP>::wantsProcessBlocks() const noexcept {
    return module_->wantsProcessBlocks();
  }

  template <typename T, typename TI, typename TP>
  bool WorkerT<T, TI, TP>::wantsInputProcessBlocks() const noexcept {
    return module_->wantsInputProcessBlocks();
  }

  template <typename T, typename TI, typename TP>
  bool WorkerT<T, TI, TP>::wantsGlobalRuns() const noexcept {
    return module_->wantsGlobalRuns();
  }

  template <typename T, typename TI, typename TP>
  bool WorkerT<T, TI, TP>::wantsGlobalLuminosityBlocks() const noexcept {
    return module_->wantsGlobalLuminosityBlocks();
  }

  template <typename T, typename TI, typename TP>
  bool WorkerT<T, TI, TP>::wantsStreamRuns() const noexcept {
    return module_->wantsStreamRuns();
  }

  template <typename T, typename TI, typename TP>
  bool WorkerT<T, TI, TP>::wantsStreamLuminosityBlocks() const noexcept {
    return module_->wantsStreamLuminosityBlocks();
  }

  template <typename T, typename TI, typename TP>
  bool WorkerT<T, TI, TP>::wantsWrites() const noexcept {
    return false;
  }
#define EDM_FOR_EACH_WORKERT_TRANSITION(M, T)                  \
  M(T, RunTransitionInfo, TransitionPhaseGlobal)               \
  M(T, RunTransitionInfo, TransitionPhaseStream)               \
  M(T, LumiTransitionInfo, TransitionPhaseGlobal)              \
  M(T, LumiTransitionInfo, TransitionPhaseStream)              \
  M(T, ProcessBlockTransitionInfo, TransitionPhaseGlobal)      \
  M(T, InputProcessBlockTransitionInfo, TransitionPhaseGlobal) \
  M(T, EventTransitionInfo, TransitionPhaseGlobal)             \
  M(T, EventTransitionInfo, TransitionPhaseStream)

#define EDM_SPECIALIZE_WORKERT_WANTS_WRITES(T, TI, TP)    \
  template <>                                             \
  bool WorkerT<T, TI, TP>::wantsWrites() const noexcept { \
    return true;                                          \
  }

  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_WANTS_WRITES, edm::global::OutputModuleBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_WANTS_WRITES, edm::one::OutputModuleBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_WANTS_WRITES, edm::limited::OutputModuleBase)

#undef EDM_SPECIALIZE_WORKERT_WANTS_WRITES

  template <typename T, typename TI, typename TP>
  SerialTaskQueue* WorkerT<T, TI, TP>::globalRunsQueue() {
    return nullptr;
  }
  template <typename T, typename TI, typename TP>
  SerialTaskQueue* WorkerT<T, TI, TP>::globalLuminosityBlocksQueue() {
    return nullptr;
  }

//one
#define EDM_SPECIALIZE_WORKERT_GLOBALQUEUES(T, TI, TP)                 \
  template <>                                                          \
  SerialTaskQueue* WorkerT<T, TI, TP>::globalRunsQueue() {             \
    return module_->globalRunsQueue();                                 \
  }                                                                    \
  template <>                                                          \
  SerialTaskQueue* WorkerT<T, TI, TP>::globalLuminosityBlocksQueue() { \
    return module_->globalLuminosityBlocksQueue();                     \
  }

  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_GLOBALQUEUES, one::EDProducerBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_GLOBALQUEUES, one::EDFilterBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_GLOBALQUEUES, one::EDAnalyzerBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_GLOBALQUEUES, one::OutputModuleBase)
#undef EDM_SPECIALIZE_WORKERT_GLOBALQUEUES

  template <typename T, typename TI, typename TP>
  inline bool WorkerT<T, TI, TP>::implDo(EventTransitionInfo const& info, ModuleCallingContext const* mcc) {
    return module_->doEvent(info, mcc);
  }

  template <typename T, typename TI, typename TP>
  inline void WorkerT<T, TI, TP>::implDoAcquire(EventTransitionInfo const&,
                                                ModuleCallingContext const*,
                                                WaitingTaskHolder&&) {}

#define EDM_SPECIALIZE_WORKERT_ACQUIRE(T, TI, TP)                                                     \
  template <>                                                                                         \
  inline void WorkerT<T, TI, TP>::implDoAcquire(                                                      \
      EventTransitionInfo const& info, ModuleCallingContext const* mcc, WaitingTaskHolder&& holder) { \
    module_->doAcquire(info, mcc, std::move(holder));                                                 \
  }

  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_ACQUIRE, global::EDProducerBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_ACQUIRE, global::EDFilterBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_ACQUIRE, global::OutputModuleBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_ACQUIRE, stream::EDProducerAdaptorBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_ACQUIRE, stream::EDFilterAdaptorBase)
#undef EDM_SPECIALIZE_WORKERT_ACQUIRE

  template <typename T, typename TI, typename TP>
  inline void WorkerT<T, TI, TP>::implDoTransformAsync(WaitingTaskHolder iTask,
                                                       size_t iTransformIndex,
                                                       EventPrincipal const& iEvent,
                                                       ParentContext const& iParent,
                                                       ServiceWeakToken const& weakToken) noexcept {
    CMS_SA_ALLOW try {
      ServiceRegistry::Operate guard(weakToken.lock());

      ModuleCallingContext mcc(
          &module_->moduleDescription(), iTransformIndex + 1, ModuleCallingContext::State::kRunning, iParent, nullptr);
      module_->doTransformAsync(iTask, iTransformIndex, iEvent, this->activityRegistry(), mcc, weakToken);
    } catch (...) {
      iTask.doneWaiting(std::current_exception());
      return;
    }
    iTask.doneWaiting(std::exception_ptr());
  }

#define EDM_SPECIALIZE_WORKERT_TRANSFORM(T, TI, TP)     \
  template <>                                           \
  inline void WorkerT<T, TI, TP>::implDoTransformAsync( \
      WaitingTaskHolder, size_t, EventPrincipal const&, ParentContext const&, ServiceWeakToken const&) noexcept {}

  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_TRANSFORM, global::EDAnalyzerBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_TRANSFORM, limited::EDAnalyzerBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_TRANSFORM, one::EDAnalyzerBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_TRANSFORM, stream::EDAnalyzerAdaptorBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_TRANSFORM, global::OutputModuleBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_TRANSFORM, limited::OutputModuleBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_TRANSFORM, one::OutputModuleBase)

#undef EDM_SPECIALIZE_WORKERT_TRANSFORM

  template <typename T, typename TI, typename TP>
  inline size_t WorkerT<T, TI, TP>::transformIndex(edm::ProductDescription const&) const noexcept {
    return -1;
  }
  template <>
  inline size_t WorkerT<global::EDFilterBase, EventTransitionInfo, TransitionPhaseGlobal>::transformIndex(
      edm::ProductDescription const& iBranch) const noexcept {
    return module_->transformIndex_(iBranch);
  }
  template <>
  inline size_t WorkerT<global::EDProducerBase, EventTransitionInfo, TransitionPhaseGlobal>::transformIndex(
      edm::ProductDescription const& iBranch) const noexcept {
    return module_->transformIndex_(iBranch);
  }
  template <>
  inline size_t WorkerT<stream::EDProducerAdaptorBase, EventTransitionInfo, TransitionPhaseGlobal>::transformIndex(
      edm::ProductDescription const& iBranch) const noexcept {
    return module_->transformIndex_(iBranch);
  }
  template <>
  inline size_t WorkerT<limited::EDFilterBase, EventTransitionInfo, TransitionPhaseGlobal>::transformIndex(
      edm::ProductDescription const& iBranch) const noexcept {
    return module_->transformIndex_(iBranch);
  }
  template <>
  inline size_t WorkerT<limited::EDProducerBase, EventTransitionInfo, TransitionPhaseGlobal>::transformIndex(
      edm::ProductDescription const& iBranch) const noexcept {
    return module_->transformIndex_(iBranch);
  }
  template <>
  inline size_t WorkerT<one::EDFilterBase, EventTransitionInfo, TransitionPhaseGlobal>::transformIndex(
      edm::ProductDescription const& iBranch) const noexcept {
    return module_->transformIndex_(iBranch);
  }
  template <>
  inline size_t WorkerT<one::EDProducerBase, EventTransitionInfo, TransitionPhaseGlobal>::transformIndex(
      edm::ProductDescription const& iBranch) const noexcept {
    return module_->transformIndex_(iBranch);
  }

  template <typename T, typename TI, typename TP>
  inline ProductResolverIndex WorkerT<T, TI, TP>::itemToGetForTransform(size_t iTransformIndex) const noexcept {
    return -1;
  }
  template <>
  inline ProductResolverIndex
  WorkerT<global::EDFilterBase, EventTransitionInfo, TransitionPhaseGlobal>::itemToGetForTransform(
      size_t iTransformIndex) const noexcept {
    return module_->transformPrefetch_(iTransformIndex);
  }
  template <>
  inline ProductResolverIndex
  WorkerT<global::EDProducerBase, EventTransitionInfo, TransitionPhaseGlobal>::itemToGetForTransform(
      size_t iTransformIndex) const noexcept {
    return module_->transformPrefetch_(iTransformIndex);
  }
  template <>
  inline ProductResolverIndex
  WorkerT<stream::EDProducerAdaptorBase, EventTransitionInfo, TransitionPhaseGlobal>::itemToGetForTransform(
      size_t iTransformIndex) const noexcept {
    return module_->transformPrefetch_(iTransformIndex);
  }
  template <>
  inline ProductResolverIndex
  WorkerT<limited::EDFilterBase, EventTransitionInfo, TransitionPhaseGlobal>::itemToGetForTransform(
      size_t iTransformIndex) const noexcept {
    return module_->transformPrefetch_(iTransformIndex);
  }
  template <>
  inline ProductResolverIndex
  WorkerT<limited::EDProducerBase, EventTransitionInfo, TransitionPhaseGlobal>::itemToGetForTransform(
      size_t iTransformIndex) const noexcept {
    return module_->transformPrefetch_(iTransformIndex);
  }
  template <>
  inline ProductResolverIndex
  WorkerT<one::EDFilterBase, EventTransitionInfo, TransitionPhaseGlobal>::itemToGetForTransform(
      size_t iTransformIndex) const noexcept {
    return module_->transformPrefetch_(iTransformIndex);
  }
  template <>
  inline ProductResolverIndex
  WorkerT<one::EDProducerBase, EventTransitionInfo, TransitionPhaseGlobal>::itemToGetForTransform(
      size_t iTransformIndex) const noexcept {
    return module_->transformPrefetch_(iTransformIndex);
  }

  template <typename T, typename TI, typename TP>
  inline bool WorkerT<T, TI, TP>::implNeedToRunSelection() const noexcept {
    return false;
  }

  template <typename T, typename TI, typename TP>
  inline bool WorkerT<T, TI, TP>::implDoPrePrefetchSelection(StreamID id,
                                                             EventPrincipal const& ep,
                                                             ModuleCallingContext const* mcc) {
    return true;
  }
  template <typename T, typename TI, typename TP>
  inline void WorkerT<T, TI, TP>::itemsToGetForSelection(std::vector<ProductResolverIndexAndSkipBit>&) const {}

#define EDM_SPECIALIZE_WORKERT_OUTPUT_SELECTION(T, TI, TP)                                                            \
  template <>                                                                                                         \
  inline bool WorkerT<T, TI, TP>::implNeedToRunSelection() const noexcept {                                           \
    return true;                                                                                                      \
  }                                                                                                                   \
  template <>                                                                                                         \
  inline bool WorkerT<T, TI, TP>::implDoPrePrefetchSelection(                                                         \
      StreamID id, EventPrincipal const& ep, ModuleCallingContext const* mcc) {                                       \
    return module_->prePrefetchSelection(id, ep, mcc);                                                                \
  }                                                                                                                   \
  template <>                                                                                                         \
  inline void WorkerT<T, TI, TP>::itemsToGetForSelection(std::vector<ProductResolverIndexAndSkipBit>& iItems) const { \
    iItems = module_->productsUsedBySelection();                                                                      \
  }

  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_OUTPUT_SELECTION, edm::one::OutputModuleBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_OUTPUT_SELECTION, edm::global::OutputModuleBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_OUTPUT_SELECTION, edm::limited::OutputModuleBase)

#undef EDM_SPECIALIZE_WORKERT_OUTPUT_SELECTION

  template <typename T, typename TI, typename TP>
  bool WorkerT<T, TI, TP>::implDoBeginProcessBlock(ProcessBlockPrincipal const& pbp, ModuleCallingContext const* mcc) {
    module_->doBeginProcessBlock(pbp, mcc);
    return true;
  }

  template <typename T, typename TI, typename TP>
  bool WorkerT<T, TI, TP>::implDoAccessInputProcessBlock(ProcessBlockPrincipal const& pbp,
                                                         ModuleCallingContext const* mcc) {
    module_->doAccessInputProcessBlock(pbp, mcc);
    return true;
  }

  template <typename T, typename TI, typename TP>
  bool WorkerT<T, TI, TP>::implDoEndProcessBlock(ProcessBlockPrincipal const& pbp, ModuleCallingContext const* mcc) {
    module_->doEndProcessBlock(pbp, mcc);
    return true;
  }

  template <typename T, typename TI, typename TP>
  inline bool WorkerT<T, TI, TP>::implDoBegin(RunTransitionInfo const& info, ModuleCallingContext const* mcc) {
    module_->doBeginRun(info, mcc);
    return true;
  }

  template <typename T, typename TI, typename TP>
  template <typename D>
  void WorkerT<T, TI, TP>::callWorkerStreamBegin(D,
                                                 StreamID id,
                                                 RunTransitionInfo const& info,
                                                 ModuleCallingContext const* mcc) {
    module_->doStreamBeginRun(id, info, mcc);
  }

  template <typename T, typename TI, typename TP>
  template <typename D>
  void WorkerT<T, TI, TP>::callWorkerStreamEnd(D,
                                               StreamID id,
                                               RunTransitionInfo const& info,
                                               ModuleCallingContext const* mcc) {
    module_->doStreamEndRun(id, info, mcc);
  }

  template <typename T, typename TI, typename TP>
  inline bool WorkerT<T, TI, TP>::implDoStreamBegin(StreamID id,
                                                    RunTransitionInfo const& info,
                                                    ModuleCallingContext const* mcc) {
    std::conditional_t<workerimpl::has_stream_functions<T>::value,
                       workerimpl::DoStreamBeginTrans<T, TI, TP, RunTransitionInfo const>,
                       workerimpl::DoNothing>
        might_call;
    might_call(this, id, info, mcc);
    return true;
  }

  template <typename T, typename TI, typename TP>
  inline bool WorkerT<T, TI, TP>::implDoStreamEnd(StreamID id,
                                                  RunTransitionInfo const& info,
                                                  ModuleCallingContext const* mcc) {
    std::conditional_t<workerimpl::has_stream_functions<T>::value,
                       workerimpl::DoStreamEndTrans<T, TI, TP, RunTransitionInfo const>,
                       workerimpl::DoNothing>
        might_call;
    might_call(this, id, info, mcc);
    return true;
  }

  template <typename T, typename TI, typename TP>
  inline bool WorkerT<T, TI, TP>::implDoEnd(RunTransitionInfo const& info, ModuleCallingContext const* mcc) {
    module_->doEndRun(info, mcc);
    return true;
  }

  template <typename T, typename TI, typename TP>
  inline bool WorkerT<T, TI, TP>::implDoWrite(RunTransitionInfo const& info, ModuleCallingContext const* mcc) {
    return true;
  }
#define EDM_SPECIALIZE_WORKERT_OUTPUT_RUN_WRITE(T, TI, TP)                                                      \
  template <>                                                                                                   \
  inline bool WorkerT<T, TI, TP>::implDoWrite(RunTransitionInfo const& info, ModuleCallingContext const* mcc) { \
    module_->doWriteRun(info.principal(), mcc);                                                                 \
    return true;                                                                                                \
  }

  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_OUTPUT_RUN_WRITE, edm::one::OutputModuleBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_OUTPUT_RUN_WRITE, edm::global::OutputModuleBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_OUTPUT_RUN_WRITE, edm::limited::OutputModuleBase)

#undef EDM_SPECIALIZE_WORKERT_OUTPUT_RUN_WRITE

  template <typename T, typename TI, typename TP>
  inline bool WorkerT<T, TI, TP>::implDoBegin(LumiTransitionInfo const& info, ModuleCallingContext const* mcc) {
    module_->doBeginLuminosityBlock(info, mcc);
    return true;
  }

  template <typename T, typename TI, typename TP>
  template <typename D>
  void WorkerT<T, TI, TP>::callWorkerStreamBegin(D,
                                                 StreamID id,
                                                 LumiTransitionInfo const& info,
                                                 ModuleCallingContext const* mcc) {
    module_->doStreamBeginLuminosityBlock(id, info, mcc);
  }

  template <typename T, typename TI, typename TP>
  template <typename D>
  void WorkerT<T, TI, TP>::callWorkerStreamEnd(D,
                                               StreamID id,
                                               LumiTransitionInfo const& info,
                                               ModuleCallingContext const* mcc) {
    module_->doStreamEndLuminosityBlock(id, info, mcc);
  }

  template <typename T, typename TI, typename TP>
  inline bool WorkerT<T, TI, TP>::implDoStreamBegin(StreamID id,
                                                    LumiTransitionInfo const& info,
                                                    ModuleCallingContext const* mcc) {
    std::conditional_t<workerimpl::has_stream_functions<T>::value,
                       workerimpl::DoStreamBeginTrans<T, TI, TP, LumiTransitionInfo>,
                       workerimpl::DoNothing>
        might_call;
    might_call(this, id, info, mcc);
    return true;
  }

  template <typename T, typename TI, typename TP>
  inline bool WorkerT<T, TI, TP>::implDoStreamEnd(StreamID id,
                                                  LumiTransitionInfo const& info,
                                                  ModuleCallingContext const* mcc) {
    std::conditional_t<workerimpl::has_stream_functions<T>::value,
                       workerimpl::DoStreamEndTrans<T, TI, TP, LumiTransitionInfo>,
                       workerimpl::DoNothing>
        might_call;
    might_call(this, id, info, mcc);

    return true;
  }

  template <typename T, typename TI, typename TP>
  inline bool WorkerT<T, TI, TP>::implDoEnd(LumiTransitionInfo const& info, ModuleCallingContext const* mcc) {
    module_->doEndLuminosityBlock(info, mcc);
    return true;
  }

  template <typename T, typename TI, typename TP>
  inline bool WorkerT<T, TI, TP>::implDoWrite(LumiTransitionInfo const& info, ModuleCallingContext const* mcc) {
    return true;
  }
#define EDM_SPECIALIZE_WORKERT_OUTPUT_LUMI_WRITE(T, TI, TP)                                                      \
  template <>                                                                                                    \
  inline bool WorkerT<T, TI, TP>::implDoWrite(LumiTransitionInfo const& info, ModuleCallingContext const* mcc) { \
    module_->doWriteLuminosityBlock(info.principal(), mcc);                                                      \
    return true;                                                                                                 \
  }

  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_OUTPUT_LUMI_WRITE, edm::one::OutputModuleBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_OUTPUT_LUMI_WRITE, edm::global::OutputModuleBase)
  EDM_FOR_EACH_WORKERT_TRANSITION(EDM_SPECIALIZE_WORKERT_OUTPUT_LUMI_WRITE, edm::limited::OutputModuleBase)

#undef EDM_SPECIALIZE_WORKERT_OUTPUT_LUMI_WRITE

  template <typename T, typename TI, typename TP>
  template <typename D>
  void WorkerT<T, TI, TP>::callWorkerBeginStream(D, StreamID id) {
    module_->doBeginStream(id);
  }

  template <typename T, typename TI, typename TP>
  template <typename D>
  void WorkerT<T, TI, TP>::callWorkerEndStream(D, StreamID id) {
    module_->doEndStream(id);
  }

  template <typename T, typename TI, typename TP>
  inline typename TransitionWorker<TI, TP>::TaskQueueAdaptor WorkerT<T, TI, TP>::serializeRunModule() {
    return typename TransitionWorker<TI, TP>::TaskQueueAdaptor{};
  }
#define EDM_SPECIALIZE_WORKERT_FOR_TRANSITION(T, TYPE, CONCURRENCY, QUEUE, TI, TP) \
  template <>                                                                      \
  Worker::TaskQueueAdaptor WorkerT<T, TI, TP>::serializeRunModule() {              \
    return QUEUE;                                                                  \
  }                                                                                \
  template <>                                                                      \
  Worker::Types WorkerT<T, TI, TP>::moduleType() const {                           \
    return Worker::Types::TYPE;                                                    \
  }                                                                                \
  template <>                                                                      \
  Worker::ConcurrencyTypes WorkerT<T, TI, TP>::moduleConcurrencyType() const {     \
    return Worker::ConcurrencyTypes::CONCURRENCY;                                  \
  }

#define EDM_SPECIALIZE_WORKERT(T, TYPE, CONCURRENCY, QUEUE)                                                      \
  EDM_SPECIALIZE_WORKERT_FOR_TRANSITION(T, TYPE, CONCURRENCY, QUEUE, RunTransitionInfo, TransitionPhaseGlobal)   \
  EDM_SPECIALIZE_WORKERT_FOR_TRANSITION(T, TYPE, CONCURRENCY, QUEUE, RunTransitionInfo, TransitionPhaseStream)   \
  EDM_SPECIALIZE_WORKERT_FOR_TRANSITION(T, TYPE, CONCURRENCY, QUEUE, LumiTransitionInfo, TransitionPhaseGlobal)  \
  EDM_SPECIALIZE_WORKERT_FOR_TRANSITION(T, TYPE, CONCURRENCY, QUEUE, LumiTransitionInfo, TransitionPhaseStream)  \
  EDM_SPECIALIZE_WORKERT_FOR_TRANSITION(                                                                         \
      T, TYPE, CONCURRENCY, QUEUE, ProcessBlockTransitionInfo, TransitionPhaseGlobal)                            \
  EDM_SPECIALIZE_WORKERT_FOR_TRANSITION(                                                                         \
      T, TYPE, CONCURRENCY, QUEUE, InputProcessBlockTransitionInfo, TransitionPhaseGlobal)                       \
  EDM_SPECIALIZE_WORKERT_FOR_TRANSITION(T, TYPE, CONCURRENCY, QUEUE, EventTransitionInfo, TransitionPhaseGlobal) \
  EDM_SPECIALIZE_WORKERT_FOR_TRANSITION(T, TYPE, CONCURRENCY, QUEUE, EventTransitionInfo, TransitionPhaseStream)

  EDM_SPECIALIZE_WORKERT(one::EDProducerBase, kProducer, kOne, &(module_->sharedResourcesAcquirer().serialQueueChain()))
  EDM_SPECIALIZE_WORKERT(one::EDFilterBase, kFilter, kOne, &(module_->sharedResourcesAcquirer().serialQueueChain()))
  EDM_SPECIALIZE_WORKERT(one::EDAnalyzerBase, kAnalyzer, kOne, &(module_->sharedResourcesAcquirer().serialQueueChain()))
  EDM_SPECIALIZE_WORKERT(one::OutputModuleBase,
                         kOutputModule,
                         kOne,
                         &(module_->sharedResourcesAcquirer().serialQueueChain()))
  EDM_SPECIALIZE_WORKERT(global::EDProducerBase, kProducer, kGlobal, Worker::TaskQueueAdaptor{})
  EDM_SPECIALIZE_WORKERT(global::EDFilterBase, kFilter, kGlobal, Worker::TaskQueueAdaptor{})
  EDM_SPECIALIZE_WORKERT(global::EDAnalyzerBase, kAnalyzer, kGlobal, Worker::TaskQueueAdaptor{})
  EDM_SPECIALIZE_WORKERT(global::OutputModuleBase, kOutputModule, kGlobal, Worker::TaskQueueAdaptor{})
  EDM_SPECIALIZE_WORKERT(limited::EDProducerBase, kProducer, kLimited, &(module_->queue()))
  EDM_SPECIALIZE_WORKERT(limited::EDFilterBase, kFilter, kLimited, &(module_->queue()))
  EDM_SPECIALIZE_WORKERT(limited::EDAnalyzerBase, kAnalyzer, kLimited, &(module_->queue()))
  EDM_SPECIALIZE_WORKERT(limited::OutputModuleBase, kOutputModule, kLimited, &(module_->queue()))
  EDM_SPECIALIZE_WORKERT(stream::EDProducerAdaptorBase, kProducer, kStream, Worker::TaskQueueAdaptor{})
  EDM_SPECIALIZE_WORKERT(stream::EDFilterAdaptorBase, kFilter, kStream, Worker::TaskQueueAdaptor{})
  EDM_SPECIALIZE_WORKERT(stream::EDAnalyzerAdaptorBase, kAnalyzer, kStream, Worker::TaskQueueAdaptor{})

#undef EDM_SPECIALIZE_WORKERT
#undef EDM_SPECIALIZE_WORKERT_FOR_TRANSITION

  //Explicitly instantiate our needed templates to avoid having the compiler
  // instantiate them in all of our libraries
#define EDM_INSTANTIATE_WORKERT(T)                                                   \
  template class WorkerT<T, RunTransitionInfo, TransitionPhaseGlobal>;               \
  template class WorkerT<T, RunTransitionInfo, TransitionPhaseStream>;               \
  template class WorkerT<T, LumiTransitionInfo, TransitionPhaseGlobal>;              \
  template class WorkerT<T, LumiTransitionInfo, TransitionPhaseStream>;              \
  template class WorkerT<T, ProcessBlockTransitionInfo, TransitionPhaseGlobal>;      \
  template class WorkerT<T, InputProcessBlockTransitionInfo, TransitionPhaseGlobal>; \
  template class WorkerT<T, EventTransitionInfo, TransitionPhaseGlobal>;             \
  template class WorkerT<T, EventTransitionInfo, TransitionPhaseStream>;

  EDM_INSTANTIATE_WORKERT(one::EDProducerBase)
  EDM_INSTANTIATE_WORKERT(one::EDFilterBase)
  EDM_INSTANTIATE_WORKERT(one::EDAnalyzerBase)
  EDM_INSTANTIATE_WORKERT(one::OutputModuleBase)
  EDM_INSTANTIATE_WORKERT(global::EDProducerBase)
  EDM_INSTANTIATE_WORKERT(global::EDFilterBase)
  EDM_INSTANTIATE_WORKERT(global::EDAnalyzerBase)
  EDM_INSTANTIATE_WORKERT(global::OutputModuleBase)
  EDM_INSTANTIATE_WORKERT(stream::EDProducerAdaptorBase)
  EDM_INSTANTIATE_WORKERT(stream::EDFilterAdaptorBase)
  EDM_INSTANTIATE_WORKERT(stream::EDAnalyzerAdaptorBase)
  EDM_INSTANTIATE_WORKERT(limited::EDProducerBase)
  EDM_INSTANTIATE_WORKERT(limited::EDFilterBase)
  EDM_INSTANTIATE_WORKERT(limited::EDAnalyzerBase)
  EDM_INSTANTIATE_WORKERT(limited::OutputModuleBase)

#undef EDM_INSTANTIATE_WORKERT
#undef EDM_FOR_EACH_WORKERT_TRANSITION
}  // namespace edm
