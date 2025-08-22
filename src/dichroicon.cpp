#include <Dichroicon.hh>
#include <RAT/AnyParse.hh>
#include <RAT/Rat.hh>
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  auto parser = new RAT::AnyParse(argc, argv);
  auto dichroicon = DICHROICON::Dichroicon(parser, argc, argv);
  dichroicon.Begin();
  dichroicon.Report();
}
