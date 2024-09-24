#include "RNTupleOutputFile.h"

#include "FWCore/Framework/interface/RunForOutput.h"
#include "FWCore/Framework/interface/LuminosityBlockForOutput.h"
#include "FWCore/Framework/interface/EventForOutput.h"
#include "FWCore/Framework/interface/ConstProductRegistry.h"
#include "FWCore/Framework/interface/FileBlock.h"
#include "FWCore/Framework/interface/ProductProvenanceRetriever.h"
#include "FWCore/Framework/interface/ConstProductRegistry.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ParameterSet/interface/Registry.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/GlobalIdentifier.h"
#include "FWCore/Utilities/interface/ConvertException.h"

#include "DataFormats/Provenance/interface/ParentageRegistry.h"
#include "DataFormats/Provenance/interface/FileID.h"
#include "DataFormats/Provenance/interface/ProcessHistoryRegistry.h"
#include "DataFormats/Provenance/interface/Provenance.h"
#include "DataFormats/Provenance/interface/IndexIntoFile.h"
#include "DataFormats/Provenance/interface/BranchChildren.h"

#include "IOPool/Common/interface/getWrapperBasePtr.h"

#include "TFile.h"
#include "ROOT/RNTuple.hxx"
#include "ROOT/RNTupleWriter.hxx"
#include <string>
#include <optional>
#include <map>

namespace {
  ROOT::RCompressionSetting::EAlgorithm::EValues convert(edm::rntuple::CompressionAlgos iAlgos) {
    using namespace edm::rntuple;
    using namespace ROOT;
    switch (iAlgos) {
      case CompressionAlgos::kLZMA:
        return RCompressionSetting::EAlgorithm::kLZMA;
      case CompressionAlgos::kZSTD:
        return RCompressionSetting::EAlgorithm::kZSTD;
      case CompressionAlgos::kZLIB:
        return RCompressionSetting::EAlgorithm::kZLIB;
      case CompressionAlgos::kLZ4:
        return RCompressionSetting::EAlgorithm::kLZ4;
    }
    return RCompressionSetting::EAlgorithm::kZSTD;
  }
}  // namespace

using namespace ROOT::Experimental;
namespace edm {
  RNTupleOutputFile::RNTupleOutputFile(std::string const& iFileName,
                                       FileBlock const& iFileBlock,
                                       SelectedProductsForBranchType const& iSelected,
                                       Config const& iConfig)
      : file_(iFileName.c_str(), "recreate", ""),
        wrapperBaseTClass_(TClass::GetClass("edm::WrapperBase")),
        selectorConfig_(iConfig.selectorConfig),
        dropMetaData_(iConfig.dropMetaData) {
    setupRuns(iSelected[InRun], iConfig);
    setupLumis(iSelected[InLumi], iConfig);
    setupEvents(iSelected[InEvent], iConfig);
    setupPSets(iConfig);
    setupParentage(iConfig);
    setupMetaData(iConfig);

    auto const& branchToChildMap = iFileBlock.branchChildren().childLookup();
    for (auto const& parentToChildren : branchToChildMap) {
      for (auto const& child : parentToChildren.second) {
        branchChildren_.insertChild(parentToChildren.first, child);
      }
    }
  }

  namespace {
    std::string fixBranchName(std::string const& iName) {
      //need to remove the '.' at the end of the branch name
      return iName.substr(0, iName.size() - 1);
    }
  }  // namespace

  void RNTupleOutputFile::setupDataProducts(SelectedProducts const& iProducts,
                                            std::vector<bool> const& iDoNotSplit,
                                            RNTupleModel& iModel) {
    unsigned int index = 0;
    for (auto const& prod : iProducts) {
      try {
        edm::convertException::wrap([&]() {
          if (index >= iDoNotSplit.size() or not iDoNotSplit[index]) {
            auto field = ROOT::Experimental::RFieldBase::Create(fixBranchName(prod.first->branchName()),
                                                                prod.first->wrappedName())
                             .Unwrap();
            iModel.AddField(std::move(field));
            branchesWithStoredHistory_.insert(prod.first->branchID());
          } else {
            auto field = std::make_unique<ROOT::Experimental::RUnsplitField>(fixBranchName(prod.first->branchName()),
                                                                             prod.first->wrappedName());
            iModel.AddField(std::move(field));
            branchesWithStoredHistory_.insert(prod.first->branchID());
          }
        });
        ++index;
      } catch (cms::Exception& iExcept) {
        std::ostringstream s;
        s << "while setting up field " << prod.first->branchName();
        iExcept.addContext(s.str());
        throw;
      }
    }
  }

