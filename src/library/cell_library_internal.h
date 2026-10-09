#pragma once

#include <memory>
#include <string>
#include <vector>

#include "boop/library/cell_library.h"

#ifdef BOOP_USE_MOCKTURTLE_LIBRARY
#include <mockturtle/io/genlib_reader.hpp>
#include <mockturtle/utils/tech_library.hpp>
#endif

namespace boop {

#ifdef BOOP_USE_MOCKTURTLE_LIBRARY
struct CellLibrary::MappingCache {
  std::vector<mockturtle::gate> vGates;
  std::unique_ptr<mockturtle::tech_library<6>> pMapping;
  std::vector<std::vector<unsigned>> vvBindingIds;
  std::vector<int> vCellIdsByBindingId;
};

struct CellLibraryMockturtleAccess {
  static void Ensure(const CellLibrary &library);
  static const std::vector<mockturtle::gate> &Gates(const CellLibrary &library);
  static const mockturtle::tech_library<6> &Mapping(const CellLibrary &library);
  static const std::vector<std::vector<unsigned>> &
  BindingIds(const CellLibrary &library);
  static const std::vector<int> &CellIdsByBindingId(const CellLibrary &library);
};
#endif

} // namespace boop
