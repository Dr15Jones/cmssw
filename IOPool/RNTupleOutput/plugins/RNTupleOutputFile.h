#if !defined(IOPool_RNTuptleOutput_RNTupleOutputFile_h)
#define IOPool_RNTuptleOutput_RNTupleOutputFile_h

#include "FWCore/Framework/interface/EventForOutput.h"
#include "FWCore/Framework/interface/FileBlock.h"

#include "FWCore/ParameterSet/interface/Registry.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/ConstProductRegistry.h"
#include "FWCore/Utilities/interface/GlobalIdentifier.h"

#include "DataFormats/Provenance/interface/ParentageRegistry.h"
#include "DataFormats/Provenance/interface/FileID.h"
#include "DataFormats/Provenance/interface/ProcessHistoryRegistry.h"
#include "DataFormats/Provenance/interface/Provenance.h"
#include "DataFormats/Provenance/interface/IndexIntoFile.h"
#include "DataFormats/Provenance/interface/BranchChildren.h"
#include "DataFormats/Provenance/interface/ThinnedAssociationsHelper.h"
#include "DataFormats/Provenance/interface/SelectedProducts.h"
#include "DataFormats/Provenance/interface/StoredProductProvenance.h"

#include "TFile.h"
#include "ROOT/RNTuple.hxx"
#include "ROOT/RNTupleWriter.hxx"
#include <string>
#include <optional>
#include <map>
#include <set>
#include <array>

using namespace ROOT::Experimental;
namespace edm {
  namespace rntuple {
    enum class CompressionAlgos { kLZMA, kZSTD, kZLIB, kLZ4 };
  }

  class RNTupleOutputFile {
  public:
    struct Config {
      ParameterSetID selectorConfig;
      std::vector<bool> doNotSplitProduct;
      rntuple::CompressionAlgos compressionAlgo = rntuple::CompressionAlgos::kZSTD;
      int compressionLevel = 4;
      bool wantAllEvents;
      bool dropMetaData = false;
      bool useTailPageOptimization = false;
    };

    explicit RNTupleOutputFile(std::string const& iFileName,
                               FileBlock const& iFileBlock,
                               SelectedProductsForBranchType const& iSelected,
                               Config const&);
    ~RNTupleOutputFile();

    void write(EventForOutput const& e);
    void writeLuminosityBlock(LuminosityBlockForOutput const&);
    void writeRun(RunForOutput const&);
    void reallyCloseFile(BranchIDLists const& iBranchIDLists, ThinnedAssociationsHelper const& iThinnedHelper);
    void openFile(FileBlock const& fb);

    struct Product {
      Product(EDGetToken iGet, BranchDescription const* iDesc, REntry::RFieldToken iField)
          : get_(std::move(iGet)), desc_(iDesc), field_(std::move(iField)) {}

      EDGetToken get_;
      BranchDescription const* desc_;
      REntry::RFieldToken field_;
    };

  private:
    void setupRuns(SelectedProducts const&, Config const&);
    void setupLumis(SelectedProducts const&, Config const&);
    void setupEvents(SelectedProducts const&, Config const&);
    void setupPSets(Config const&);
    void setupParentage(Config const&);
    void setupMetaData(Config const&);

    void fillPSets();
    void fillParentage();
    void fillMetaData(BranchIDLists const& iBranchIDLists, ThinnedAssociationsHelper const& iThinnedHelper);

    void setupDataProducts(SelectedProducts const&, std::vector<bool> const&, RNTupleModel&);
    //Can't call until the model is frozen
    std::vector<Product> associateDataProducts(SelectedProducts const&, RNTupleModel const&);

    std::vector<std::unique_ptr<edm::WrapperBase>> writeDataProducts(std::vector<Product> const& iProduct,
                                                                     OccurrenceForOutput const& iOccurence,
                                                                     REntry&);
    std::vector<StoredProductProvenance> writeDataProductProvenance(std::vector<Product> const& iProduct,
                                                                    EventForOutput const& iEvent);
    bool insertProductProvenance(ProductProvenance const& iProv, std::set<StoredProductProvenance>& oToKeep);
    void insertAncestorsProvenance(ProductProvenance const& iProv,
                                   ProductProvenanceRetriever const&,
                                   std::set<StoredProductProvenance>& oToKeep);
    TFile file_;
    std::unique_ptr<RNTupleWriter> events_;
    std::optional<REntry::RFieldToken> eventAuxField_;
    std::optional<REntry::RFieldToken> eventProvField_;
    std::optional<REntry::RFieldToken> eventSelField_;
    std::optional<REntry::RFieldToken> branchListField_;

    std::unique_ptr<RNTupleWriter> runs_;
    std::optional<REntry::RFieldToken> runAuxField_;

    std::unique_ptr<RNTupleWriter> lumis_;
    std::optional<REntry::RFieldToken> lumiAuxField_;

    std::unique_ptr<RNTupleWriter> parameterSets_;
    std::unique_ptr<RNTupleWriter> parentage_;
    std::unique_ptr<RNTupleWriter> metaData_;

    std::map<ParentageID, unsigned int> parentageIDs_;
    ProcessHistoryRegistry processHistoryRegistry_;
    std::set<BranchID> branchesWithStoredHistory_;

    IndexIntoFile::EntryNumber_t eventEntryNumber_ = 0LL;
    IndexIntoFile::EntryNumber_t lumiEntryNumber_ = 0LL;
    IndexIntoFile::EntryNumber_t runEntryNumber_ = 0LL;
    IndexIntoFile indexIntoFile_;
    BranchChildren branchChildren_;

    std::array<std::vector<Product>, NumBranchTypes> products_;
    TClass const* wrapperBaseTClass_;

    ParameterSetID selectorConfig_;
    bool extendSelectorConfig_ = true;
    bool dropMetaData_ = false;
  };
}  // namespace edm
#endif