  std::vector<RNTupleOutputFile::Product> RNTupleOutputFile::associateDataProducts(SelectedProducts const& iProducts,
                                                                                   RNTupleModel const& iModel) {
    std::vector<Product> ret;
    ret.reserve(iProducts.size());
    for (auto const& prod : iProducts) {
      ret.emplace_back(prod.second, prod.first, iModel.GetToken(fixBranchName(prod.first->branchName())));
    }
    return ret;
  }

  void RNTupleOutputFile::setupRuns(SelectedProducts const& iProducts, Config const& iConfig) {
    std::string kRunAuxName = "RunAuxiliary";
    {
      auto model = ROOT::Experimental::RNTupleModel::CreateBare();
      {
        auto field = ROOT::Experimental::RFieldBase::Create(kRunAuxName, "edm::RunAuxiliary").Unwrap();
        model->AddField(std::move(field));
      }
      const std::vector<bool> unsplitNothing;
      setupDataProducts(iProducts, unsplitNothing, *model);

      auto writeOptions = ROOT::Experimental::RNTupleWriteOptions();
      writeOptions.SetCompression(convert(iConfig.compressionAlgo), iConfig.compressionLevel);
      writeOptions.SetUseTailPageOptimization(iConfig.useTailPageOptimization);
      runs_ = ROOT::Experimental::RNTupleWriter::Append(std::move(model), "Runs", file_, writeOptions);
    }
    products_[InRun] = associateDataProducts(iProducts, runs_->GetModel());
    runAuxField_ = runs_->GetModel().GetToken(kRunAuxName);
  }
  void RNTupleOutputFile::setupLumis(SelectedProducts const& iProducts, Config const& iConfig) {
    std::string kLumiAuxName = "LuminosityBlockAuxiliary";
    {
      auto model = ROOT::Experimental::RNTupleModel::CreateBare();
      {
        auto field = ROOT::Experimental::RFieldBase::Create(kLumiAuxName, "edm::LuminosityBlockAuxiliary").Unwrap();
        model->AddField(std::move(field));
      }
      const std::vector<bool> unsplitNothing;
      setupDataProducts(iProducts, unsplitNothing, *model);

      auto writeOptions = ROOT::Experimental::RNTupleWriteOptions();
      writeOptions.SetCompression(convert(iConfig.compressionAlgo), iConfig.compressionLevel);
      writeOptions.SetUseTailPageOptimization(iConfig.useTailPageOptimization);
      lumis_ = ROOT::Experimental::RNTupleWriter::Append(std::move(model), "LuminosityBlocks", file_, writeOptions);
    }
    products_[InLumi] = associateDataProducts(iProducts, lumis_->GetModel());
    lumiAuxField_ = lumis_->GetModel().GetToken(kLumiAuxName);
  }
  void RNTupleOutputFile::setupEvents(SelectedProducts const& iProducts, Config const& iConfig) {
    std::string kEventAuxName = "EventAuxiliary";
    std::string kEventProvName = "EventProductProvenance";
    std::string kEventSelName = "EventSelections";
    std::string kBranchListName = "BranchListIndexes";
    {
      auto model = ROOT::Experimental::RNTupleModel::CreateBare();
      {
        auto field = ROOT::Experimental::RFieldBase::Create(kEventAuxName, "edm::EventAuxiliary").Unwrap();
        model->AddField(std::move(field));
      }
      {
        auto field =
            ROOT::Experimental::RFieldBase::Create(kEventProvName, "std::vector<edm::StoredProductProvenance>").Unwrap();
        model->AddField(std::move(field));
      }
      {
        auto field = ROOT::Experimental::RFieldBase::Create(kEventSelName, "std::vector<edm::Hash<1> >").Unwrap();
        model->AddField(std::move(field));
      }
      {
        auto field = ROOT::Experimental::RFieldBase::Create(kBranchListName, "std::vector<unsigned short>").Unwrap();
        model->AddField(std::move(field));
      }
      setupDataProducts(iProducts, iConfig.doNotSplitProduct, *model);

      auto writeOptions = ROOT::Experimental::RNTupleWriteOptions();
      writeOptions.SetCompression(convert(iConfig.compressionAlgo), iConfig.compressionLevel);
      writeOptions.SetUseTailPageOptimization(iConfig.useTailPageOptimization);
      events_ = ROOT::Experimental::RNTupleWriter::Append(std::move(model), "Events", file_, writeOptions);
    }
    products_[InEvent] = associateDataProducts(iProducts, events_->GetModel());

    eventAuxField_ = events_->GetModel().GetToken(kEventAuxName);
    eventProvField_ = events_->GetModel().GetToken(kEventProvName);
    eventSelField_ = events_->GetModel().GetToken(kEventSelName);
    branchListField_ = events_->GetModel().GetToken(kBranchListName);

    // Note: The EventSelectionIDVector should have a one to one correspondence with the processes in the process history.
    // Therefore, a new entry should be added if and only if the current process has been added to the process history,
    // which is done if and only if there is a produced product.
    Service<ConstProductRegistry> reg;
    extendSelectorConfig_ = reg->anyProductProduced() || !iConfig.wantAllEvents;
  }
  void RNTupleOutputFile::setupPSets(Config const& iConfig) {
    auto model = ROOT::Experimental::RNTupleModel::CreateBare();
    {
      auto field = ROOT::Experimental::RFieldBase::Create("IdToParameterSetsBlobs",
                                                          "std::pair<edm::Hash<1>,edm::ParameterSetBlob>")
                       .Unwrap();
      model->AddField(std::move(field));
    }
    auto writeOptions = ROOT::Experimental::RNTupleWriteOptions();
    writeOptions.SetCompression(convert(iConfig.compressionAlgo), iConfig.compressionLevel);
    writeOptions.SetUseTailPageOptimization(iConfig.useTailPageOptimization);
    parameterSets_ = ROOT::Experimental::RNTupleWriter::Append(std::move(model), "ParameterSets", file_, writeOptions);
  }

