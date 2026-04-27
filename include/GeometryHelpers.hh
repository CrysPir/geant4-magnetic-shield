#ifndef GeometryHelpers_h
#define GeometryHelpers_h 1

#include "G4ThreeVector.hh"
#include "G4Types.hh"

namespace GeometryHelpers
{
/// Ray: origin + t * dir, t >= 0. Finite cylinder: r <= R, |z| <= halfZ (caps included).
G4bool RayIntersectsFiniteCylinder(const G4ThreeVector& origin,
                                   const G4ThreeVector& dirUnit,
                                   G4double radius,
                                   G4double halfZ);
}  // namespace GeometryHelpers

#endif
