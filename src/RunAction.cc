#include "RunAction.hh"

#include "G4GenericMessenger.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PhysicalConstants.hh"
#include "G4Run.hh"
#include "G4SystemOfUnits.hh"

#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

namespace
{
constexpr std::size_t kMaxTagLen = 64;

/// Russian notes appended to run_summary.txt (same directory as run_summary_explained.txt).
std::string BuildRunSummaryExplained(const std::string& summaryText)
{
  static const char kExplainedTail[] =
    "\n\n"
    "--------------------------------------------------------------------------------\n"
    "Пояснения (метрики защиты и облучения жилого модуля)\n"
    "--------------------------------------------------------------------------------\n"
    "\n"
    "Primary aimed at habitat / entered habitat\n"
    "  Геометрический критерий: прямая из точки рождения первичного луча вдоль\n"
    "  начального импульса пересекает конечный цилиндр жилого модуля (R = 2 м,\n"
    "  полудлина по Z = 5 м). «Entered» — сколько из этих направленных первичных\n"
    "  реально вошли в объём (первое пересечение границы внутрь).\n"
    "\n"
    "Shielding efficiency (aimed primary, %)\n"
    "  100 * (N_aimed - N_entered) / N_aimed — доля направленных на цилиндр\n"
    "  первичных, которые не попали внутрь объёма (экран, поле и т.д.). Чем выше\n"
    "  процент, тем сильнее отсев по этой подвыборке.\n"
    "\n"
    "Primary aimed (charged only) / entered / efficiency charged\n"
    "  То же для заряженных первичных (гаммы не считаются). При совпадении счётчиков\n"
    "  с общими aimed/entered среди направленных на цилиндр нейтральных первичных\n"
    "  не было.\n"
    "\n"
    "Events with n/gamma (track!=1) entering habitat (% of events)\n"
    "  Доля событий, где во внутрь модуля зашёл хотя бы один вторичный нейтрон или\n"
    "  гамма (не первичный трек). Показывает нейтральную и каскадную компоненту\n"
    "  отдельно от первичного попадания.\n"
    "\n"
    "Absorbed dose in habitat (air proxy), Gy\n"
    "  Поглощённая энергия в объёме модуля (воздух NIST) / масса объёма. Это доза\n"
    "  в воздухе-заместителе, не в ткани; при данной статистике величина может быть\n"
    "  очень малой.\n"
    "\n"
    "Neutron captures in MgB2 (count)\n"
    "  Захваты нейтронов в MgB2 (материал соленоида). Ноль при малой статистике\n"
    "  или при настройке физики/порога не исключается; для нейтронов важен\n"
    "  корректный HP-список и пути к данным.\n"
    "\n"
    "Total track entries / Events with any habitat entry\n"
    "  Служебные счётчики: сколько раз любой трек впервые вошёл в модуль и в скольких\n"
    "  событиях был хотя бы один такой вход (первичный или вторичный).\n";

  return summaryText + kExplainedTail;
}

G4String SanitizeOutputTag(const G4String& raw)
{
  std::string s = raw;
  while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front())) != 0) {
    s.erase(s.begin());
  }
  while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())) != 0) {
    s.pop_back();
  }
  if (s.empty()) {
    return {};
  }
  for (char& c : s) {
    const unsigned char u = static_cast<unsigned char>(c);
    if (std::isalnum(u) != 0 || c == '_' || c == '-') {
      continue;
    }
    c = '_';
  }
  if (s.size() > kMaxTagLen) {
    s.resize(kMaxTagLen);
  }
  if (s == "." || s == "..") {
    return {};
  }
  return G4String(s);
}
}  // namespace

RunAction::RunAction() : fSpectrumCounts(kSpectrumBins, 0)
{
  DefineCommands();
}

RunAction::~RunAction()
{
  delete fMessenger;
}

