#include "MagneticField.hh"

#include <cmath>

namespace
{
constexpr G4double kRSolenoid = 2.5 * m;
constexpr G4double kRGradientMax = 6.5 * m;
constexpr G4double kRTailScale = 3.0 * m;
constexpr G4double kB0 = 2.0 * tesla;
constexpr G4double kBAtGradientEdge = 0.2 * tesla;
constexpr G4double kSlope = (kB0 - kBAtGradientEdge) / (kRGradientMax - kRSolenoid);
}  // namespace

MagneticField::MagneticField() = default;

MagneticField::~MagneticField() = default;

void MagneticField::GetFieldValue(const G4double point[4], G4double* bField) const
{
  const G4double x = point[0];
  const G4double y = point[1];
  const G4double r = std::sqrt(x * x + y * y);

  G4double bz = 0.0;
  if (r <= kRSolenoid) {
    bz = kB0;
  } else if (r < kRGradientMax) {
    bz = kB0 - kSlope * (r - kRSolenoid);
  } else {
    // Outside the gradient rings keep a decaying tail instead of hard cutoff.
    bz = kBAtGradientEdge * std::exp(-(r - kRGradientMax) / kRTailScale);
  }

  bField[0] = 0.0;
  bField[1] = 0.0;
  bField[2] = bz;
}
