#include <CLHEP/Geometry/Transform3D.h>
#include <CLHEP/Units/SystemOfUnits.h>

#include <G4Box.hh>
#include <G4Ellipsoid.hh>
#include <G4IntersectionSolid.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4LogicalVolume.hh>
#include <G4Material.hh>
#include <G4PVPlacement.hh>
#include <G4Polyhedra.hh>
#include <G4SubtractionSolid.hh>
#include <G4Tubs.hh>
#include <G4UnionSolid.hh>
#include <G4VPhysicalVolume.hh>
#include <G4VSolid.hh>
#include <G4VisAttributes.hh>
#include <GeoHexDichroiconFactory.hh>
#include <RAT/GeoTubeFactory.hh>
#include <RAT/Materials.hh>

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

  std::vector<double> rs = table->GetDArray("rs");
  std::vector<double> zs = table->GetDArray("zs");
  for (auto &r : rs) r *= CLHEP::mm;
  for (auto &z : zs) z *= CLHEP::mm;
  if (rs.size() != zs.size()) {
    RAT::Log::Die("GeoHexDichroiconFactory error: " + table->GetName() + "[" + table->GetIndex() +
                  "].rs and zs must have the same number of elements");
  }
  double max_r = *std::max_element(rs.begin(), rs.end());
  RAT::debug << "GeoHexDichroiconFactory: "
             << "Max radius: " << max_r / CLHEP::mm << " mm" << newline;
  std::vector<double> panel_size = table->GetDArray("panel_size");
  double panel_x = panel_size.at(0) * CLHEP::mm;
  double panel_y = panel_size.at(1) * CLHEP::mm;
  double panel_z = panel_size.at(2) * CLHEP::mm;
  RAT::debug << "GeoHexDichroiconFactory: "
             << "Panel size: " << panel_x / CLHEP::mm << " mm x " << panel_y / CLHEP::mm << " mm x "
             << panel_z / CLHEP::mm << " mm" << newline;
  auto panel_solid = new G4Box(volume_name + "_panel_solid", panel_x, panel_y, panel_z);
  G4LogicalVolume *panel_lv =
      new G4LogicalVolume(panel_solid, G4Material::GetMaterial(material_name), volume_name + "_panel_lv");
  G4VisAttributes *panel_vis = new G4VisAttributes();
  panel_vis->SetForceWireframe(true);
  // panel_vis->SetColor(G4Color(1.0, 0.0, 0.0, 0.01));
  panel_lv->SetVisAttributes(panel_vis);

  // full winstton cone
  auto hex_solid = new G4Polyhedra(volume_name + "_hex_solid", 0, 360 * CLHEP::deg, 6, rs.size(), rs.data(), zs.data());
  // intersecting regions
  auto trisect1_solid =
      new G4IntersectionSolid(volume_name + "_trisec1_solid", hex_solid, hex_solid, G4Translate3D(max_r, 0, 0));
  auto trisect2_solid = new G4IntersectionSolid(volume_name + "_trisec2_solid", hex_solid, hex_solid,
                                                G4Translate3D(-max_r / 2., max_r * (std::sqrt(3) / 2), 0));
  auto trisect3_solid = new G4IntersectionSolid(volume_name + "_trisec3_solid", hex_solid, hex_solid,
                                                G4Translate3D(-max_r / 2., -max_r * (std::sqrt(3) / 2), 0));
  // non-intersecting base
  auto hex_base_solid =
      new G4SubtractionSolid(volume_name + "_base_solid_partial1", hex_solid, hex_solid, G4Translate3D(max_r, 0, 0));
  hex_base_solid = new G4SubtractionSolid(volume_name + "_base_solid_partial2", hex_base_solid, hex_solid,
                                          G4Translate3D(-max_r / 2., max_r * (std::sqrt(3) / 2), 0));
  hex_base_solid = new G4SubtractionSolid(volume_name + "_base_solid", hex_base_solid, hex_solid,
                                          G4Translate3D(-max_r / 2., -max_r * (std::sqrt(3) / 2), 0));

  // logical volumes
  const G4VisAttributes *vis = GetVisAttributes(table);
  auto trisect1_lv =
      new G4LogicalVolume(trisect1_solid, G4Material::GetMaterial(material_name), volume_name + "_trisec1_lv");
  trisect1_lv->SetVisAttributes(vis);
  auto trisect2_lv =
      new G4LogicalVolume(trisect2_solid, G4Material::GetMaterial(material_name), volume_name + "_trisec2_lv");
  trisect2_lv->SetVisAttributes(vis);
  auto trisect3_lv =
      new G4LogicalVolume(trisect3_solid, G4Material::GetMaterial(material_name), volume_name + "_trisec3_lv");
  trisect3_lv->SetVisAttributes(vis);
  // define two bases, one for each type of dichroicon
  auto hex_base_a_lv =
      new G4LogicalVolume(hex_base_solid, G4Material::GetMaterial(material_name), volume_name + "_base_a_lv");
  hex_base_a_lv->SetVisAttributes(vis);
  auto hex_base_b_lv =
      new G4LogicalVolume(hex_base_solid, G4Material::GetMaterial(material_name), volume_name + "_base_b_lv");
  // TODO: set different vis attributes for b type
  // TODO: set surfaces

  // determine placements of the cones
  // subtract off some edge to determine the range of hex centers.
  panel_x -= max_r;
  panel_y -= max_r * std::sqrt(3) / 2;
  // spacing between hex centers
  double delta_x = max_r * 1.5;
  double delta_y = max_r * std::sqrt(3);
  size_t ncols = std::floor(panel_x / delta_x);
  size_t nrows = std::floor(panel_y / delta_y);
  // loop over all rows and columns. For each iteration, add hexagons in all 4 quadrants.
  //
  // Coordinate in Hexagonal Efficient Coordinate System:
  // https://en.wikipedia.org/wiki/Hexagonal_Efficient_Coordinate_System
  // Flipped 90 degree from standard orientation.
  int hecs_a = 0;  // alternates between 0 and 1 every column
  int hecs_c = 0;  // increments every two columns.
  int hecs_r = 0;  // row number.
  int copy_number = 0;
  auto place_cone = [&](double xx, double yy) {
    new G4PVPlacement(G4Translate3D(xx, yy, 0),
                      hex_base_a_lv,                                                // what LV to place
                      volume_name + "_base_a_phys_" + std::to_string(copy_number),  // name
                      panel_lv,                                                     // mother LV
                      false,        // many placement (dummy variable in G4, not used)
                      copy_number,  // the copy number
                      true          // perform overlap check on the fly
    );
    new G4PVPlacement(G4Translate3D(xx, yy, 0),
                      trisect1_lv,                                                    // what LV to place
                      volume_name + "_trisect1_phys_" + std::to_string(copy_number),  // name
                      panel_lv,                                                       // mother LV
                      false,        // many placement (dummy variable in G4, not used)
                      copy_number,  // the copy number
                      true          // perform overlap check on the fly
    );
    new G4PVPlacement(G4Translate3D(xx, yy, 0),
                      trisect2_lv,                                                    // what LV to place
                      volume_name + "_trisect2_phys_" + std::to_string(copy_number),  // name
                      panel_lv,                                                       // mother LV
                      false,        // many placement (dummy variable in G4, not used)
                      copy_number,  // the copy number
                      true          // perform overlap check on the fly
    );
    new G4PVPlacement(G4Translate3D(xx, yy, 0),
                      trisect3_lv,                                                    // what LV to place
                      volume_name + "_trisect3_phys_" + std::to_string(copy_number),  // name
                      panel_lv,                                                       // mother LV
                      false,        // many placement (dummy variable in G4, not used)
                      copy_number,  // the copy number
                      true          // perform overlap check on the fly
    );
    // Always place base_b on the right side of the type-a hex.
    // FIXME: this works OK in the middle but not at the edges.
    if (xx + max_r < panel_x) {
      new G4PVPlacement(G4Transform3D(G4RotationMatrix().rotateZ(180 * CLHEP::deg), G4ThreeVector(xx + max_r, yy, 0)),
                        hex_base_b_lv,                                                // what LV to place
                        volume_name + "_base_b_phys_" + std::to_string(copy_number),  // name
                        panel_lv,                                                     // mother LV
                        false,        // many placement (dummy variable in G4, not used)
                        copy_number,  // the copy number
                        true          // perform overlap check on the fly
      );
    }
    copy_number++;
  };
  for (size_t col = 0; col <= ncols; col++) {
    hecs_r = 0;
    for (size_t row = 0; row <= nrows; row++) {
      double x = 3.0 * (hecs_c + hecs_a / 2.0) * max_r;
      double y = std::sqrt(3) * (hecs_r + hecs_a / 2.0) * max_r;
      if (x > panel_x || y > panel_y)
        continue;  // this check is still necessary despite the ncol/ncrows calculation because the a=1 column can still
                   // go out of bounds.
      RAT::debug << "x = " << x / CLHEP::mm << " mm, y = " << y / CLHEP::mm << " mm" << newline;
      place_cone(x, y);

      if (x != 0) place_cone(-x, y);
      if (y != 0) place_cone(x, -y);
      if (x != 0 && y != 0) place_cone(-x, -y);
      hecs_r++;
    }
    if (hecs_a == 1) hecs_c++;
    hecs_a = 1 - hecs_a;
  }
  return panel_lv;
}

const G4VisAttributes *GeoHexDichroiconFactory::GetVisAttributes(RAT::DBLinkPtr table) {
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
  // Check for invisible flag last
  try {
    int invisible = table->GetI("invisible");
    if (invisible == 1) {
      delete vis;
      return &G4VisAttributes::GetInvisible();
    }
  } catch (RAT::DBNotFoundError &e) {
  };
  return vis;
}

}  // namespace Dichroicon
