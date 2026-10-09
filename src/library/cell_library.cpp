#include "boop/library/cell_library.h"

#include "cell_library_internal.h"
#include <cassert>

#ifdef BOOP_USE_MOCKTURTLE_LIBRARY
#include <cstdint>

#include <kitty/constructors.hpp>
#endif

BOOP_IMPL_START

namespace boop {

#ifdef BOOP_USE_MOCKTURTLE_LIBRARY
namespace {

mockturtle::phase_type ConvertPinPhase(CellLibrary::PinPhase phase) {
  switch (phase) {
  case CellLibrary::UNKNOWN:
    return mockturtle::phase_type::UNKNOWN;
  case CellLibrary::INV:
    return mockturtle::phase_type::INV;
  case CellLibrary::NONINV:
    return mockturtle::phase_type::NONINV;
  }
  assert(false);
  return mockturtle::phase_type::UNKNOWN;
}

} // namespace

void CellLibraryMockturtleAccess::Ensure(const CellLibrary &library) {
  if (library.pMappingCache_) {
    return;
  }
  auto cache = std::make_shared<CellLibrary::MappingCache>();
  cache->vvBindingIds.assign(library.GetNumCells(), {});
  for (int nCell = 0; nCell < library.GetNumCells(); ++nCell) {
    std::vector<std::string> vInputNames;
    vInputNames.reserve(library.GetNumInputs(nCell));
    for (int nInput = 0; nInput < library.GetNumInputs(nCell); ++nInput) {
      vInputNames.push_back(library.GetInputName(nCell, nInput));
    }
    for (int nOutput = 0; nOutput < library.GetNumOutputs(nCell); ++nOutput) {
      std::vector<mockturtle::pin> vPins;
      vPins.reserve(library.GetNumInputs(nCell));
      for (int nInput = 0; nInput < library.GetNumInputs(nCell); ++nInput) {
        const auto &arc = library.GetTimingArc(nCell, nOutput, nInput);
        vPins.push_back(
            {library.GetInputName(nCell, nInput), ConvertPinPhase(arc.phase),
             library.GetInputLoad(nCell, nInput),
             library.GetMaxLoad(nCell, nOutput), arc.dRiseBlockDelay,
             arc.dRiseFanoutDelay, arc.dFallBlockDelay, arc.dFallFanoutDelay});
      }
      const unsigned nBindingId = static_cast<unsigned>(cache->vGates.size());
      const std::string &strFunction =
          library.GetOutputFunction(nCell, nOutput);
      kitty::dynamic_truth_table tt{
          static_cast<uint32_t>(library.GetNumInputs(nCell))};
      const bool fParsed =
          kitty::create_from_formula(tt, strFunction, vInputNames);
      assert(fParsed);
      (void)fParsed;
      cache->vGates.push_back(
          {nBindingId, library.GetCellName(nCell), strFunction,
           static_cast<uint32_t>(library.GetNumInputs(nCell)), tt,
           library.GetOutputArea(nCell, nOutput), std::move(vPins),
           library.GetOutputName(nCell, nOutput)});
      cache->vvBindingIds[nCell].push_back(nBindingId);
      cache->vCellIdsByBindingId.push_back(nCell);
    }
  }
  cache->pMapping =
      std::make_unique<mockturtle::tech_library<6>>(cache->vGates);
  library.pMappingCache_ = std::move(cache);
}

const std::vector<mockturtle::gate> &
CellLibraryMockturtleAccess::Gates(const CellLibrary &library) {
  Ensure(library);
  return library.pMappingCache_->vGates;
}

const mockturtle::tech_library<6> &
CellLibraryMockturtleAccess::Mapping(const CellLibrary &library) {
  Ensure(library);
  return *library.pMappingCache_->pMapping;
}

const std::vector<std::vector<unsigned>> &
CellLibraryMockturtleAccess::BindingIds(const CellLibrary &library) {
  Ensure(library);
  return library.pMappingCache_->vvBindingIds;
}

const std::vector<int> &
CellLibraryMockturtleAccess::CellIdsByBindingId(const CellLibrary &library) {
  Ensure(library);
  return library.pMappingCache_->vCellIdsByBindingId;
}
#endif

} // namespace boop

BOOP_IMPL_END
