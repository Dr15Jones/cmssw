#include <memory>

#include "SimG4Core/CustomPhysics/interface/CustomPhysicsListSS.h"
#include "SimG4Core/CustomPhysics/interface/CustomParticleFactory.h"
#include "SimG4Core/CustomPhysics/interface/CustomParticle.h"
#include "SimG4Core/CustomPhysics/interface/DummyChargeFlipProcess.h"
#include "SimG4Core/CustomPhysics/interface/CustomProcessHelper.h"
#include "SimG4Core/CustomPhysics/interface/CustomPDGParser.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/FileInPath.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "SimG4Core/CustomPhysics/interface/FullModelHadronicProcess.h"
#include "SimG4Core/CustomPhysics/interface/CMSDarkPairProductionProcess.h"

#include "G4hMultipleScattering.hh"
#include "G4hIonisation.hh"
#include "G4CoulombScattering.hh"
#include "G4ProcessManager.hh"
#include "G4AutoDelete.hh"

using namespace CLHEP;

G4ThreadLocal CustomProcessHelper* CustomPhysicsListSS::myHelper = nullptr;

CustomPhysicsListSS::CustomPhysicsListSS(const std::string& name, const edm::ParameterSet& p, bool apinew)
    : G4VPhysicsConstructor(name) {
  myConfig = p;
  if (apinew) {
    dfactor = p.getParameter<double>("DarkMPFactor");
    fHadronicInteraction = p.getParameter<bool>("RhadronPhysics");
  } else {
    // this is left for backward compatibility
    dfactor = p.getParameter<double>("dark_factor");
    fHadronicInteraction = p.getParameter<bool>("rhadronPhysics");
  }
  edm::FileInPath fp = p.getParameter<edm::FileInPath>("particlesDef");
  particleDefFilePath = fp.fullPath();
  fParticleFactory = std::make_unique<CustomParticleFactory>();

  edm::LogVerbatim("SimG4CoreCustomPhysics") << "CustomPhysicsListSS: Path for custom particle definition file: \n"
                                             << particleDefFilePath;
}

void CustomPhysicsListSS::ConstructParticle() {
  edm::LogVerbatim("SimG4CoreCustomPhysicsSS") << "===== CustomPhysicsList::ConstructParticle ";
  fParticleFactory.get()->loadCustomParticles(particleDefFilePath);
}

void CustomPhysicsListSS::ConstructProcess() {
  edm::LogVerbatim("SimG4CoreCustomPhysicsSS") << "CustomPhysicsListSS: adding CustomPhysics processes";

  G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();

  for (auto particle : fParticleFactory.get()->getCustomParticles()) {
    CustomParticle* cp = dynamic_cast<CustomParticle*>(particle);
    if (cp) {
      G4ProcessManager* pmanager = particle->GetProcessManager();
      edm::LogVerbatim("SimG4CoreCustomPhysics")
          << "CustomPhysicsListSS: " << particle->GetParticleName() << " PDGcode= " << particle->GetPDGEncoding()
          << " Mass= " << particle->GetPDGMass() / GeV << " GeV.";
      if (pmanager) {
        if (particle->GetPDGCharge() != 0.0) {
          ph->RegisterProcess(new G4CoulombScattering, particle);
          ph->RegisterProcess(new G4hIonisation, particle);
        }
        if (cp->GetCloud() && fHadronicInteraction &&
            (CustomPDGParser::s_isgluinoHadron(particle->GetPDGEncoding()) ||
             (CustomPDGParser::s_isstopHadron(particle->GetPDGEncoding())) ||
             (CustomPDGParser::s_issbottomHadron(particle->GetPDGEncoding())))) {
          edm::LogVerbatim("SimG4CoreCustomPhysics")
              << "CustomPhysicsListSS: " << particle->GetParticleName()
              << " CloudMass= " << cp->GetCloud()->GetPDGMass() / GeV
              << " GeV; SpectatorMass= " << cp->GetSpectator()->GetPDGMass() / GeV << " GeV.";

          if (nullptr == myHelper) {
            myHelper = new CustomProcessHelper(myConfig, fParticleFactory.get());
            G4AutoDelete::Register(myHelper);
          }
          pmanager->AddDiscreteProcess(new FullModelHadronicProcess(myHelper));
        }
        if (particle->GetParticleType() == "darkpho") {
          CMSDarkPairProductionProcess* darkGamma = new CMSDarkPairProductionProcess(dfactor);
          pmanager->AddDiscreteProcess(darkGamma);
        }
      }
    }
  }
}
