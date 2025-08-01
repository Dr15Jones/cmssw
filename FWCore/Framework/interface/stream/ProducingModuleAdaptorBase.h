#ifndef FWCore_Framework_stream_ProducingModuleAdaptorBase_h
#define FWCore_Framework_stream_ProducingModuleAdaptorBase_h
// -*- C++ -*-
//
// Package:     FWCore/Framework
// Class  :     ProducingModuleAdaptorBase
//
/**\class edm::stream::ProducingModuleAdaptorBase ProducingModuleAdaptorBase.h "FWCore/Framework/interface/stream/ProducingModuleAdaptorBase.h"

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Fri, 02 Aug 2013 18:09:15 GMT
//

// system include files
#include <array>
#include <map>
#include <string>
#include <vector>
#include <unordered_map>

// user include files
#include "FWCore/Utilities/interface/ProductResolverIndex.h"
#include "FWCore/Common/interface/FWCoreCommonFwd.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "DataFormats/Provenance/interface/ModuleDescription.h"
#include "FWCore/ParameterSet/interface/ParameterSetfwd.h"
#include "FWCore/Utilities/interface/BranchType.h"
#include "FWCore/Utilities/interface/StreamID.h"
#include "FWCore/Utilities/interface/RunIndex.h"
#include "FWCore/Utilities/interface/LuminosityBlockIndex.h"
#include "FWCore/Utilities/interface/ESIndices.h"
#include "FWCore/Utilities/interface/Transition.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ProcessBlock.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/ServiceRegistry/interface/ServiceRegistryfwd.h"

// forward declarations

namespace edm {
  class Event;
  class ModuleCallingContext;
  class ProductResolverIndexHelper;
  class EDConsumerBase;
  class PreallocationConfiguration;
  class ProductResolverIndexAndSkipBit;
  class ThinnedAssociationsHelper;
  class ActivityRegistry;
  class WaitingTaskHolder;
  class ServiceWeakToken;
  class SignallingProductRegistryFiller;
  struct ModuleConsumesMinimalESInfo;

  namespace maker {
    template <typename T>
    class ModuleHolderT;
  }

  namespace eventsetup {
    struct ComponentDescription;
    class ESRecordsToProductResolverIndices;
  }  // namespace eventsetup

  namespace stream {
    template <typename T>
    class ProducingModuleAdaptorBase {
    public:
      template <typename U>
      friend class edm::WorkerT;
      template <typename U>
      friend class edm::maker::ModuleHolderT;

      ProducingModuleAdaptorBase();
      ProducingModuleAdaptorBase(const ProducingModuleAdaptorBase&) = delete;                   // stop default
      const ProducingModuleAdaptorBase& operator=(const ProducingModuleAdaptorBase&) = delete;  // stop default
      virtual ~ProducingModuleAdaptorBase();

      // ---------- const member functions ---------------------

      // ---------- static member functions --------------------

      // ---------- member functions ---------------------------
      const ModuleDescription& moduleDescription() const noexcept { return moduleDescription_; }

      virtual bool wantsProcessBlocks() const noexcept = 0;
      virtual bool wantsInputProcessBlocks() const noexcept = 0;
      virtual bool wantsGlobalRuns() const noexcept = 0;
      virtual bool wantsGlobalLuminosityBlocks() const noexcept = 0;
      virtual bool hasAcquire() const noexcept = 0;
      virtual bool hasAccumulator() const noexcept = 0;
      virtual bool wantsStreamRuns() const noexcept = 0;
      virtual bool wantsStreamLuminosityBlocks() const noexcept = 0;

      void registerProductsAndCallbacks(ProducingModuleAdaptorBase const*, SignallingProductRegistryFiller* reg);

      void itemsToGet(BranchType, std::vector<ProductResolverIndexAndSkipBit>&) const;
      void itemsMayGet(BranchType, std::vector<ProductResolverIndexAndSkipBit>&) const;
      std::vector<ProductResolverIndexAndSkipBit> const& itemsToGetFrom(BranchType) const;

      std::vector<ESResolverIndex> const& esGetTokenIndicesVector(edm::Transition iTrans) const;
      std::vector<ESRecordIndex> const& esGetTokenRecordIndicesVector(edm::Transition iTrans) const;

      void updateLookup(BranchType iBranchType, ProductResolverIndexHelper const&, bool iPrefetchMayGet);
      void updateLookup(eventsetup::ESRecordsToProductResolverIndices const&);
      void releaseMemoryPostLookupSignal();
      virtual void selectInputProcessBlocks(ProductRegistry const&, ProcessBlockHelperBase const&) = 0;

      void convertCurrentProcessAlias(std::string const& processName);

      std::vector<ModuleConsumesInfo> moduleConsumesInfos() const;
      std::vector<ModuleConsumesMinimalESInfo> moduleConsumesMinimalESInfos() const;

      using ModuleToResolverIndicies =
          std::unordered_multimap<std::string, std::tuple<edm::TypeID const*, const char*, edm::ProductResolverIndex>>;

      void resolvePutIndicies(BranchType iBranchType,
                              ModuleToResolverIndicies const& iIndicies,
                              std::string const& moduleLabel);

      std::vector<edm::ProductResolverIndex> const& indiciesForPutProducts(BranchType iBranchType) const;

      ProductResolverIndex transformPrefetch_(size_t iTransformIndex) const noexcept;
      size_t transformIndex_(edm::ProductDescription const& iBranch) const noexcept;
      void doTransformAsync(WaitingTaskHolder iTask,
                            size_t iTransformIndex,
                            EventPrincipal const& iEvent,
                            ActivityRegistry*,
                            ModuleCallingContext,
                            ServiceWeakToken const&) noexcept;

    protected:
      template <typename F>
      void createStreamModules(F iFunc) {
        unsigned int iStreamModule = 0;
        for (auto& m : m_streamModules) {
          m = iFunc(iStreamModule);
          m->setModuleDescriptionPtr(&moduleDescription_);
          ++iStreamModule;
        }
      }

      void commit(ProcessBlock& iProcessBlock) {
        iProcessBlock.commit_(m_streamModules[0]->indiciesForPutProducts(InProcess));
      }
      void commit(Run& iRun) { iRun.commit_(m_streamModules[0]->indiciesForPutProducts(InRun)); }
      void commit(LuminosityBlock& iLumi) { iLumi.commit_(m_streamModules[0]->indiciesForPutProducts(InLumi)); }
      template <typename I>
      void commit(Event& iEvent, I* iID) {
        iEvent.commit_(m_streamModules[0]->indiciesForPutProducts(InEvent), iID);
      }

      const EDConsumerBase* consumer() { return m_streamModules[0]; }

      const ProducerBase* producer() { return m_streamModules[0]; }

      void deleteModulesEarly();

    private:
      void doPreallocate(PreallocationConfiguration const&);
      virtual void preallocRuns(unsigned int) {}
      virtual void preallocLumis(unsigned int) {}
      virtual void setupStreamModules() = 0;
      virtual void doBeginJob() = 0;
      virtual void doEndJob() = 0;

      void doBeginStream(StreamID);
      void doEndStream(StreamID);
      void doStreamBeginRun(StreamID, RunTransitionInfo const&, ModuleCallingContext const*);
      virtual void setupRun(T*, RunIndex) = 0;
      void doStreamEndRun(StreamID, RunTransitionInfo const&, ModuleCallingContext const*);
      virtual void streamEndRunSummary(T*, edm::Run const&, edm::EventSetup const&) = 0;

      void doStreamBeginLuminosityBlock(StreamID, LumiTransitionInfo const&, ModuleCallingContext const*);
      virtual void setupLuminosityBlock(T*, LuminosityBlockIndex) = 0;
      void doStreamEndLuminosityBlock(StreamID, LumiTransitionInfo const&, ModuleCallingContext const*);
      virtual void streamEndLuminosityBlockSummary(T*, edm::LuminosityBlock const&, edm::EventSetup const&) = 0;

      virtual void doBeginProcessBlock(ProcessBlockPrincipal const&, ModuleCallingContext const*) = 0;
      virtual void doAccessInputProcessBlock(ProcessBlockPrincipal const&, ModuleCallingContext const*) = 0;
      virtual void doEndProcessBlock(ProcessBlockPrincipal const&, ModuleCallingContext const*) = 0;
      virtual void doBeginRun(RunTransitionInfo const&, ModuleCallingContext const*) = 0;
      virtual void doEndRun(RunTransitionInfo const&, ModuleCallingContext const*) = 0;
      virtual void doBeginLuminosityBlock(LumiTransitionInfo const&, ModuleCallingContext const*) = 0;
      virtual void doEndLuminosityBlock(LumiTransitionInfo const&, ModuleCallingContext const*) = 0;

      virtual void doRespondToCloseOutputFile() = 0;
      void doRegisterThinnedAssociations(ProductRegistry const&, ThinnedAssociationsHelper&);

      // ---------- member data --------------------------------
      void setModuleDescription(ModuleDescription const& md) { moduleDescription_ = md; }
      ModuleDescription moduleDescription_;

    protected:
      std::vector<T*> m_streamModules;
    };
  }  // namespace stream
}  // namespace edm

#endif
