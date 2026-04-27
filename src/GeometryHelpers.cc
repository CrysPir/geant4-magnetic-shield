#include "GeometryHelpers.hh"

#include <cmath>

namespace
{
constexpr G4double kEps = 1.0e-12;
}

G4bool GeometryHelpers::RayIntersectsFiniteCylinder(const G4ThreeVector& origin,
                                                    const G4ThreeVector& dirUnit,
                                                    G4double radius,
                                                    G4double halfZ)
{
  const G4double ox = origin.x();
  const G4double oy = origin.y();
  const G4double oz = origin.z();
  const G4double r0 = std::hypot(ox, oy);
  if (r0 <= radius + kEps && std::abs(oz) <= halfZ + kEps) {
    return true;
  }
  const G4double dx = dirUnit.x();
  const G4double dy = dirUnit.y();
  const G4double dz = dirUnit.z();

  G4double tMin = 0.0;
  G4double tMax = 1.0e30;

  if (std::abs(dz) > kEps) {
    G4double t1 = (-halfZ - oz) / dz;
    G4double t2 = (halfZ - oz) / dz;
    if (t1 > t2) {
      std::swap(t1, t2);
    }
    tMin = std::max(tMin, t1);
    tMax = std::min(tMax, t2);
    if (tMin > tMax) {
      return false;
    }
  } else {
    if (oz < -halfZ || oz > halfZ) {
      return false;
    }
  }

  const G4double a = dx * dx + dy * dy;
  if (a < kEps * kEps) {
    const G4double r2 = ox * ox + oy * oy;
    if (r2 > radius * radius) {
      return false;
    }
    return tMax >= tMin && tMax >= 0.0;
  }

  const G4double b = 2.0 * (ox * dx + oy * dy);
  const G4double c = ox * ox + oy * oy - radius * radius;
  const G4double disc = b * b - 4.0 * a * c;
  if (disc < 0.0) {
    return false;
  }

  const G4double sqrtD = std::sqrt(disc);
  G4double tCylMin = (-b - sqrtD) / (2.0 * a);
  G4double tCylMax = (-b + sqrtD) / (2.0 * a);
  if (tCylMin > tCylMax) {
    std::swap(tCylMin, tCylMax);
  }

  if (tCylMax < 0.0) {
    return false;
  }

  if (tCylMin < 0.0 && tCylMax >= 0.0) {
    const G4double r0 = std::hypot(ox, oy);
    if (r0 > radius + kEps) {
      return false;
    }
    tCylMin = 0.0;
  }

  const G4double tEnter = std::max(tMin, tCylMin);
  const G4double tExit = std::min(tMax, tCylMax);
  if (tEnter > tExit) {
    return false;
  }

  if (tExit < 0.0) {
    return false;
  }

  return true;
}
