#pragma once
#include <RAT/DB.hh>
#include <RAT/GeoFactory.hh>

namespace Dichroicon {
class GeoHexDichroiconFactory : public RAT::GeoFactory {
 public:
  GeoHexDichroiconFactory() : GeoFactory("hexdichroicon"){};
  G4VPhysicalVolume* Construct(RAT::DBLinkPtr table) override;
  G4LogicalVolume* MakeDichroicon(RAT::DBLinkPtr table);
};

}  // namespace Dichroicon
