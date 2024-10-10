#include "FWCore/Framework/interface/one/OutputModule.h"
#include "FWCore/Framework/interface/EventForOutput.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ConstProductRegistry.h"
#include "FWCore/Framework/interface/FileBlock.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
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

#include "RNTupleOutputFile.h"

#include <string>
#include <map>
#include <regex>
#include "boost/algorithm/string.hpp"

using namespace ROOT::Experimental;

namespace {
  edm::rntuple::CompressionAlgos convertTo(std::string const& iName) {
    if (iName == "LZMA") {
      return edm::rntuple::CompressionAlgos::kLZMA;
    }
    if (iName == "ZSTD") {
      return edm::rntuple::CompressionAlgos::kZSTD;
    }
    if (iName == "LZ4") {
      return edm::rntuple::CompressionAlgos::kLZ4;
    }
    if (iName == "ZLIB") {
      return edm::rntuple::CompressionAlgos::kZLIB;
    }
    throw cms::Exception("UnknownCompression") << "An unknown compression algorithm was specified: " << iName;
  }

  struct SetSplitForDataProduct {
    SetSplitForDataProduct(std::string const& iName, bool iDoNotSplit)
        : branch_(convert(iName)), doNotSplit_(iDoNotSplit) {}
    bool match(std::string const& iName) const;
    std::regex convert(std::string const& iGlobBranchExpression) const;

    std::regex branch_;
    bool doNotSplit_;
  };

  inline bool SetSplitForDataProduct::match(std::string const& iBranchName) const {
    return std::regex_match(iBranchName, branch_);
  }

  std::regex SetSplitForDataProduct::convert(std::string const& iGlobBranchExpression) const {
    std::string tmp(iGlobBranchExpression);
    boost::replace_all(tmp, "*", ".*");
    boost::replace_all(tmp, "?", ".");
    return std::regex(tmp);
  }

  std::vector<SetSplitForDataProduct> fromConfig(std::vector<edm::ParameterSet> const& iConfig) {
    std::vector<SetSplitForDataProduct> returnValue;
    returnValue.reserve(iConfig.size());

    for (auto const& prod : iConfig) {
      returnValue.emplace_back(prod.getUntrackedParameter<std::string>("product"),
                               prod.getUntrackedParameter<bool>("turnOffSplitting"));
    }
    return returnValue;
  }

  std::optional<bool> turnOffSplitting(std::string const& iName, std::vector<SetSplitForDataProduct> const& iSpecial) {
    auto nameNoDot = iName.substr(0, iName.size() - 1);
    for (auto const& prod : iSpecial) {
      if (prod.match(nameNoDot)) {
        return prod.doNotSplit_;
      }
    }
    return {};
  }

}  // namespace

namespace edm {

  class RNTupleOutputModule : public one::OutputModule<> {
  public:
    explicit RNTupleOutputModule(ParameterSet const& pset);
    ~RNTupleOutputModule() final;
    static void fillDescriptions(ConfigurationDescriptions& descriptions);

  private:
    void write(EventForOutput const& e) final;
    void writeLuminosityBlock(LuminosityBlockForOutput const&) final;
    void writeRun(RunForOutput const&) final;
    void reallyCloseFile() final;
    void openFile(FileBlock const& fb) final;
    std::string fileName_;
    std::unique_ptr<RNTupleOutputFile> file_;
    std::vector<SetSplitForDataProduct> overrideSplitting_;
    rntuple::CompressionAlgos compressionAlgo_;
    unsigned int compressionLevel_;
    unsigned long long approxZippedClusterSize_;
    unsigned long long maxUnzippedClusterSize_;
    unsigned long long initialNElementsPerPage_;
    unsigned long long maxUnzippedPageSize_;
    unsigned long long pageBufferBudget_;
    bool useBufferedWrite_;
    bool useDirectIO_;
    bool dropMetaData_;
    bool turnOffSplitting_;
  };

  RNTupleOutputModule::RNTupleOutputModule(ParameterSet const& pset)
      : one::OutputModuleBase(pset),
        one::OutputModule<>(pset),
        fileName_(pset.getUntrackedParameter<std::string>("fileName")),
        overrideSplitting_(
            fromConfig(pset.getUntrackedParameter<std::vector<edm::ParameterSet>>("overrideDataProductSplitting"))),
        compressionAlgo_(convertTo(pset.getUntrackedParameter<std::string>("compressionAlgorithm"))),
        compressionLevel_(pset.getUntrackedParameter<unsigned int>("compressionLevel")),
        approxZippedClusterSize_(pset.getUntrackedParameter<unsigned long long>("approxZippedClusterSize")),
        maxUnzippedClusterSize_(pset.getUntrackedParameter<unsigned long long>("maxUnzippedClusterSize")),
        initialNElementsPerPage_(pset.getUntrackedParameter<unsigned long long>("initialNElementsPerPage")),
        maxUnzippedPageSize_(pset.getUntrackedParameter<unsigned long long>("maxUnzippedPageSize")),
        pageBufferBudget_(pset.getUntrackedParameter<unsigned long long>("pageBufferBudget")),
        useBufferedWrite_(pset.getUntrackedParameter<bool>("useBufferedWrite")),
        useDirectIO_(pset.getUntrackedParameter<bool>("useDirectIO")),
        dropMetaData_(pset.getUntrackedParameter<bool>("dropPerEventDataProductProvenance")),
        turnOffSplitting_(pset.getUntrackedParameter<bool>("turnOffSplitting")) {}