void RunAction::DefineCommands()
{
  fMessenger = new G4GenericMessenger(this, "/sim/", "Simulation output");
  fMessenger->DeclareMethod("outputTag", &RunAction::SetOutputTag)
    .SetGuidance("Subdirectory under simulations/ for this run's reports "
                 "(run_summary.txt, run_summary_explained.txt, spectrum_passed_habitat.csv). "
                 "Letters, digits, underscore, hyphen only. Empty -> write under simulations/ directly.");
}

void RunAction::SetOutputTag(const G4String& tag)
{
  fOutputTag = SanitizeOutputTag(tag);
}

void RunAction::BeginOfRunAction(const G4Run*)
{
  fTracksEnteredHabitat = 0;
  fEventsWithHabitatEntry = 0;
  fNAimed = 0;
  fNAimedEntered = 0;
  fNAimedCharged = 0;
  fNAimedChargedEntered = 0;
  fNEventsNeutralInHabitat = 0;
  fEdepHabitat = 0.0;
  fNeutronCaptureMgB2 = 0;
  std::fill(fSpectrumCounts.begin(), fSpectrumCounts.end(), 0);
}

void RunAction::AddEdepHabitat(G4double edep)
{
  fEdepHabitat += edep;
}

void RunAction::FillPassedSpectrum(G4double kineticEnergy)
{
  if (kineticEnergy <= 0.0) {
    return;
  }
  const G4double logSpan = std::log(kSpecEmax / kSpecEmin);
  G4double x = std::log(kineticEnergy / kSpecEmin) / logSpan;
  if (x < 0.0) {
    x = 0.0;
  }
  if (x >= 1.0) {
    x = 1.0 - 1.0e-15;
  }
  G4int bin = static_cast<G4int>(x * kSpectrumBins);
  if (bin < 0) {
    bin = 0;
  }
  if (bin >= kSpectrumBins) {
    bin = kSpectrumBins - 1;
  }
  ++fSpectrumCounts[static_cast<std::size_t>(bin)];
}

void RunAction::AddNeutronCaptureInMgB2()
{
  ++fNeutronCaptureMgB2;
}

void RunAction::RegisterEventOutcome(G4bool primaryAimed,
                                     G4bool primaryCharged,
                                     G4bool primaryEnteredHabitat,
                                     G4bool neutralSecondaryEnteredHabitat)
{
  if (primaryAimed) {
    ++fNAimed;
    if (primaryEnteredHabitat) {
      ++fNAimedEntered;
    }
  }
  if (primaryAimed && primaryCharged) {
    ++fNAimedCharged;
    if (primaryEnteredHabitat) {
      ++fNAimedChargedEntered;
    }
  }
  if (neutralSecondaryEnteredHabitat) {
    ++fNEventsNeutralInHabitat;
  }
}