  void RNTupleOutputFile::fillPSets() {
    std::pair<ParameterSetID, ParameterSetBlob> idToBlob;

    auto rentry = parameterSets_->CreateEntry();
    rentry->BindRawPtr("IdToParameterSetsBlobs", const_cast<void*>(static_cast<void const*>(&(idToBlob))));

    for (auto const& pset : *pset::Registry::instance()) {
      idToBlob.first = pset.first;
      idToBlob.second.pset() = pset.second.toString();

      parameterSets_->Fill(*rentry);
    }
  }

  void RNTupleOutputFile::setupParentage(Config const& iConfig) {
    auto model = ROOT::Experimental::RNTupleModel::CreateBare();
    {
      auto field = ROOT::Experimental::RFieldBase::Create("Description", "edm::Parentage").Unwrap();
      model->AddField(std::move(field));
    }
    auto writeOptions = ROOT::Experimental::RNTupleWriteOptions();
    writeOptions.SetCompression(convert(iConfig.compressionAlgo), iConfig.compressionLevel);
    writeOptions.SetUseTailPageOptimization(iConfig.useTailPageOptimization);
    parentage_ = ROOT::Experimental::RNTupleWriter::Append(std::move(model), "Parentage", file_, writeOptions);
  }
  void RNTupleOutputFile::fillParentage() {
    ParentageRegistry& ptReg = *ParentageRegistry::instance();

    std::vector<ParentageID> orderedIDs(parentageIDs_.size());
    for (auto const& parentageID : parentageIDs_) {
      orderedIDs[parentageID.second] = parentageID.first;
    }

    auto rentry = parentage_->CreateEntry();
    //now put them into the TTree in the correct order
    for (auto const& orderedID : orderedIDs) {
      auto desc = ptReg.getMapped(orderedID);
      rentry->BindRawPtr("Description", const_cast<void*>(static_cast<void const*>(desc)));
      parentage_->Fill(*rentry);
    }
  }

