#include "TrackingAction.hh"

#include "RunAction.hh"

#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4String.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VProcess.hh"

TrackingAction::TrackingAction(RunAction* runAction) : fRunAction(runAction) {}

TrackingAction::~TrackingAction() = default;

void TrackingAction::PostUserTrackingAction(const G4Track* track)
{
  if (!fRunAction) {
    return;
  }
  if (track->GetDefinition()->GetParticleName() != "neutron") {
    return;
  }
  if (track->GetTrackStatus() != fStopAndKill) {
    return;
  }

  const G4VPhysicalVolume* pv = track->GetVolume();
  if (!pv) {
    return;
  }
  const G4Material* mat = pv->GetLogicalVolume()->GetMaterial();
  if (!mat || mat->GetName() != "MgB2") {
    return;
  }

  const auto* lastStep = track->GetStep();
  G4String endProc;
  if (lastStep && lastStep->GetPostStepPoint() && lastStep->GetPostStepPoint()->GetProcessDefinedStep()) {
    endProc = lastStep->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName();
  }

  if (endProc.find("Capture") != G4String::npos) {
    fRunAction->AddNeutronCaptureInMgB2();
  }
}
