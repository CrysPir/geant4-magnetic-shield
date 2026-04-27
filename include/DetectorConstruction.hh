#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4LogicalVolume;
class G4VPhysicalVolume;

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
  DetectorConstruction();
  ~DetectorConstruction() override;

  G4VPhysicalVolume* Construct() override;
  void ConstructSDandField() override;

  G4LogicalVolume* GetHabitatVolume() const { return fHabitatVolume; }

private:
  void DefineMaterials();

  G4LogicalVolume* fHabitatVolume = nullptr;
};

#endif