  void RNTupleOutputFile::setupMetaData(Config const& iConfig) {
    auto model = ROOT::Experimental::RNTupleModel::CreateBare();
    {
      //Ultimately will need a new class specific for RNTuple
      //auto field = ROOT::Experimental::RFieldBase::Create("FileFormatVersion", "edm::FileFormatVersion").Unwrap();
      //model->AddField(std::move(field));
    }
    {
      auto field = ROOT::Experimental::RFieldBase::Create("FileIdentifier", "edm::FileID").Unwrap();
      model->AddField(std::move(field));
    }

    {
      auto field = ROOT::Experimental::RFieldBase::Create("IndexIntoFile", "edm::IndexIntoFile").Unwrap();
      model->AddField(std::move(field));
    }
    {
      auto field = ROOT::Experimental::RFieldBase::Create("MergeableRunProductMetadata",
                                                          "edm::StoredMergeableRunProductMetadata")
                       .Unwrap();
      model->AddField(std::move(field));
    }
    {
      auto field =
          ROOT::Experimental::RFieldBase::Create("ProcessHistory", "std::vector<edm::ProcessHistory>").Unwrap();
      model->AddField(std::move(field));
    }
    {
      auto field = ROOT::Experimental::RFieldBase::Create("ProductRegistry", "edm::ProductRegistry").Unwrap();
      model->AddField(std::move(field));
    }
    {
      auto field =
          ROOT::Experimental::RFieldBase::Create("BranchIDLists", "std::vector<std::vector<unsigned int> >").Unwrap();
      model->AddField(std::move(field));
    }
    {
      auto field =
          ROOT::Experimental::RFieldBase::Create("ThinnedAssociationsHelper", "edm::ThinnedAssociationsHelper").Unwrap();
      model->AddField(std::move(field));
    }
    {
      auto field = ROOT::Experimental::RFieldBase::Create("ProductDependencies", "edm::BranchChildren").Unwrap();
      model->AddField(std::move(field));
    }

    auto writeOptions = ROOT::Experimental::RNTupleWriteOptions();
    writeOptions.SetCompression(convert(iConfig.compressionAlgo), iConfig.compressionLevel);
    writeOptions.SetUseTailPageOptimization(iConfig.useTailPageOptimization);
    metaData_ = ROOT::Experimental::RNTupleWriter::Append(std::move(model), "MetaData", file_, writeOptions);
  }

  void RNTupleOutputFile::fillMetaData(BranchIDLists const& iBranchIDLists,
                                       ThinnedAssociationsHelper const& iThinnedHelper) {
    auto rentry = metaData_->CreateEntry();

    FileID id(createGlobalIdentifier());

    rentry->BindRawPtr("FileIdentifier", &id);

    indexIntoFile_.sortVector_Run_Or_Lumi_Entries();
    rentry->BindRawPtr("IndexIntoFile", &indexIntoFile_);

    ProcessHistoryVector procHistoryVector;
    for (auto const& ph : processHistoryRegistry_) {
      procHistoryVector.push_back(ph.second);
    }
    rentry->BindRawPtr("ProcessHistory", &procHistoryVector);

    // Make a local copy of the ProductRegistry, removing any transient or pruned products.
    using ProductList = ProductRegistry::ProductList;
    Service<ConstProductRegistry> reg;
    ProductRegistry pReg(reg->productList());
    ProductList& pList = const_cast<ProductList&>(pReg.productList());
    for (auto const& prod : pList) {
      if (prod.second.branchID() != prod.second.originalBranchID()) {
        if (branchesWithStoredHistory_.find(prod.second.branchID()) != branchesWithStoredHistory_.end()) {
          branchesWithStoredHistory_.insert(prod.second.originalBranchID());
        }
      }
    }
    std::set<BranchID>::iterator end = branchesWithStoredHistory_.end();
    for (ProductList::iterator it = pList.begin(); it != pList.end();) {
      if (branchesWithStoredHistory_.find(it->second.branchID()) == end) {
        // avoid invalidating iterator on deletion
        ProductList::iterator itCopy = it;
        ++it;
        pList.erase(itCopy);

      } else {
        ++it;
      }
    }

    rentry->BindRawPtr("ProductRegistry", &pReg);
    rentry->BindRawPtr("ThinnedAssociationsHelper", const_cast<void*>(static_cast<const void*>(&iThinnedHelper)));
    rentry->BindRawPtr("BranchIDLists", const_cast<void*>(static_cast<const void*>(&iBranchIDLists)));
    rentry->BindRawPtr("ProductDependencies", const_cast<void*>(static_cast<const void*>(&branchChildren_)));

    metaData_->Fill(*rentry);
  }