  void RNTupleOutputModule::openFile(FileBlock const& fb) {
    RNTupleOutputFile::Config conf;
    conf.wantAllEvents = wantAllEvents();
    conf.selectorConfig = selectorConfig();
    conf.compressionAlgo = compressionAlgo_;
    conf.compressionLevel = compressionLevel_;
    conf.approxZippedClusterSize = approxZippedClusterSize_;
    conf.maxUnzippedClusterSize = maxUnzippedClusterSize_;
    conf.initialNElementsPerPage = initialNElementsPerPage_;
    conf.maxUnzippedPageSize = maxUnzippedPageSize_;
    conf.pageBufferBudget = pageBufferBudget_;
    conf.useBufferedWrite = useBufferedWrite_;
    conf.useDirectIO = useDirectIO_;
    conf.dropMetaData = dropMetaData_;
    if (turnOffSplitting_ and overrideSplitting_.empty()) {
      auto const& prods = keptProducts()[InEvent];
      conf.doNotSplitProduct = std::vector<bool>(prods.size(), true);
    } else if (not overrideSplitting_.empty()) {
      auto const& prods = keptProducts()[InEvent];
      conf.doNotSplitProduct = std::vector<bool>(prods.size(), turnOffSplitting_);
      unsigned int index = 0;
      for (auto const& prod : prods) {
        auto choice = turnOffSplitting(prod.first->branchName(), overrideSplitting_);
        if (choice) {
          std::cout << "found match " << prod.first->branchName() << std::endl;
          if (*choice != turnOffSplitting_) {
            conf.doNotSplitProduct[index] = *choice;
          }
        }
        ++index;
      }
    }
    file_ = std::make_unique<RNTupleOutputFile>(fileName_, fb, keptProducts(), conf);
  }

  void RNTupleOutputModule::reallyCloseFile() {
    if (file_) {
      file_->reallyCloseFile(*branchIDLists(), *thinnedAssociationsHelper());
    }
  }

  RNTupleOutputModule::~RNTupleOutputModule() {}

  void RNTupleOutputModule::write(EventForOutput const& e) { file_->write(e); }

  void RNTupleOutputModule::writeLuminosityBlock(LuminosityBlockForOutput const& iLumi) {
    file_->writeLuminosityBlock(iLumi);
  }

  void RNTupleOutputModule::writeRun(RunForOutput const& iRun) { file_->writeRun(iRun); }

  void RNTupleOutputModule::fillDescriptions(ConfigurationDescriptions& descriptions) {
    ParameterSetDescription desc;
    desc.setComment("Outputs event information into an RNTuple container.");
    desc.addUntracked<std::string>("fileName")->setComment("RNTuple file to read");
    desc.addUntracked<std::string>("compressionAlgorithm", "ZSTD")
        ->setComment(
            "Algorithm used to compress data in the ROOT output file, allowed values are ZLIB, LZMA, LZ4, and ZSTD");
    desc.addUntracked<unsigned int>("compressionLevel", 4)->setComment("ROOT compression level of output file.");
    ROOT::Experimental::RNTupleWriteOptions ops;
    desc.addUntracked<unsigned long long>("approxZippedClusterSize", ops.GetApproxZippedClusterSize())->setComment(
          "Approximation of the target compressed cluster size"
                                                                                                                   );
    desc.addUntracked<unsigned long long>("maxUnzippedClusterSize", ops.GetMaxUnzippedClusterSize())->setComment(
          "Memory limit for committing a cluster. High compression leads to high IO buffer size."
);
    
    desc.addUntracked<unsigned long long>("initialNElementsPerPage", ops.GetInitialNElementsPerPage())->setComment(
"Initially, columns start with a page large enough to hold the given number of elements. The 'pageBufferBudget'  needs to be large enough to hold the initial pages of all columns."
);
    desc.addUntracked<unsigned long long>("maxUnzippedPageSize", ops.GetMaxUnzippedPageSize())->setComment(
          "Pages can grow only to the given limit (bytes)."
);
    desc.addUntracked<unsigned long long>("pageBufferBudget", ops.GetPageBufferBudget())->setComment(
         "The maximum size that the sum of all page buffers used for writing into a persistent sink are allowed to use."
         " If set to zero, RNTuple will auto-adjust the budget based on the value of 'approxZippedClusterSize'."
         " If set manually, the size needs to be large enough to hold all initial page buffers."
);
    
    desc.addUntracked<bool>("useBufferedWrite", ops.GetUseBufferedWrite())->setComment(
         "Turn on use of buffered writing. This buffers compressed pages in memory, reorders them to keep pages of the same column adjacent, and coalesces the writes when committing a cluster."
);
    desc.addUntracked<bool>("useDirectIO", ops.GetUseDirectIO())->setComment(
         "Set use of direct IO. this introduces alignment requirements that may vary between filesystems and platforms"
);

    
    desc.addUntracked<bool>("dropPerEventDataProductProvenance", false)
        ->setComment(
            "do not store which data products were consumed to create a given data product for a given event.");
    desc.addUntracked<bool>("turnOffSplitting", false)
        ->setComment("Do not split top level fields when storing data products");

    {
      ParameterSetDescription specialSplit;
      specialSplit.addUntracked<std::string>("product")->setComment(
          "Name of data product needing a special split setting. The name can contain wildcards '*' and '?'");
      specialSplit.addUntracked<bool>("turnOffSplitting", true)
          ->setComment("Explicitly set if should or should not split (default is not split)");
      desc.addVPSetUntracked("overrideDataProductSplitting", specialSplit, std::vector<ParameterSet>());
    }

    OutputModule::fillDescription(desc);
    descriptions.addDefault(desc);
  }
}  // namespace edm

using edm::RNTupleOutputModule;
DEFINE_FWK_MODULE(RNTupleOutputModule);
