#include "EventAction.hh"

#include "RunAction.hh"

#include "G4Event.hh"

EventAction::EventAction(RunAction* runAction) : fRunAction(runAction) {}

EventAction::~EventAction() = default;

void EventAction::BeginOfEventAction(const G4Event*)
{
  fTracksEnteredHabitat.clear();
  fAnyEntryThisEvent = false;
  // Do not clear fPrimaryAimed / fPrimaryCharged here: some Geant4 builds invoke
  // BeginOfEventAction after primary generation, which would erase values set
  // in PrimaryGeneratorAction::GeneratePrimaries and break aimed-at-habitat metrics.
  fPrimaryEnteredHabitat = false;
  fNeutralSecondaryEnteredHabitat = false;
}

void EventAction::EndOfEventAction(const G4Event*)
{
  if (!fRunAction) {
    return;
  }
  fRunAction->RegisterEventOutcome(
    fPrimaryAimed, fPrimaryCharged, fPrimaryEnteredHabitat, fNeutralSecondaryEnteredHabitat);
}

bool EventAction::MarkTrackEnteredHabitat(G4int trackId)
{
  return fTracksEnteredHabitat.insert(trackId).second;
}

bool EventAction::MarkAnyEntryThisEvent()
{
  if (fAnyEntryThisEvent) {
    return false;
  }

  fAnyEntryThisEvent = true;
  return true;
}
