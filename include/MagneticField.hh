#ifndef MagneticField_h
#define MagneticField_h 1

#include "G4MagneticField.hh"
#include "G4SystemOfUnits.hh"

class MagneticField : public G4MagneticField
{
public:
  MagneticField();
  ~MagneticField() override;

  void GetFieldValue(const G4double point[4], G4double* bField) const override;
};

#endif