void RunAction::EndOfRunAction(const G4Run* run)
{
  const G4int nEvents = run->GetNumberOfEvent();
  const G4double nEventsD = static_cast<G4double>(std::max(1, nEvents));

  G4NistManager* nist = G4NistManager::Instance();
  G4Material* air = nist->FindOrBuildMaterial("G4_AIR");
  constexpr G4double kHabitatRadius = 2.0 * m;
  constexpr G4double kHabitatHalfZ = 5.0 * m;
  const G4double volume = pi * kHabitatRadius * kHabitatRadius * (2.0 * kHabitatHalfZ);
  const G4double mass = air->GetDensity() * volume;
  const G4double doseGy = (mass > 0.0) ? fEdepHabitat / mass : 0.0;
  const G4double doseSvEst = doseGy * fQualityFactor;

  const G4double effOverall =
    (fNAimed > 0) ? 100.0 * (1.0 - static_cast<G4double>(fNAimedEntered) / static_cast<G4double>(fNAimed)) : 0.0;
  const G4double effCharged =
    (fNAimedCharged > 0)
      ? 100.0 * (1.0 - static_cast<G4double>(fNAimedChargedEntered) / static_cast<G4double>(fNAimedCharged))
      : 0.0;
  const G4double fracNeutralEvents = 100.0 * static_cast<G4double>(fNEventsNeutralInHabitat) / nEventsD;

  std::ostringstream summary;
  summary << std::setprecision(17);
  summary << "=== Run summary ===\n";
  summary << "Run ID: " << run->GetRunID() << "\n";
  summary << "Output tag (simulations subdir): " << (fOutputTag.empty() ? G4String("(none)") : fOutputTag) << "\n";
  summary << "Events: " << nEvents << "\n";
  summary << "Primary aimed at habitat (finite cylinder): " << fNAimed << "\n";
  summary << "  ... entered habitat: " << fNAimedEntered << "\n";
  summary << "Shielding efficiency (aimed primary, %): " << effOverall << "\n";
  summary << "Primary aimed (charged only): " << fNAimedCharged << "\n";
  summary << "  ... entered: " << fNAimedChargedEntered << "\n";
  summary << "Shielding efficiency charged (aimed, %): " << effCharged << "\n";
  summary << "Events with n/gamma (track!=1) entering habitat (% of events): " << fracNeutralEvents << "\n";
  summary << "Absorbed dose in habitat (air proxy), Gy: " << doseGy << "\n";
  summary << "Equivalent dose estimate (Q=" << fQualityFactor << "), Sv: " << doseSvEst << "\n";
  summary << "Neutron captures in MgB2 (count): " << fNeutronCaptureMgB2 << "\n";
  summary << "Total track entries to habitat (diagnostic): " << fTracksEnteredHabitat << "\n";
  summary << "Events with any habitat entry: " << fEventsWithHabitatEntry << "\n";
  summary << "===================\n";

  const std::string summaryText = summary.str();
  G4cout << G4endl;
  G4cout << summaryText;

  namespace fs = std::filesystem;
  fs::path outDir = fs::path("simulations");
  if (!fOutputTag.empty()) {
    outDir /= std::string(fOutputTag);
  }
  std::error_code ec;
  fs::create_directories(outDir, ec);
  if (ec) {
    G4cerr << "Warning: could not create " << outDir.string() << ": " << ec.message() << G4endl;
  }

  const fs::path summaryPath = outDir / "run_summary.txt";
  {
    std::ofstream sumOut(summaryPath, std::ios::out | std::ios::trunc);
    if (sumOut) {
      sumOut << summaryText;
      sumOut.close();
      G4cout << "Run summary written to " << summaryPath.string() << G4endl;
    } else {
      G4cerr << "Warning: could not write " << summaryPath.string() << G4endl;
    }
  }

  const fs::path summaryExplainedPath = outDir / "run_summary_explained.txt";
  {
    const std::string explainedText = BuildRunSummaryExplained(summaryText);
    std::ofstream exOut(summaryExplainedPath, std::ios::out | std::ios::trunc);
    if (exOut) {
      exOut << explainedText;
      exOut.close();
      G4cout << "Run summary with notes written to " << summaryExplainedPath.string() << G4endl;
    } else {
      G4cerr << "Warning: could not write " << summaryExplainedPath.string() << G4endl;
    }
  }

  const fs::path spectrumPath = outDir / "spectrum_passed_habitat.csv";
  std::ofstream out(spectrumPath, std::ios::out | std::ios::trunc);
  out << "bin_low_MeV,bin_high_MeV,count\n";
  for (G4int i = 0; i < kSpectrumBins; ++i) {
    const G4double e0 = kSpecEmin * std::exp((static_cast<G4double>(i) / kSpectrumBins) * std::log(kSpecEmax / kSpecEmin));
    const G4double e1 =
      kSpecEmin * std::exp((static_cast<G4double>(i + 1) / kSpectrumBins) * std::log(kSpecEmax / kSpecEmin));
    out << std::setprecision(8) << e0 / MeV << "," << e1 / MeV << "," << fSpectrumCounts[static_cast<std::size_t>(i)]
        << "\n";
  }
  out.close();
  G4cout << "Spectrum (first KE in habitat) written to " << spectrumPath.string() << G4endl;
}

void RunAction::CountTrackEnteredHabitat()
{
  ++fTracksEnteredHabitat;
}

void RunAction::CountEventWithHabitatEntry()
{
  ++fEventsWithHabitatEntry;
}
