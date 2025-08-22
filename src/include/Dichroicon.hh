#ifndef __DICHROICON_Dichroicon__
#define __DICHROICON_Dichroicon__

#include <Config.hh>
#include <GeoHexDichroiconFactory.hh>
#include <RAT/AnyParse.hh>
#include <RAT/ProcAllocator.hh>
#include <RAT/ProcBlockManager.hh>
#include <RAT/Rat.hh>

namespace Dichroicon {
class Dichroicon : public RAT::Rat {
 public:
  Dichroicon(RAT::AnyParse *p, int argc, char **argv);
};
}  // namespace Dichroicon

#endif  // __DICHROICON_Dichroicon__
