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
    rntuple::CompressionAlgos compressionAlgo_;
    unsigned int compressionLevel_;
  };

  RNTupleOutputModule::RNTupleOutputModule(ParameterSet const& pset)
      : one::OutputModuleBase(pset),
        one::OutputModule<>(pset),
        fileName_(pset.getUntrackedParameter<std::string>("fileName")),
        compressionAlgo_(convertTo(pset.getUntrackedParameter<std::string>("compressionAlgorithm"))),
        compressionLevel_(pset.getUntrackedParameter<unsigned int>("compressionLevel")) {}

  void RNTupleOutputModule::openFile(FileBlock const& fb) {
    RNTupleOutputFile::Config conf;
    conf.wantAllEvents = wantAllEvents();
    conf.selectorConfig = selectorConfig();
    conf.compressionAlgo = compressionAlgo_;
    conf.compressionLevel = compressionLevel_;
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
    OutputModule::fillDescription(desc);
    descriptions.addDefault(desc);
  }
}  // namespace edm

using edm::RNTupleOutputModule;
DEFINE_FWK_MODULE(RNTupleOutputModule);
