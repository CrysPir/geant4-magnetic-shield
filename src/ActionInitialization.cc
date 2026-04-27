#include "ActionInitialization.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh"
#include "TrackingAction.hh"

ActionInitialization::ActionInitialization(const DetectorConstruction* detectorConstruction)
  : fDetectorConstruction(detectorConstruction)
{
}

ActionInitialization::~ActionInitialization() = default;

void ActionInitialization::BuildForMaster() const
{
}

void ActionInitialization::Build() const
{
  auto* runAction = new RunAction();
  SetUserAction(runAction);

  auto* eventAction = new EventAction(runAction);
  SetUserAction(eventAction);

  SetUserAction(new PrimaryGeneratorAction(eventAction));
  SetUserAction(new SteppingAction(fDetectorConstruction, eventAction, runAction));
  SetUserAction(new TrackingAction(runAction));
}
