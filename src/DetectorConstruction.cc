#include "DetectorConstruction.hh"
#include "MagneticField.hh"

#include "G4Box.hh"
#include "G4ChordFinder.hh"
#include "G4ClassicalRK4.hh"
#include "G4Element.hh"
#include "G4FieldManager.hh"
#include "G4Isotope.hh"
#include "G4LogicalVolume.hh"
#include "G4Mag_UsualEqRhs.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4String.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4TransportationManager.hh"
#include "G4Torus.hh"
#include "G4Tubs.hh"

#include <string>

namespace
{
constexpr G4double kHabitatRadius = 2.0 * m;
constexpr G4double kHabitatHalfZ = 5.0 * m;
constexpr G4double kAlInnerR = 2.0 * m;
constexpr G4double kAlOuterR = 2.003 * m;
constexpr G4double kPolyInnerR = 2.003 * m;
constexpr G4double kPolyOuterR = 2.006 * m;
constexpr G4double kSolenoidInnerR = 2.5 * m;
constexpr G4double kSolenoidOuterR = 2.55 * m;
constexpr G4double kSolenoidHalfZ = 6.0 * m;
constexpr G4double kRingTorusR = 6.5 * m;
constexpr G4double kRingTubeR = 0.1 * m;
constexpr G4double kWorldHalf = 12.0 * m;
}  // namespace

DetectorConstruction::DetectorConstruction() = default;

DetectorConstruction::~DetectorConstruction() = default;

void DetectorConstruction::DefineMaterials()
{
  G4NistManager* nist = G4NistManager::Instance();

  nist->FindOrBuildMaterial("G4_AIR");
  nist->FindOrBuildMaterial("G4_Al");
  nist->FindOrBuildMaterial("G4_POLYETHYLENE");

  auto* isoB10 = new G4Isotope("B10", 5, 10, 10.012937 * g / mole);
  auto* isoB11 = new G4Isotope("B11", 5, 11, 11.009305 * g / mole);
  auto* elB = new G4Element("B_natural", "B", 2);
  elB->AddIsotope(isoB10, 20.0 * perCent);
  elB->AddIsotope(isoB11, 80.0 * perCent);

  G4Element* elMg = nist->FindOrBuildElement("Mg");

  auto* mgB2 = new G4Material("MgB2", 2.6 * g / cm3, 2);
  mgB2->AddElement(elMg, 1);
  mgB2->AddElement(elB, 2);
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  DefineMaterials();

  G4NistManager* nist = G4NistManager::Instance();
  G4Material* air = nist->FindOrBuildMaterial("G4_AIR");
  G4Material* aluminum = nist->FindOrBuildMaterial("G4_Al");
  G4Material* polyethylene = nist->FindOrBuildMaterial("G4_POLYETHYLENE");
  G4Material* mgB2 = nist->FindOrBuildMaterial("MgB2");

  G4Box* worldSolid = new G4Box("World", kWorldHalf, kWorldHalf, kWorldHalf);
  G4LogicalVolume* worldLogical = new G4LogicalVolume(worldSolid, air, "World");
  G4VPhysicalVolume* worldPhysical = new G4PVPlacement(
    nullptr, G4ThreeVector(), worldLogical, "World", nullptr, false, 0, true);

  G4Tubs* habitatSolid =
    new G4Tubs("Habitat", 0.0, kHabitatRadius, kHabitatHalfZ, 0.0, 360.0 * deg);
  fHabitatVolume = new G4LogicalVolume(habitatSolid, air, "Habitat");
  new G4PVPlacement(
    nullptr, G4ThreeVector(), fHabitatVolume, "Habitat", worldLogical, false, 0, true);

  G4Tubs* alShieldSolid =
    new G4Tubs("AlShield", kAlInnerR, kAlOuterR, kHabitatHalfZ, 0.0, 360.0 * deg);
  G4LogicalVolume* alShieldLogical =
    new G4LogicalVolume(alShieldSolid, aluminum, "AlShield");
  new G4PVPlacement(
    nullptr, G4ThreeVector(), alShieldLogical, "AlShield", worldLogical, false, 0, true);

  G4Tubs* polyShieldSolid =
    new G4Tubs("PolyShield", kPolyInnerR, kPolyOuterR, kHabitatHalfZ, 0.0, 360.0 * deg);
  G4LogicalVolume* polyShieldLogical =
    new G4LogicalVolume(polyShieldSolid, polyethylene, "PolyShield");
  new G4PVPlacement(
    nullptr, G4ThreeVector(), polyShieldLogical, "PolyShield", worldLogical, false, 0, true);

  G4Tubs* solenoidSolid = new G4Tubs(
    "Solenoid", kSolenoidInnerR, kSolenoidOuterR, kSolenoidHalfZ, 0.0, 360.0 * deg);
  G4LogicalVolume* solenoidLogical = new G4LogicalVolume(solenoidSolid, mgB2, "Solenoid");
  new G4PVPlacement(
    nullptr, G4ThreeVector(), solenoidLogical, "Solenoid", worldLogical, false, 0, true);

  G4Torus* ringSolid =
    new G4Torus("Ring", 0.0, kRingTubeR, kRingTorusR, 0.0, 360.0 * deg);
  G4LogicalVolume* ringLogical = new G4LogicalVolume(ringSolid, mgB2, "Ring");
  const G4double zPositions[4] = {-4.5 * m, -1.5 * m, 1.5 * m, 4.5 * m};

  for (int i = 0; i < 4; ++i) {
    const G4String ringName = "Ring_" + std::to_string(i);
    new G4PVPlacement(nullptr,
                      G4ThreeVector(0.0, 0.0, zPositions[i]),
                      ringLogical,
                      ringName,
                      worldLogical,
                      false,
                      i,
                      true);
  }

  return worldPhysical;
}

void DetectorConstruction::ConstructSDandField()
{
  auto* magneticField = new MagneticField();

  G4FieldManager* fieldManager =
    G4TransportationManager::GetTransportationManager()->GetFieldManager();
  fieldManager->SetDetectorField(magneticField);

  auto* equation = new G4Mag_UsualEqRhs(magneticField);
  auto* stepper = new G4ClassicalRK4(equation);

  constexpr G4double minStep = 1.0 * mm;
  auto* chordFinder = new G4ChordFinder(magneticField, minStep, stepper);
  fieldManager->SetChordFinder(chordFinder);

  fieldManager->SetMinimumEpsilonStep(1.0e-4);
  fieldManager->SetDeltaIntersection(1.0e-2 * mm);
}
