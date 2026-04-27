#ifndef CosmicRaySampler_h
#define CosmicRaySampler_h 1

#include "G4ThreeVector.hh"
#include "G4Types.hh"

class G4ParticleDefinition;

class CosmicRaySampler
{
public:
  enum class AngularMode { kIsotropic, kHemisphereTowardMinusZ };

  CosmicRaySampler();

  /// Sample one cosmic-ray primary: sets definition, kinetic energy (total), unit direction.
  void SamplePrimary(G4ParticleDefinition*& definition,
                     G4double& kineticEnergy,
                     G4ThreeVector& directionUnit) const;

  void SampleSpeciesAndEnergy(G4ParticleDefinition*& definition, G4double& kineticEnergy) const;
  void SampleAngularDirection(G4ThreeVector& directionUnit) const;

  void SetAngularMode(AngularMode mode) { fAngularMode = mode; }
  AngularMode GetAngularMode() const { return fAngularMode; }

private:
  G4double SamplePowerLawE(G4double p, G4double eMin, G4double eMax) const;
  void SampleDirectionImpl(G4ThreeVector& dir) const;

  AngularMode fAngularMode = AngularMode::kHemisphereTowardMinusZ;
};

#endif
