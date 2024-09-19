#include "RNTupleInputFile.h"

#include "DataFormats/Provenance/interface/EventAuxiliary.h"
#include "DataFormats/Provenance/interface/LuminosityBlockAuxiliary.h"
#include "DataFormats/Provenance/interface/RunAuxiliary.h"
#include "DataFormats/Provenance/interface/FileID.h"
#include "DataFormats/Provenance/interface/StoredMergeableRunProductMetadata.h"
#include "DataFormats/Provenance/interface/ProcessHistory.h"
#include "DataFormats/Provenance/interface/ProductRegistry.h"
#include "DataFormats/Provenance/interface/ThinnedAssociationsHelper.h"
#include "DataFormats/Provenance/interface/BranchChildren.h"
#include "DataFormats/Provenance/interface/ProcessHistoryRegistry.h"
#include "DataFormats/Provenance/interface/Parentage.h"
#include "DataFormats/Provenance/interface/ParentageRegistry.h"

#include <cassert>
#include <iostream>
using namespace ROOT::Experimental;
namespace edm {

  RNTupleInputFile::RNTupleInputFile(std::string const& iName)
      : file_(TFile::Open(iName.c_str())),
        runs_(file_.get(), "Runs", "RunAuxiliary"),
        lumis_(file_.get(), "LuminosityBlocks", "LuminosityBlockAuxiliary"),
        events_(file_.get(), "Events", "EventAuxiliary") {}

  std::vector<ParentageID> RNTupleInputFile::readParentage() {
    auto parentageTuple = RNTupleReader::Open(*file_->Get<RNTuple>("Parentage"));
    auto entry = parentageTuple->GetModel().CreateBareEntry();

    edm::Parentage parentage;
    entry->BindRawPtr("Description", &parentage);
    std::vector<ParentageID> retValue;

    ParentageRegistry& registry = *ParentageRegistry::instance();

    retValue.reserve(parentageTuple->GetNEntries());

    for (ROOT::Experimental::NTupleSize_t i = 0; i < parentageTuple->GetNEntries(); ++i) {
      parentageTuple->LoadEntry(i, *entry);
      registry.insertMapped(parentage);
      retValue.push_back(parentage.id());
    }

    return retValue;
  }

  void RNTupleInputFile::readMeta(edm::ProductRegistry& iReg,
                                  edm::ProcessHistoryRegistry& iHist,
                                  BranchIDLists& iBranchIDLists) {
    auto meta = RNTupleReader::Open(*file_->Get<RNTuple>("MetaData"));
    assert(meta.get());

    //BEWARE, if you do not 'BindRawPtr' to all top level Fields,
    // using CreateBareEntry with LoadEntry will seg-fault!
    //auto entry = meta->GetModel().CreateBareEntry();
    auto entry = meta->GetModel().CreateEntry();

    entry->BindRawPtr("IndexIntoFile", &index_);

    edm::FileID id;
    entry->BindRawPtr("FileIdentifier", &id);

    edm::StoredMergeableRunProductMetadata mergeable;
    entry->BindRawPtr("MergeableRunProductMetadata", &mergeable);

    std::vector<edm::ProcessHistory> processHist;
    entry->BindRawPtr("ProcessHistory", &processHist);

    edm::ProductRegistry reg;
    entry->BindRawPtr("ProductRegistry", &reg);

    entry->BindRawPtr("BranchIDLists", &iBranchIDLists);

    edm::ThinnedAssociationsHelper thinned;
    entry->BindRawPtr("ThinnedAssociationsHelper", &thinned);

    edm::BranchChildren branchChildren;
    entry->BindRawPtr("ProductDependencies", &branchChildren);

    meta->LoadEntry(0, *entry);

    {
      auto& pList = reg.productListUpdator();
      for (auto& product : pList) {
        BranchDescription& prod = product.second;
        prod.initBranchName();
        if (prod.present()) {
          prod.initFromDictionary();
          prod.setOnDemand(true);
        }
        if (prod.branchType() == InEvent) {
          events_.setupToReadProductIfAvailable(prod);
        } else if (prod.branchType() == InLumi) {
          lumis_.setupToReadProductIfAvailable(prod);
        } else if (prod.branchType() == InRun) {
          runs_.setupToReadProductIfAvailable(prod);
        }
      }
    }
    reg.setFrozen(false);
    iReg.updateFromInput(reg.productList());

    for (auto const& h : processHist) {
      iHist.registerProcessHistory(h);
    }

    std::vector<ProcessHistoryID> orderedHistory;
    index_.fixIndexes(orderedHistory);
    index_.setNumberOfEvents(events_.numberOfEntries());
    //index_.setEventFinder();
    bool needEventNumbers = false;
    bool needEventEntries = false;
    index_.fillEventNumbersOrEntries(needEventNumbers, needEventEntries);

    iter_ = index_.begin(IndexIntoFile::firstAppearanceOrder);
    iterEnd_ = index_.end(IndexIntoFile::firstAppearanceOrder);
  }

  IndexIntoFile::EntryType RNTupleInputFile::getNextItemType() {
    if (*iter_ == *iterEnd_) {
      return IndexIntoFile::kEnd;
    }
    return iter_->getEntryType();
  }

  IndexIntoFile::EntryNumber_t RNTupleInputFile::readLuminosityBlock() {
    assert(*iter_ != *iterEnd_);
    assert(iter_->getEntryType() == IndexIntoFile::kLumi);
    auto v = iter_->entry();
    ++(*iter_);
    return v;
  }

  std::shared_ptr<LuminosityBlockAuxiliary> RNTupleInputFile::readLuminosityBlockAuxiliary() {
    auto lumiAux = std::make_shared<LuminosityBlockAuxiliary>();
    assert(*iter_ != *iterEnd_);
    assert(iter_->getEntryType() == IndexIntoFile::kLumi);
    auto view = lumis_.auxView(lumiAux);
    view(iter_->entry());
    return lumiAux;
  }

  IndexIntoFile::EntryNumber_t RNTupleInputFile::readEvent() {
    assert(*iter_ != *iterEnd_);
    assert(iter_->getEntryType() == IndexIntoFile::kEvent);
    auto v = iter_->entry();
    ++(*iter_);
    return v;
  }

  std::shared_ptr<EventAuxiliary> RNTupleInputFile::readEventAuxiliary() {
    auto eventAux = std::make_shared<EventAuxiliary>();
    assert(*iter_ != *iterEnd_);
    assert(iter_->getEntryType() == IndexIntoFile::kEvent);
    auto view = events_.auxView(eventAux);
    view(iter_->entry());
    return eventAux;
  }

  std::shared_ptr<RunAuxiliary> RNTupleInputFile::readRunAuxiliary() {
    auto runAux = std::make_shared<RunAuxiliary>();
    assert(*iter_ != *iterEnd_);
    assert(iter_->getEntryType() == IndexIntoFile::kRun);

    auto view = runs_.auxView(runAux);
    view(iter_->entry());
    return runAux;
  }

  IndexIntoFile::EntryNumber_t RNTupleInputFile::readRun() {
    assert(*iter_ != *iterEnd_);
    assert(iter_->getEntryType() == IndexIntoFile::kRun);
    auto v = iter_->entry();
    ++(*iter_);
    return v;
  }

}  // namespace edm
