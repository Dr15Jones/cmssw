#ifndef SimG4Core_CustomPhysics_CMSSIMPInelasticProcess_H
#define SimG4Core_CustomPhysics_CMSSIMPInelasticProcess_H

#include "G4HadronicProcess.hh"

class G4ParticleDefinition;

class CMSSIMPInelasticProcess : public G4HadronicProcess {
public:
  CMSSIMPInelasticProcess(const G4String& processName = "SIMPInelastic");

  ~CMSSIMPInelasticProcess() override = default;

  G4bool IsApplicable(const G4ParticleDefinition& aParticleType) override;

  // generic PostStepDoIt recommended for all derived classes
  G4VParticleChange* PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) override;

  CMSSIMPInelasticProcess& operator=(const CMSSIMPInelasticProcess& right) = delete;
  CMSSIMPInelasticProcess(const CMSSIMPInelasticProcess&) = delete;

private:
  G4ParticleDefinition* theParticle;
};

#endif
