#include <vector>

#include "boop/interface/mockturtle_interface.h"
#include "boop/library/cell_library.h"
#include "boop/network/and_network.h"
#include "boop/network/bound_network.h"

int main() {
  boop::CellLibrary cell_library;
  const boop::CellLibrary::TimingArc arc = {boop::CellLibrary::UNKNOWN, 1.0,
                                            0.0, 1.0, 0.0};
  cell_library.AddCell("zero", {}, {}, {"Y"}, {999.0}, {"0"}, {0.0}, {{}});
  cell_library.AddCell("one", {}, {}, {"Y"}, {999.0}, {"1"}, {0.0}, {{}});
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
  if (!boop::MockturtleMap(&network, &mapped) || mapped.GetNumPis() != 2 ||
      mapped.GetNumPos() != 1 || mapped.GetNumCells() != 1) {
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
  if (!boop::MockturtleMap(&multi_output_network, &multi_output_mapped) ||
      multi_output_mapped.GetNumPis() != 2 ||
      multi_output_mapped.GetNumPos() != 2 ||
      multi_output_mapped.GetNumCells() != 1 ||
      multi_output_mapped.GetNumInstances() != 1) {
    return 1;
  }

  boop::AndNetwork constant_network;
  constant_network.AddPoEdge(constant_network.Node2Edge(constant_network.GetConst0(), false));
  constant_network.AddPoEdge(constant_network.Node2Edge(constant_network.GetConst0(), true));

  boop::BoundNetwork constant_mapped(&cell_library);
  if (!boop::MockturtleMap(&constant_network, &constant_mapped)) {
    return 1;
  }
  std::vector<int> constant_outputs;
  constant_mapped.ForEachPo([&](int output) {
    constant_outputs.push_back(constant_mapped.GetFanin(output, 0));
  });
  if (constant_outputs != std::vector<int>{constant_mapped.GetConst0(),
                                           constant_mapped.GetConst1()}) {
    return 1;
  }

  boop::CellLibrary no_tie_cell_library;
  no_tie_cell_library.AddCell("buf", {"a"}, {1.0}, {"Y"}, {999.0}, {"a"}, {1.0}, {{arc}});
  boop::BoundNetwork failed_mapping(&no_tie_cell_library);
  failed_mapping.AddPi();
  const int old_node_count = failed_mapping.GetNumNodes();
  if (boop::MockturtleMap(&constant_network, &failed_mapping) ||
      failed_mapping.GetNumNodes() != old_node_count ||
      failed_mapping.GetNumPis() != 1) {
    return 1;
  }
  no_tie_cell_library.AddCell("zero", {}, {}, {"Y"}, {999.0}, {"0"}, {0.0},
                              {{}});
  no_tie_cell_library.AddCell("one", {}, {}, {"Y"}, {999.0}, {"1"}, {0.0},
                              {{}});
  if (!boop::MockturtleMap(&constant_network, &failed_mapping)) {
    return 1;
  }
  return 0;
}
