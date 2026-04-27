#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"

#include "FTFP_BERT.hh"
#include "G4PhysListFactory.hh"
#include "G4RunManagerFactory.hh"
#include "G4String.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4VUserPhysicsList.hh"
#include "Randomize.hh"

int main(int argc, char** argv)
{
  auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::SerialOnly);
  auto* detectorConstruction = new DetectorConstruction();
  runManager->SetUserInitialization(detectorConstruction);

  G4PhysListFactory factory;
  G4VUserPhysicsList* physicsList = nullptr;
  const G4String candidates[] = {"FTFP_BERT_HP", "Shielding", "QGSP_BIC_HP"};
  for (const auto& name : candidates) {
    if (factory.IsReferencePhysList(name)) {
      physicsList = factory.GetReferencePhysList(name);
      break;
    }
  }
  if (!physicsList) {
    physicsList = new FTFP_BERT;
  }
  runManager->SetUserInitialization(physicsList);

  runManager->SetUserInitialization(new ActionInitialization(detectorConstruction));

  runManager->Initialize();

  auto* visManager = new G4VisExecutive;
  visManager->Initialize();

  auto* uiManager = G4UImanager::GetUIpointer();

  if (argc == 1) {
    auto* ui = new G4UIExecutive(argc, argv);
    uiManager->ApplyCommand("/control/execute macros/vis.mac");
    ui->SessionStart();
    delete ui;
  } else {
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    uiManager->ApplyCommand(command + fileName);
  }

  delete visManager;
  delete runManager;
  return 0;
}
