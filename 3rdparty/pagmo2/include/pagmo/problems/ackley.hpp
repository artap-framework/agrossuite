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

#ifndef PAGMO_PROBLEMS_ACKLEY_HPP
#define PAGMO_PROBLEMS_ACKLEY_HPP

#include <string>
#include <utility>

#include <pagmo/detail/visibility.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/types.hpp>

namespace pagmo
{

/// The Ackley problem.
/**
 *
 * \image html ackley.png "Two-dimensional Ackley function." width=3cm
 *
 * This is a scalable box-constrained continuous single-objective problem.
 * The objective function is the generalised n-dimensional Ackley function:
 * \f[
 * 	F\left(x_1,\ldots,x_n\right) = 20 + e - 20e^{-\frac 15 \sqrt{\frac 1n \sum_{i=1}^n x_i^2}} - e^{\frac 1n
 * \sum_{i=1}^n \cos(2\pi x_i)}, \quad x_i \in \left[ -15,30 \right].
 * \f]
 * The global minimum is in \f$x_i=0\f$, where \f$ F\left( 0,\ldots,0 \right) = 0 \f$.
 */
struct PAGMO_DLL_PUBLIC ackley {
    /// Constructor from dimension
    /**
     * Constructs an Ackley problem
     *
     * @param dim the problem dimensions.
     *
     * @throw std::invalid_argument if \p dim is < 1
     */
    ackley(unsigned dim = 1u);
    // Fitness computation
    vector_double fitness(const vector_double &) const;
    // Box-bounds
    std::pair<vector_double, vector_double> get_bounds() const;
    /// Problem name
    /**
     * @return a string containing the problem name
     */
    std::string get_name() const
    {
        return "Ackley Function";
    }
    // Optimal solution
    vector_double best_known() const;
    /// Problem dimensions
    unsigned m_dim;

private:
    // Object serialization
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &, unsigned);
};

} // namespace pagmo

PAGMO_S11N_PROBLEM_EXPORT_KEY(pagmo::ackley)

#endif
