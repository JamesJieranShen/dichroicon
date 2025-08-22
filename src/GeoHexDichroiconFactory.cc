#include <CLHEP/Units/SystemOfUnits.h>

#include <G4Ellipsoid.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4PVPlacement.hh>
#include <G4SubtractionSolid.hh>
#include <G4Tubs.hh>
#include <G4UnionSolid.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VSolid.hh>
#include <GeoHexDichroiconFactory.hh>
#include <RAT/GeoTubeFactory.hh>

namespace Dichroicon {

G4VPhysicalVolume *GeoHexDichroiconFactory::Construct(RAT::DBLinkPtr table) {
  std::string mother_name = table->GetS("mother");
  G4LogicalVolume *mother;
  if (mother_name == "")
    mother = 0;  // World volume has no mother
  else {
    mother = FindMother(mother_name);
    if (mother == 0)
      RAT::Log::Die("Unable to find mother volume \"" + mother_name + "\" for " + table->GetName() + "[" +
                    table->GetIndex() + "]");
  }
  G4LogicalVolume *logi = MakeDichroicon(table);
  G4VPhysicalVolume *phys = ConstructPhysicalVolume(logi, mother, table);
}

G4LogicalVolume *GeoHexDichroiconFactory::MakeDichroicon(RAT::DBLinkPtr table) { return nullptr; }

}  // namespace Dichroicon
