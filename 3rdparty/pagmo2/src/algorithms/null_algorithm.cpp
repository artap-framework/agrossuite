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

#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/null_algorithm.hpp>
#include <pagmo/population.hpp>
#include <pagmo/s11n.hpp>

namespace pagmo
{

/// Evolve method.
/**
 * In the null algorithm, the evolve method just returns the input
 * population.
 *
 * @param pop input population.
 *
 * @return a copy of the input population.
 */
population null_algorithm::evolve(const population &pop) const
{
    return pop;
}

// Serialization support.
template <typename Archive>
void null_algorithm::serialize(Archive &, unsigned)
{
}

} // namespace pagmo

PAGMO_S11N_ALGORITHM_IMPLEMENT(pagmo::null_algorithm)
