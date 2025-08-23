#include <CLHEP/Units/SystemOfUnits.h>

#include <G4Ellipsoid.hh>
#include <G4IntersectionSolid.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4PVPlacement.hh>
#include <G4Polyhedra.hh>
#include <G4Tubs.hh>
#include <G4UnionSolid.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VSolid.hh>
#include <G4VisAttributes.hh>
#include <GeoHexDichroiconFactory.hh>
#include <RAT/GeoTubeFactory.hh>
#include <RAT/Materials.hh>

#include "Geant4/G4Types.hh"

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
  return phys;
}

G4LogicalVolume *GeoHexDichroiconFactory::MakeDichroicon(RAT::DBLinkPtr table) {
  std::string volume_name = table->GetIndex();
  std::string material_name = table->GetS("material");
  G4LogicalVolume *lv = NULL;

  std::vector<double> rs = table->GetDArray("rs");
  std::vector<double> zs = table->GetDArray("zs");
  for (auto &r : rs) r *= CLHEP::mm;
  for (auto &z : zs) z *= CLHEP::mm;
  if (rs.size() != zs.size()) {
    RAT::Log::Die("GeoHexDichroiconFactory error: " + table->GetName() + "[" + table->GetIndex() +
                  "].rs and zs must have the same number of elements");
  }
  auto hex_solid = new G4Polyhedra("hexdichroicon", 0, 360 * CLHEP::deg, 6, rs.size(), rs.data(), zs.data());
  lv = new G4LogicalVolume(hex_solid, G4Material::GetMaterial(material_name), volume_name);
  double max_r = *std::max_element(rs.begin(), rs.end());
  auto trisect_vol = new G4IntersectionSolid("hexdichroicon_trisec", hex_solid, hex_solid,
                                             G4Transform3D(G4RotationMatrix(), G4ThreeVector(max_r * CLHEP::mm, 0, 0)));
  G4LogicalVolume *trisect_lv1 =
      new G4LogicalVolume(trisect_vol, G4Material::GetMaterial(material_name), volume_name + "_trisec");
  G4VPhysicalVolume *trisect_phys1 = new G4PVPlacement(G4Transform3D(G4RotationMatrix(), G4ThreeVector(0, 0, 0)),
                                                       trisect_lv1, volume_name + "_trisec1", lv, false, 0, true);
  G4VPhysicalVolume *trisect_phys2 =
      new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateZ(120 * CLHEP::deg), G4ThreeVector(0, 0, 0)),
                        trisect_lv1, volume_name + "_trisec2", lv, false, 1, true);
  G4VPhysicalVolume *trisect_phys3 =
      new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateZ(240 * CLHEP::deg), G4ThreeVector(0, 0, 0)),
                        trisect_lv1, volume_name + "_trisec3", lv, false, 2, true);
  // TODO: Surface

  // Optional visualization parts
  G4VisAttributes *vis = new G4VisAttributes();

  try {
    const std::vector<double> &color = table->GetDArray("color");
    if (color.size() == 3)  // RGB
      vis->SetColour(G4Colour(color[0], color[1], color[2]));
    else if (color.size() == 4)  // RGBA
      vis->SetColour(G4Colour(color[0], color[1], color[2], color[3]));
    else
      RAT::warn << "GeoSolidFactory error: " << table->GetName() << "[" << table->GetIndex()
                << "].color must have 3 or 4 components" << newline;
  } catch (RAT::DBNotFoundError &e) {
  };
  // NOTE: G4 Voxel Optimization -- should disable?

  try {
    std::string drawstyle = table->GetS("drawstyle");
    if (drawstyle == "wireframe")
      vis->SetForceWireframe(true);
    else if (drawstyle == "solid")
      vis->SetForceSolid(true);
    else
      RAT::warn << "GeoSolidFactory error: " << table->GetName() << "[" << table->GetIndex()
                << "].drawstyle must be either \"wireframe\" or \"solid\".";
  } catch (RAT::DBNotFoundError &e) {
  };

  try {
    int force_auxedge = table->GetI("force_auxedge");
    vis->SetForceAuxEdgeVisible(force_auxedge == 1);
  } catch (RAT::DBNotFoundError &e) {
  };

  lv->SetVisAttributes(vis);

  // Check for invisible flag last
  try {
    int invisible = table->GetI("invisible");
    if (invisible == 1) lv->SetVisAttributes(G4VisAttributes::GetInvisible());
  } catch (RAT::DBNotFoundError &e) {
  };

  return lv;
}

}  // namespace Dichroicon
