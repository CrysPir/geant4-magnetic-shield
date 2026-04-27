#include "MagneticField.hh"

#include <cmath>

namespace
{
constexpr G4double kRSolenoid = 2.5 * m;
constexpr G4double kRGradientMax = 6.5 * m;
constexpr G4double kB0 = 2.0 * tesla;
constexpr G4double kSlope = 1.0 * tesla / m;
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
    if (bz < 0.0) {
      bz = 0.0;
    }
  } else {
    bz = 0.0;
  }

  bField[0] = 0.0;
  bField[1] = 0.0;
  bField[2] = bz;
}
