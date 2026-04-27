# Файл DetectorConstruction.hh:   
  
#ifndef DetectorConstruction_h  
#define DetectorConstruction_h 1  
  
#include "G4VUserDetectorConstruction.hh"  
#include "globals.hh"  
  
class G4VPhysicalVolume;  
class G4LogicalVolume;  
  
class DetectorConstruction : public G4VUserDetectorConstruction  
{  
public:  
    DetectorConstruction();  
    virtual ~DetectorConstruction();  
    virtual G4VPhysicalVolume* Construct();  
    virtual void ConstructSDandField();  
  
    // Метод для получения логического объёма жилого модуля  
    G4LogicalVolume* GetHabitatVolume() const { return fHabitatVolume; }  
  
private:  
    void DefineMaterials();  
    G4LogicalVolume* fHabitatVolume;  
};  
  
#endif  
  
  
### Файл DetectorConstruction.cc:  
  
#include "DetectorConstruction.hh"  
#include "G4NistManager.hh"  
#include "G4Box.hh"  
#include "G4Tubs.hh"  
#include "G4Torus.hh"  
#include "G4LogicalVolume.hh"  
#include "G4PVPlacement.hh"  
#include "G4SystemOfUnits.hh"  
#include "G4Material.hh"  
  
DetectorConstruction::DetectorConstruction()  
{}  
  
DetectorConstruction::~DetectorConstruction()  
{}  
  
void DetectorConstruction::DefineMaterials()  
{  
    G4NistManager* nist = G4NistManager::Instance();  
    // Определение материалов из NIST  
    nist->FindOrBuildMaterial("G4_AIR");  
    nist->FindOrBuildMaterial("G4_Al");  
    nist->FindOrBuildMaterial("G4_POLYETHYLENE");  
      
    // Определение MgB₂  
    G4Material* MgB2 = new G4Material("MgB2", 2.6*g/cm3, 2);  
    MgB2->AddElement(nist->FindOrBuildElement("Mg"), 1);  
    MgB2->AddElement(nist->FindOrBuildElement("B"), 2);  
}  
  
G4VPhysicalVolume* DetectorConstruction::Construct()  
{  
    DefineMaterials();  
    G4NistManager* nist = G4NistManager::Instance();  
    G4Material* air = nist->FindOrBuildMaterial("G4_AIR");  
    G4Material* Al = nist->FindOrBuildMaterial("G4_Al");  
    G4Material* poly = nist->FindOrBuildMaterial("G4_POLYETHYLENE");  
    G4Material* MgB2 = nist->FindOrBuildMaterial("MgB2");  
  
    // --- Мир (World) ---  
    G4Box* worldBox = new G4Box("World", 20*m, 20*m, 20*m);  
    G4LogicalVolume* worldLog = new G4LogicalVolume(worldBox, air, "World");  
    G4VPhysicalVolume* worldPhys = new G4PVPlacement(0, G4ThreeVector(), worldLog, "World", 0, false, 0);  
  
    // --- Жилой модуль (просто полость, но в неё можно будет добавлять физику) ---  
    G4Tubs* habitatSolid = new G4Tubs("Habitat", 0, 2*m, 5*m, 0, 360*deg);  
    fHabitatVolume = new G4LogicalVolume(habitatSolid, air, "Habitat");  
    new G4PVPlacement(0, G4ThreeVector(0,0,0), fHabitatVolume, "Habitat", worldLog, false, 0);  
  
    // --- Пассивная защита: 3 мм алюминия + 3 мм полиэтилена ---  
    // Внешний слой полиэтилена  
    G4Tubs* polyShieldSolid = new G4Tubs("PolyShield", 2.003*m, 2.006*m, 5.006*m, 0, 360*deg);  
    G4LogicalVolume* polyShieldLog = new G4LogicalVolume(polyShieldSolid, poly, "PolyShield");  
    new G4PVPlacement(0, G4ThreeVector(0,0,0), polyShieldLog, "PolyShield", worldLog, false, 0);  
      
    // Внутренний слой алюминия  
    G4Tubs* alShieldSolid = new G4Tubs("AlShield", 2.0*m, 2.003*m, 5.003*m, 0, 360*deg);  
    G4LogicalVolume* alShieldLog = new G4LogicalVolume(alShieldSolid, Al, "AlShield");  
    new G4PVPlacement(0, G4ThreeVector(0,0,0), alShieldLog, "AlShield", worldLog, false, 0);  
  
    // --- Соленоид из MgB₂ ---  
    G4Tubs* solenoidSolid = new G4Tubs("Solenoid", 2.5*m, 2.55*m, 6.0*m, 0, 360*deg);  
    G4LogicalVolume* solenoidLog = new G4LogicalVolume(solenoidSolid, MgB2, "Solenoid");  
    new G4PVPlacement(0, G4ThreeVector(0,0,0), solenoidLog, "Solenoid", worldLog, false, 0);  
  
    // --- Градиентные кольца (4 шт.) из MgB₂ ---  
    G4Torus* ringSolid = new G4Torus("Ring", 0, 0.1*m, 6.5*m, 0, 360*deg);  
    G4LogicalVolume* ringLog = new G4LogicalVolume(ringSolid, MgB2, "Ring");  
    G4double zPositions[4] = { -4.5*m, -1.5*m, 1.5*m, 4.5*m };  
    for (int i = 0; i < 4; ++i) {  
        new G4PVPlacement(0, G4ThreeVector(0, 0, zPositions[i]), ringLog,  
                          "Ring_" + std::to_string(i), worldLog, false, i);  
    }  
  
    return worldPhys;  
}  
  
void DetectorConstruction::ConstructSDandField()  
{  
    // Здесь будет подключено ваше поле  
}  
