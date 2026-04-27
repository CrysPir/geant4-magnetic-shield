#include "CosmicRaySampler.hh"

#include "G4IonTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4PhysicalConstants.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include <cmath>

namespace
{
constexpr G4double kGcrFraction = 0.7;
constexpr G4double kSepFraction = 0.3;

constexpr G4double kGcrProton = 0.90;
constexpr G4double kGcrAlpha = 0.09;
constexpr G4double kGcrFe = 0.01;

constexpr G4double kSepProton = 0.898;
constexpr G4double kSepAlpha = 0.08;
constexpr G4double kSepElectron = 0.02;
constexpr G4double kSepGamma = 0.002;

constexpr G4double kGcrP = 2.7;
constexpr G4double kGcrEmin = 0.1 * GeV;
constexpr G4double kGcrEmax = 1.0e5 * GeV;

constexpr G4double kSepP = 2.0;
constexpr G4double kSepProtEmin = 1.0 * MeV;
constexpr G4double kSepProtEmax = 1000.0 * MeV;
constexpr G4double kSepLightEmin = 0.1 * MeV;
constexpr G4double kSepLightEmax = 10.0 * MeV;
}  // namespace

CosmicRaySampler::CosmicRaySampler() = default;

G4double CosmicRaySampler::SamplePowerLawE(G4double p, G4double eMin, G4double eMax) const
{
  if (eMax <= eMin * (1.0 + 1.0e-15)) {
    return eMin;
  }
  const G4double u = G4UniformRand();
  if (std::abs(p - 1.0) < 1.0e-9) {
    return std::exp(std::log(eMin) + u * (std::log(eMax) - std::log(eMin)));
  }
  const G4double exp = 1.0 - p;
  const G4double tMin = std::pow(eMin, exp);
  const G4double tMax = std::pow(eMax, exp);
  return std::pow(tMin + u * (tMax - tMin), 1.0 / exp);
}

void CosmicRaySampler::SampleDirectionImpl(G4ThreeVector& dir) const
{
  if (fAngularMode == AngularMode::kIsotropic) {
    const G4double cosTheta = 2.0 * G4UniformRand() - 1.0;
    const G4double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));
    const G4double phi = twopi * G4UniformRand();
    dir.set(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);
    dir = dir.unit();
    return;
  }

  const G4double cosTheta = G4UniformRand();
  const G4double sinTheta = std::sqrt(std::max(0.0, 1.0 - cosTheta * cosTheta));
  const G4double phi = twopi * G4UniformRand();
  G4ThreeVector local(sinTheta * std::cos(phi), sinTheta * std::sin(phi), -cosTheta);
  dir = local.unit();
}

void CosmicRaySampler::SampleSpeciesAndEnergy(G4ParticleDefinition*& definition,
                                              G4double& kineticEnergy) const
{
  auto* table = G4ParticleTable::GetParticleTable();
  auto* ionTable = G4IonTable::GetIonTable();

  const G4bool isGcr = (G4UniformRand() < kGcrFraction);
  const G4double w = G4UniformRand();

  if (isGcr) {
    if (w < kGcrProton) {
      definition = table->FindParticle("proton");
      kineticEnergy = SamplePowerLawE(kGcrP, kGcrEmin, kGcrEmax);
    } else if (w < kGcrProton + kGcrAlpha) {
      definition = table->FindParticle("alpha");
      const G4double ePerNucleon = SamplePowerLawE(kGcrP, kGcrEmin, kGcrEmax);
      kineticEnergy = ePerNucleon * 4.0;
    } else {
      definition = ionTable->GetIon(26, 56);
      const G4double ePerNucleon = SamplePowerLawE(kGcrP, kGcrEmin, kGcrEmax);
      kineticEnergy = ePerNucleon * 56.0;
    }
  } else {
    if (w < kSepProton) {
      definition = table->FindParticle("proton");
      kineticEnergy = SamplePowerLawE(kSepP, kSepProtEmin, kSepProtEmax);
    } else if (w < kSepProton + kSepAlpha) {
      definition = table->FindParticle("alpha");
      const G4double ePerNucleon = SamplePowerLawE(kSepP, kSepProtEmin, kSepProtEmax);
      kineticEnergy = ePerNucleon * 4.0;
    } else if (w < kSepProton + kSepAlpha + kSepElectron) {
      definition = table->FindParticle("e-");
      kineticEnergy = SamplePowerLawE(kSepP, kSepLightEmin, kSepLightEmax);
    } else {
      definition = table->FindParticle("gamma");
      kineticEnergy = SamplePowerLawE(kSepP, kSepLightEmin, kSepLightEmax);
    }
  }
}

void CosmicRaySampler::SampleAngularDirection(G4ThreeVector& directionUnit) const
{
  SampleDirectionImpl(directionUnit);
}

void CosmicRaySampler::SamplePrimary(G4ParticleDefinition*& definition,
                                     G4double& kineticEnergy,
                                     G4ThreeVector& directionUnit) const
{
  SampleSpeciesAndEnergy(definition, kineticEnergy);
  SampleAngularDirection(directionUnit);
}