  void RNTupleOutputFile::openFile(FileBlock const& fb) {
    auto const& branchToChildMap = fb.branchChildren().childLookup();
    for (auto const& parentToChildren : branchToChildMap) {
      for (auto const& child : parentToChildren.second) {
        branchChildren_.insertChild(parentToChildren.first, child);
      }
    }
  }

  void RNTupleOutputFile::reallyCloseFile(BranchIDLists const& iBranchIDLists,
                                          ThinnedAssociationsHelper const& iThinnedHelper) {
    fillPSets();
    fillParentage();
    fillMetaData(iBranchIDLists, iThinnedHelper);
  }

  RNTupleOutputFile::~RNTupleOutputFile() {}

  std::vector<std::unique_ptr<WrapperBase>> RNTupleOutputFile::writeDataProducts(std::vector<Product> const& iProducts,
                                                                                 OccurrenceForOutput const& iOccurence,
                                                                                 REntry& iEntry) {
    std::vector<std::unique_ptr<WrapperBase>> dummies;

    for (auto const& p : iProducts) {
      auto h = iOccurence.getByToken(p.get_, p.desc_->unwrappedTypeID());
      auto product = h.wrapper();
      if (nullptr == product) {
        // No product with this ID is in the event.
        // Add a null product.
        TClass* cp = p.desc_->wrappedType().getClass();
        assert(cp != nullptr);
        int offset = cp->GetBaseClassOffset(wrapperBaseTClass_);
        void* p = cp->New();
        std::unique_ptr<WrapperBase> dummy = getWrapperBasePtr(p, offset);
        product = dummy.get();
        dummies.emplace_back(std::move(dummy));
      }
      iEntry.BindRawPtr(p.field_, const_cast<void*>(static_cast<void const*>(product)));
    }
    return dummies;
  }

  std::vector<StoredProductProvenance> RNTupleOutputFile::writeDataProductProvenance(
      std::vector<Product> const& iProducts, EventForOutput const& iEvent) {
    std::set<StoredProductProvenance> provenanceToKeep;

    if (not dropMetaData_) {
      for (auto const& p : iProducts) {
        auto h = iEvent.getByToken(p.get_, p.desc_->unwrappedTypeID());
        if (h.isValid()) {
          auto prov = h.provenance()->productProvenance();
          if (not prov) {
            prov = iEvent.productProvenanceRetrieverPtr()->branchIDToProvenance(p.desc_->originalBranchID());
          }
          if (prov) {
            insertProductProvenance(*prov, provenanceToKeep);
          }
        }
      }
    }
    return std::vector<StoredProductProvenance>(provenanceToKeep.begin(), provenanceToKeep.end());
  }

  bool RNTupleOutputFile::insertProductProvenance(ProductProvenance const& iProv,
                                                  std::set<StoredProductProvenance>& oToInsert) {
    StoredProductProvenance toStore;
    toStore.branchID_ = iProv.branchID().id();
    auto itFound = oToInsert.find(toStore);
    if (itFound == oToInsert.end()) {
      //get the index to the ParentageID or insert a new value if not already present
      auto i = parentageIDs_.emplace(iProv.parentageID(), static_cast<unsigned int>(parentageIDs_.size()));
      toStore.parentageIDIndex_ = i.first->second;
      if (toStore.parentageIDIndex_ >= parentageIDs_.size()) {
        throw edm::Exception(errors::LogicError)
            << "RootOutputFile::insertProductProvenance\n"
            << "The parentage ID index value " << toStore.parentageIDIndex_
            << " is out of bounds.  The maximum value is currently " << parentageIDs_.size() - 1 << ".\n"
            << "This should never happen.\n"
            << "Please report this to the framework developers.";
      }

      oToInsert.insert(toStore);
      return true;
    }
    return false;
  }

