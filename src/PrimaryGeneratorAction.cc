#include "PrimaryGeneratorAction.hh"

#include "EventAction.hh"
#include "GeometryHelpers.hh"

#include "G4Event.hh"
#include "G4GenericMessenger.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"
#include "globals.hh"

#include <cmath>

namespace
{
constexpr G4double kHabitatRadius = 2.0 * m;
constexpr G4double kHabitatHalfZ = 5.0 * m;
constexpr G4double kSourceRadius = 10.5 * m;
/// Disk just inside world (+Z); flux toward -Z (into ship along -Z).
constexpr G4double kSkyDiskZ = 11.0 * m;
}  // namespace

PrimaryGeneratorAction::PrimaryGeneratorAction(EventAction* eventAction)
  : fEventAction(eventAction)
{
  fParticleGun = new G4ParticleGun(1);
  DefineCommands();
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fMessenger;
  delete fParticleGun;
}

void PrimaryGeneratorAction::DefineCommands()
{
  fMessenger = new G4GenericMessenger(this, "/cosmic/", "Cosmic ray primary settings");
  fMessenger->DeclareProperty("isotropic", fIsotropicAngular)
    .SetGuidance("If true, isotropic 4pi directions; if false, hemisphere toward -Z.");
  fMessenger->DeclareProperty("useSkyDisk", fUseSkyDisk)
    .SetGuidance("If true (default), emit from a disk above +Z so rays hit the habitat cylinder; "
                 "if false, emit from a sphere (very few rays intersect — aimed count ~0).");
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  fSampler.SetAngularMode(fIsotropicAngular ? CosmicRaySampler::AngularMode::kIsotropic
                                             : CosmicRaySampler::AngularMode::kHemisphereTowardMinusZ);

  G4ParticleDefinition* definition = nullptr;
  G4double kineticEnergy = 0.0;
  G4ThreeVector direction;
  G4ThreeVector pos;

  fSampler.SampleSpeciesAndEnergy(definition, kineticEnergy);
  fSampler.SampleAngularDirection(direction);

  if (fUseSkyDisk) {
    const G4double rr = kHabitatRadius * std::sqrt(G4UniformRand());
    const G4double ph = twopi * G4UniformRand();
    pos.set(rr * std::cos(ph), rr * std::sin(ph), kSkyDiskZ);
  } else {
    const G4double cosTheta = 2.0 * G4UniformRand() - 1.0;
    const G4double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));
    const G4double phi = twopi * G4UniformRand();
    pos.set(kSourceRadius * sinTheta * std::cos(phi),
            kSourceRadius * sinTheta * std::sin(phi),
            kSourceRadius * cosTheta);
  }

  const G4bool aimed =
    GeometryHelpers::RayIntersectsFiniteCylinder(pos, direction, kHabitatRadius, kHabitatHalfZ);
  const G4bool charged = definition && std::abs(definition->GetPDGCharge()) > 1.0e-9;

  if (fEventAction) {
    fEventAction->SetPrimaryAimed(aimed);
    fEventAction->SetPrimaryCharged(charged);
  }

  if (!definition) {
    definition = G4ParticleTable::GetParticleTable()->FindParticle("proton");
    kineticEnergy = 100.0 * MeV;
  }

  fParticleGun->SetParticleDefinition(definition);
  fParticleGun->SetParticleEnergy(kineticEnergy);
  fParticleGun->SetParticleMomentumDirection(direction);
  fParticleGun->SetParticlePosition(pos);
  fParticleGun->GeneratePrimaryVertex(event);
}
