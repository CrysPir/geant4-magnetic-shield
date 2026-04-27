#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"

class DetectorConstruction;

class ActionInitialization : public G4VUserActionInitialization
{
public:
  explicit ActionInitialization(const DetectorConstruction* detectorConstruction);
  ~ActionInitialization() override;

  void BuildForMaster() const override;
  void Build() const override;

private:
  const DetectorConstruction* fDetectorConstruction = nullptr;
};

#endif
