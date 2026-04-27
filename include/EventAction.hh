#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

#include <unordered_set>

class G4Event;
class RunAction;

class EventAction : public G4UserEventAction
{
public:
  explicit EventAction(RunAction* runAction);
  ~EventAction() override;

  void BeginOfEventAction(const G4Event* event) override;
  void EndOfEventAction(const G4Event* event) override;

  bool MarkTrackEnteredHabitat(G4int trackId);
  bool MarkAnyEntryThisEvent();

  void SetPrimaryAimed(G4bool v) { fPrimaryAimed = v; }
  void SetPrimaryCharged(G4bool v) { fPrimaryCharged = v; }
  void SetPrimaryEnteredHabitat() { fPrimaryEnteredHabitat = true; }
  void SetNeutralSecondaryEnteredHabitat() { fNeutralSecondaryEnteredHabitat = true; }

private:
  RunAction* fRunAction = nullptr;

  std::unordered_set<G4int> fTracksEnteredHabitat;
  bool fAnyEntryThisEvent = false;

  G4bool fPrimaryAimed = false;
  G4bool fPrimaryCharged = false;
  G4bool fPrimaryEnteredHabitat = false;
  G4bool fNeutralSecondaryEnteredHabitat = false;
};

#endif
