//
// ********************************************************************
// Authors of this file: Dustin Stolp (dostolp@ucdavis.edu)
//                       Sushil S. Chauhan (schauhan@cern.ch)
//
// -----------------------------------------------------------------------------

#include "SimG4Core/CustomPhysics/interface/CMSDarkPairProductionProcess.h"
#include "G4PhysicalConstants.hh"
#include <CLHEP/Units/SystemOfUnits.h>
#include "G4BetheHeitlerModel.hh"
#include "G4PairProductionRelModel.hh"
#include "G4Electron.hh"

using namespace std;

CMSDarkPairProductionProcess::CMSDarkPairProductionProcess(G4double df, const G4String& processName, G4ProcessType type)
    : G4VEmProcess(processName, type), isInitialised(false), darkFactor(df) {
  SetMinKinEnergy(2.0 * electron_mass_c2);
  SetProcessSubType(fGammaConversion);
  SetStartFromNullFlag(true);
  SetBuildTableFlag(true);
  SetSecondaryParticle(G4Electron::Electron());
  SetLambdaBinning(220);
}

G4bool CMSDarkPairProductionProcess::IsApplicable(const G4ParticleDefinition& p) {
  G4int pdg = std::abs(p.GetPDGEncoding());
  return (pdg == 1023 || pdg == 1072000);
}

void CMSDarkPairProductionProcess::InitialiseProcess(const G4ParticleDefinition* p) {
  if (!isInitialised) {
    isInitialised = true;

    AddEmModel(0, new CMSDarkPairProduction(p, darkFactor));
  }
}

G4double CMSDarkPairProductionProcess::MinPrimaryEnergy(const G4ParticleDefinition* p, const G4Material*) {
  return std::max(2 * CLHEP::electron_mass_c2 - p->GetPDGMass(), 0.0);
}
