// -*- C++ -*-
//
// Package:     PhysicsTools/MicroAODOutput
// Class  :     MicroAODOutputModule
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Christopher Jones
//         Created:  Mon, 07 Aug 2017 14:21:41 GMT
//

// system include files
#include <string>
#include "TFile.h"

// user include files
#include "FWCore/Framework/interface/OutputModule.h"
#include "FWCore/Framework/interface/one/OutputModule.h"
#include "FWCore/Framework/interface/RunForOutput.h"
#include "FWCore/Framework/interface/LuminosityBlockForOutput.h"
#include "FWCore/Framework/interface/EventForOutput.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/JobReport.h"
#include "FWCore/Utilities/interface/GlobalIdentifier.h"
#include "FWCore/Utilities/interface/Digest.h"

class MicroAODOutputModule : public edm::one::OutputModule<> {
public:
  MicroAODOutputModule(edm::ParameterSet const& pset);
  virtual ~MicroAODOutputModule();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual void write(edm::EventForOutput const& e) override;
  virtual void writeLuminosityBlock(edm::LuminosityBlockForOutput const&) override;
  virtual void writeRun(edm::RunForOutput const&) override;
  virtual bool isFileOpen() const override;
  virtual void openFile(edm::FileBlock const&) override;
  virtual void reallyCloseFile() override;

  std::string m_fileName;
  std::string m_logicalFileName;
  edm::JobReport::Token m_jrToken;
  std::unique_ptr<TFile> m_file;
};


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
MicroAODOutputModule::MicroAODOutputModule(edm::ParameterSet const& pset):
  edm::one::OutputModuleBase::OutputModuleBase(pset),
  edm::one::OutputModule<>(pset),
  m_fileName(pset.getUntrackedParameter<std::string>("fileName")),
  m_logicalFileName(pset.getUntrackedParameter<std::string>("logicalFileName"))
{
}

MicroAODOutputModule::~MicroAODOutputModule()
{
}

void 
MicroAODOutputModule::write(edm::EventForOutput const& iEvent) {
  //Get data from 'e' and write it to the file

  edm::Service<edm::JobReport> jr;
  jr->eventWrittenToFile(m_jrToken, iEvent.id().run(), iEvent.id().event());
}

void 
MicroAODOutputModule::writeLuminosityBlock(edm::LuminosityBlockForOutput const& iLumi) {
  edm::Service<edm::JobReport> jr;
  jr->reportLumiSection(m_jrToken, iLumi.id().run(), iLumi.id().value());

}

void 
MicroAODOutputModule::writeRun(edm::RunForOutput const& iRun) {
  edm::Service<edm::JobReport> jr;
  jr->reportRunNumber(m_jrToken, iRun.id().run());
}

bool 
MicroAODOutputModule::isFileOpen() const {
  return nullptr != m_file.get();
}

void 
MicroAODOutputModule::openFile(edm::FileBlock const&) {
  m_file = std::make_unique<TFile>(m_fileName.c_str(),"RECREATE");
  edm::Service<edm::JobReport> jr;
  cms::Digest branchHash;
  m_jrToken = jr->outputFileOpened(m_fileName,
                                   m_logicalFileName,
                                   std::string(),
                                   "MicroAODOutputModule",
                                   description().moduleLabel(),
                                   edm::createGlobalIdentifier(),
                                   std::string(),
                                   branchHash.digest().toString(),
                                   std::vector<std::string>()
                                   );

  /* Setup file structure here */
}
void 
MicroAODOutputModule::reallyCloseFile() {
  m_file->Write();
  m_file->Close();
  m_file.reset();
  edm::Service<edm::JobReport> jr;
  jr->outputFileClosed(m_jrToken);
}

void 
MicroAODOutputModule::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;

  desc.addUntracked<std::string>("fileName");
  desc.addUntracked<std::string>("logicalFileName","");

  //replace with whatever you want to get from the EDM by default
  const std::vector<std::string> keep = {"drop *", "keep *edmTable*_*_*_*"};
  edm::OutputModule::fillDescription(desc, keep);
  
  //Used by Workflow management for their own meta data
  edm::ParameterSetDescription dataSet;
  dataSet.setAllowAnything();
  desc.addUntracked<edm::ParameterSetDescription>("dataset", dataSet)
    ->setComment("PSet is only used by Data Operations and not by this module.");

  descriptions.addDefault(desc);

}

DEFINE_FWK_MODULE(MicroAODOutputModule);
