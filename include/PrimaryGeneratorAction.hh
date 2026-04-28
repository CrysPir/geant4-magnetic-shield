#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "CosmicRaySampler.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"

class G4Event;
class G4ParticleGun;
class G4GenericMessenger;
class EventAction;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
  explicit PrimaryGeneratorAction(EventAction* eventAction);
  ~PrimaryGeneratorAction() override;

  void GeneratePrimaries(G4Event* event) override;

private:
  void DefineCommands();

  G4ParticleGun* fParticleGun = nullptr;
  EventAction* fEventAction = nullptr;
  CosmicRaySampler fSampler;
  G4GenericMessenger* fMessenger = nullptr;
  G4bool fIsotropicAngular = true;
  /// Default false: source on sphere for isotropic 4pi irradiation.
  G4bool fUseSkyDisk = false;
};

#endif
