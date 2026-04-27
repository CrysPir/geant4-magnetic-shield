#ifndef TrackingAction_h
#define TrackingAction_h 1

#include "G4UserTrackingAction.hh"

class G4Track;
class RunAction;

class TrackingAction : public G4UserTrackingAction
{
public:
  explicit TrackingAction(RunAction* runAction);
  ~TrackingAction() override;

  void PostUserTrackingAction(const G4Track* track) override;

private:
  RunAction* fRunAction = nullptr;
};

#endif
