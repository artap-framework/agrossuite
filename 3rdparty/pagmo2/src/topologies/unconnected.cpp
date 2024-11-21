/* Copyright 2017-2021 PaGMO development team

This file is part of the PaGMO library.

The PaGMO library is free software; you can redistribute it and/or modify
it under the terms of either:

  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.

or

  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.

or both in parallel, as here.

The PaGMO library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the PaGMO library.  If not,
see https://www.gnu.org/licenses/. */

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include <pagmo/s11n.hpp>
#include <pagmo/topologies/unconnected.hpp>
#include <pagmo/topology.hpp>
#include <pagmo/types.hpp>

namespace pagmo
{

// Get connections (returns empty vectors).
std::pair<std::vector<std::size_t>, vector_double> unconnected::get_connections(std::size_t) const
{
    return std::make_pair(std::vector<std::size_t>{}, vector_double{});
}

// Add the next vertex (no-op).
void unconnected::push_back() {}

// Name.
std::string unconnected::get_name() const
{
    return "Unconnected";
}

// Serialization.
template <typename Archive>
void unconnected::serialize(Archive &, unsigned)
{
}

} // namespace pagmo

PAGMO_S11N_TOPOLOGY_IMPLEMENT(pagmo::unconnected)
