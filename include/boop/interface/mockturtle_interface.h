#pragma once

#include <string>

#include "boop/config.h"
#include "boop/network/bound_network.h"

BOOP_HEADER_START

namespace boop {

template <typename Ntk, typename Rng>
extern std::string MockturtlePerformLocal(Ntk *pNtk, Rng &rng);

template <typename Ntk, typename Library>
bool MockturtleMap(const Ntk *pNtk, const Library &library,
                   BoundNetwork *pMapped);

} // namespace boop

BOOP_HEADER_END