  void RNTupleOutputFile::insertAncestorsProvenance(ProductProvenance const& iProv,
                                                    ProductProvenanceRetriever const& iMapper,
                                                    std::set<StoredProductProvenance>& oToKeep) {
    std::vector<BranchID> const& parentIDs = iProv.parentage().parents();
    for (auto const& parentID : parentIDs) {
      branchesWithStoredHistory_.insert(parentID);
      ProductProvenance const* info = iMapper.branchIDToProvenance(parentID);
      if (info) {
        if (insertProductProvenance(*info, oToKeep)) {
          //haven't seen this one yet
          insertAncestorsProvenance(*info, iMapper, oToKeep);
        }
      }
    }
  }

  void RNTupleOutputFile::write(EventForOutput const& e) {
    {
      auto rentry = events_->CreateEntry();
      rentry->BindRawPtr(*eventAuxField_, const_cast<void*>(static_cast<void const*>(&(e.eventAuxiliary()))));
      rentry->BindRawPtr(*eventSelField_, const_cast<void*>(static_cast<void const*>(&(e.eventSelectionIDs()))));
      rentry->BindRawPtr(*branchListField_, const_cast<void*>(static_cast<void const*>(&(e.branchListIndexes()))));

      EventSelectionIDVector esids = e.eventSelectionIDs();
      if (extendSelectorConfig_) {
        esids.push_back(selectorConfig_);
      }
      rentry->BindRawPtr(*eventSelField_, &esids);

      auto dummies = writeDataProducts(products_[InEvent], e, *rentry);
      auto prov = writeDataProductProvenance(products_[InEvent], e);
      rentry->BindRawPtr(*eventProvField_, &prov);
      events_->Fill(*rentry);
    }

    processHistoryRegistry_.registerProcessHistory(e.processHistory());
    // Store the reduced ID in the IndexIntoFile
    ProcessHistoryID reducedPHID = processHistoryRegistry_.reducedProcessHistoryID(e.processHistoryID());
    // Add event to index
    indexIntoFile_.addEntry(reducedPHID, e.run(), e.luminosityBlock(), e.event(), eventEntryNumber_);
    ++eventEntryNumber_;
  }

  void RNTupleOutputFile::writeLuminosityBlock(LuminosityBlockForOutput const& iLumi) {
    {
      auto rentry = lumis_->CreateEntry();
      rentry->BindRawPtr(*lumiAuxField_,
                         const_cast<void*>(static_cast<void const*>(&(iLumi.luminosityBlockAuxiliary()))));
      auto dummies = writeDataProducts(products_[InLumi], iLumi, *rentry);
      lumis_->Fill(*rentry);
    }
    processHistoryRegistry_.registerProcessHistory(iLumi.processHistory());
    // Store the reduced ID in the IndexIntoFile
    ProcessHistoryID reducedPHID = processHistoryRegistry_.reducedProcessHistoryID(iLumi.processHistoryID());
    // Add lumi to index.
    indexIntoFile_.addEntry(reducedPHID, iLumi.run(), iLumi.luminosityBlock(), 0U, lumiEntryNumber_);
    ++lumiEntryNumber_;
  }

  void RNTupleOutputFile::writeRun(RunForOutput const& iRun) {
    {
      auto rentry = runs_->CreateEntry();
      rentry->BindRawPtr(*runAuxField_, const_cast<void*>(static_cast<void const*>(&(iRun.runAuxiliary()))));
      auto dummies = writeDataProducts(products_[InRun], iRun, *rentry);
      runs_->Fill(*rentry);
    }
    processHistoryRegistry_.registerProcessHistory(iRun.processHistory());
    // Store the reduced ID in the IndexIntoFile
    ProcessHistoryID reducedPHID = processHistoryRegistry_.reducedProcessHistoryID(iRun.processHistoryID());
    // Add run to index.
    indexIntoFile_.addEntry(reducedPHID, iRun.run(), 0U, 0U, runEntryNumber_);
    ++runEntryNumber_;
  }

}  // namespace edm
