#include <Dichroicon.hh>

namespace DICHROICON {
Dichroicon::Dichroicon(RAT::AnyParse *p, int argc, char **argv)
    : Rat(p, argc, argv) {
  // Append an additional data directory (for ratdb and geo)
  char *dichroicondata = getenv("DICHROICONDATA");
  if (dichroicondata != NULL) {
    ratdb_directories.insert(static_cast<std::string>(dichroicondata) +
                             "/ratdb");
    model_directories.insert(static_cast<std::string>(dichroicondata) +
                             "/models");
  }
  // Initialize a geometry factory
  // Include a new type of processor
  // Add a unique component to the datastructure
}
} // namespace DICHROICON
