#include "SteppingAction.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"
#include "RunAction.hh"

#include "G4LogicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"

SteppingAction::SteppingAction(const DetectorConstruction* detectorConstruction,
                               EventAction* eventAction,
                               RunAction* runAction)
  : fDetectorConstruction(detectorConstruction), fEventAction(eventAction), fRunAction(runAction)
{
}

SteppingAction::~SteppingAction() = default;

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  const auto* habitatLogical = fDetectorConstruction->GetHabitatVolume();
  if (!habitatLogical || !fRunAction || !fEventAction) {
    return;
  }

  const G4LogicalVolume* preLV = step->GetPreStepPoint()->GetPhysicalVolume()
                                   ? step->GetPreStepPoint()->GetPhysicalVolume()->GetLogicalVolume()
                                   : nullptr;
  const G4LogicalVolume* postLV = step->GetPostStepPoint()->GetPhysicalVolume()
                                    ? step->GetPostStepPoint()->GetPhysicalVolume()->GetLogicalVolume()
                                    : nullptr;

  if (preLV == habitatLogical || postLV == habitatLogical) {
    fRunAction->AddEdepHabitat(step->GetTotalEnergyDeposit());
  }

  const G4StepPoint* postPoint = step->GetPostStepPoint();
  if (postPoint->GetStepStatus() != fGeomBoundary) {
    return;
  }

  G4VPhysicalVolume* postVolume = postPoint->GetPhysicalVolume();
  if (!postVolume) {
    return;
  }

  if (postVolume->GetLogicalVolume() != habitatLogical) {
    return;
  }

  G4Track* track = step->GetTrack();
  const G4int trackId = track->GetTrackID();
  if (fEventAction->MarkTrackEnteredHabitat(trackId)) {
    fRunAction->CountTrackEnteredHabitat();
    fRunAction->FillPassedSpectrum(postPoint->GetKineticEnergy());

    if (fEventAction->MarkAnyEntryThisEvent()) {
      fRunAction->CountEventWithHabitatEntry();
    }

    if (trackId == 1) {
      fEventAction->SetPrimaryEnteredHabitat();
    } else {
      const G4ParticleDefinition* pd = track->GetDefinition();
      if (pd) {
        const G4String name = pd->GetParticleName();
        if (name == "neutron" || name == "gamma") {
          fEventAction->SetNeutralSecondaryEnteredHabitat();
        }
      }
    }
  }
}
