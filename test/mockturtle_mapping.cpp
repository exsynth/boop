#include <sstream>
#include <vector>

#include <lorina/genlib.hpp>
#include <mockturtle/io/genlib_reader.hpp>
#include <mockturtle/utils/tech_library.hpp>

#include "boop/interface/mockturtle_interface.h"
#include "boop/library/cell_library.h"
#include "boop/network/and_network.h"
#include "boop/network/bound_network.h"

int main() {
  std::istringstream genlib("GATE and2 1 Y=a*b; PIN * UNKNOWN 1 999 1 0 1 0\n"
                            "GATE xor2 1 Y=a^b; PIN * UNKNOWN 1 999 1 0 1 0\n"
                            "GATE inv 1 Y=!a; PIN * INV 1 999 1 0 1 0\n"
                            "GATE buf 1 Y=a; PIN * NONINV 1 999 1 0 1 0\n"
                            "GATE ha 1.5 S=a^b; PIN * UNKNOWN 1 999 1 0 1 0\n"
                            "GATE ha 1.5 C=a*b; PIN * UNKNOWN 1 999 1 0 1 0\n");
  std::vector<mockturtle::gate> gates;
  if (lorina::read_genlib(genlib, mockturtle::genlib_reader(gates)) !=
      lorina::return_code::success) {
    return 1;
  }
  mockturtle::tech_library<6> tech_library(gates);

  boop::CellLibrary cell_library;
  const boop::CellLibrary::TimingArc arc = {boop::CellLibrary::UNKNOWN, 1.0,
                                            0.0, 1.0, 0.0};
  cell_library.AddCell("and2", {"a", "b"}, {1.0, 1.0}, {"Y"}, {999.0}, {"a*b"},
                       {1.0}, {{arc, arc}});
  cell_library.AddCell("xor2", {"a", "b"}, {1.0, 1.0}, {"Y"}, {999.0}, {"a^b"},
                       {1.0}, {{arc, arc}});
  cell_library.AddCell("inv", {"a"}, {1.0}, {"Y"}, {999.0}, {"!a"}, {1.0},
                       {{arc}});
  cell_library.AddCell("buf", {"a"}, {1.0}, {"Y"}, {999.0}, {"a"}, {1.0},
                       {{arc}});
  cell_library.AddCell("ha", {"a", "b"}, {1.0, 1.0}, {"S", "C"}, {999.0, 999.0},
                       {"a^b", "a*b"}, {2.0, 2.0}, {{arc, arc}, {arc, arc}});

  boop::AndNetwork network;
  const int a = network.AddPi();
  const int b = network.AddPi();
  const int y = network.AddAnd(a, b, false, false);
  network.AddPo(y, false);

  boop::BoundNetwork mapped(&cell_library);
  boop::MockturtleMap(&network, tech_library, &mapped);
  if (mapped.GetNumPis() != 2 || mapped.GetNumPos() != 1 ||
      mapped.GetNumCells() != 1) {
    return 1;
  }

  boop::AndNetwork multi_output_network;
  const int multi_a = multi_output_network.AddPi();
  const int multi_b = multi_output_network.AddPi();
  const int a_edge = multi_output_network.Node2Edge(multi_a, false);
  const int b_edge = multi_output_network.Node2Edge(multi_b, false);
  multi_output_network.AddPoEdge(
      multi_output_network.AddXorEdge(a_edge, b_edge));
  multi_output_network.AddPoEdge(
      multi_output_network.AddAndEdge(a_edge, b_edge));

  boop::BoundNetwork multi_output_mapped(&cell_library);
  boop::MockturtleMap(&multi_output_network, tech_library,
                      &multi_output_mapped);
  if (multi_output_mapped.GetNumPis() != 2 ||
      multi_output_mapped.GetNumPos() != 2 ||
      multi_output_mapped.GetNumCells() != 1 ||
      multi_output_mapped.GetNumInstances() != 1) {
    return 1;
  }
  return 0;
}
