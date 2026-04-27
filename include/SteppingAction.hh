#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"

class DetectorConstruction;
class EventAction;
class RunAction;
class G4Step;

class SteppingAction : public G4UserSteppingAction
{
public:
  SteppingAction(const DetectorConstruction* detectorConstruction,
                 EventAction* eventAction,
                 RunAction* runAction);
  ~SteppingAction() override;

  void UserSteppingAction(const G4Step* step) override;

private:
  const DetectorConstruction* fDetectorConstruction = nullptr;
  EventAction* fEventAction = nullptr;
  RunAction* fRunAction = nullptr;
};

#endif
