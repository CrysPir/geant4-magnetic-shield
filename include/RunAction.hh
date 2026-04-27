#ifndef RunAction_h
#define RunAction_h 1

#include "G4String.hh"
#include "G4SystemOfUnits.hh"
#include "G4UserRunAction.hh"
#include "globals.hh"

#include <vector>

class G4GenericMessenger;
class G4Run;

class RunAction : public G4UserRunAction
{
public:
  RunAction();
  ~RunAction() override;

  void SetOutputTag(const G4String& tag);

  void BeginOfRunAction(const G4Run* run) override;
  void EndOfRunAction(const G4Run* run) override;

  void CountTrackEnteredHabitat();
  void CountEventWithHabitatEntry();

  void AddEdepHabitat(G4double edep);
  void FillPassedSpectrum(G4double kineticEnergy);
  void AddNeutronCaptureInMgB2();

  void RegisterEventOutcome(G4bool primaryAimed,
                            G4bool primaryCharged,
                            G4bool primaryEnteredHabitat,
                            G4bool neutralSecondaryEnteredHabitat);

  void SetQualityFactor(G4double q) { fQualityFactor = q; }

  [[nodiscard]] G4String GetOutputTag() const { return fOutputTag; }

private:
  void DefineCommands();

  G4GenericMessenger* fMessenger = nullptr;
  /// If non-empty, files go under simulations/<tag>/ (single path segment, sanitized).
  G4String fOutputTag;

  static constexpr G4int kSpectrumBins = 40;
  static constexpr G4double kSpecEmin = 0.1 * MeV;
  static constexpr G4double kSpecEmax = 1.0e7 * MeV;

  G4int fTracksEnteredHabitat = 0;
  G4int fEventsWithHabitatEntry = 0;

  G4int fNAimed = 0;
  G4int fNAimedEntered = 0;
  G4int fNAimedCharged = 0;
  G4int fNAimedChargedEntered = 0;
  G4int fNEventsNeutralInHabitat = 0;

  G4double fEdepHabitat = 0.0;
  G4int fNeutronCaptureMgB2 = 0;
  G4double fQualityFactor = 2.5;

  std::vector<G4long> fSpectrumCounts;
};

#endif
