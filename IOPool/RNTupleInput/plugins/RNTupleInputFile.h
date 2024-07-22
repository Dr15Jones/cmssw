#ifndef IOPool_Input_RNTupleInputFile_h
#define IOPool_Input_RNTupleInputFile_h

#include "DataFormats/Provenance/interface/IndexIntoFile.h"
#include "DataFormats/Provenance/interface/ParentageID.h"
#include "DataProductsRNTuple.h"

#include "TFile.h"
#include "ROOT/RNTuple.hxx"
#include "ROOT/RNTupleReader.hxx"

#include <optional>
#include <memory>

namespace edm {
  class RunAuxiliary;
  class LuminosityBlockAuxiliary;
  class EventAuxiliary;
  class ProductRegistry;
  class ProcessHistoryRegistry;

  class RNTupleInputFile {
  public:
    RNTupleInputFile(std::string const& iFileName);

    IndexIntoFile::EntryType getNextItemType();

    std::shared_ptr<LuminosityBlockAuxiliary> readLuminosityBlockAuxiliary();
    IndexIntoFile::EntryNumber_t readLuminosityBlock();

    std::shared_ptr<EventAuxiliary> readEventAuxiliary();
    IndexIntoFile::EntryNumber_t readEvent();

    std::shared_ptr<RunAuxiliary> readRunAuxiliary();
    IndexIntoFile::EntryNumber_t readRun();

    void readMeta(ProductRegistry&, ProcessHistoryRegistry&, BranchIDLists& iBranchIDLists);
    std::vector<ParentageID> readParentage();

    input::DataProductsRNTuple* runProducts() { return &runs_; }
    input::DataProductsRNTuple* luminosityBlockProducts() { return &lumis_; }
    input::DataProductsRNTuple* eventProducts() { return &events_; }

  private:
    std::unique_ptr<TFile> file_;

    input::DataProductsRNTuple runs_;
    input::DataProductsRNTuple lumis_;
    input::DataProductsRNTuple events_;

    IndexIntoFile index_;
    std::optional<IndexIntoFile::IndexIntoFileItr> iter_;
    std::optional<IndexIntoFile::IndexIntoFileItr> iterEnd_;
  };
}  // namespace edm
#endif
