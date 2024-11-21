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

#include <cmath>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <pagmo/detail/constants.hpp>
#include <pagmo/exceptions.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/problems/rastrigin.hpp>
#include <pagmo/s11n.hpp>
#include <pagmo/types.hpp>

namespace pagmo
{

rastrigin::rastrigin(unsigned dim) : m_dim(dim)
{
    if (dim < 1u) {
        pagmo_throw(std::invalid_argument,
                    "Rastrigin Function must have minimum 1 dimension, " + std::to_string(dim) + " requested");
    }
}

/// Fitness computation
/**
 * Computes the fitness for this UDP
 *
 * @param x the decision vector.
 *
 * @return the fitness of \p x.
 */
vector_double rastrigin::fitness(const vector_double &x) const
{
    vector_double f(1, 0.);
    const auto omega = 2. * pagmo::detail::pi();
    auto n = x.size();
    for (decltype(n) i = 0u; i < n; ++i) {
        f[0] += x[i] * x[i] - 10. * std::cos(omega * x[i]);
    }
    f[0] += 10. * static_cast<double>(n);
    return f;
}

/// Box-bounds
/**
 * It returns the box-bounds for this UDP.
 *
 * @return the lower and upper bounds for each of the decision vector components
 */
std::pair<vector_double, vector_double> rastrigin::get_bounds() const
{
    vector_double lb(m_dim, -5.12);
    vector_double ub(m_dim, 5.12);
    return {lb, ub};
}

/// Gradients
/**
 * It returns the fitness gradient for this UDP.
 *
 * The gradient is represented in a sparse form as required by
 * problem::gradient().
 *
 * @param x the decision vector.
 *
 * @return the gradient of the fitness function
 */
vector_double rastrigin::gradient(const vector_double &x) const
{
    auto n = x.size();
    vector_double g(n);
    const auto omega = 2. * pagmo::detail::pi();
    for (decltype(n) i = 0u; i < n; ++i) {
        g[i] = 2 * x[i] + 10.0 * omega * std::sin(omega * x[i]);
    }
    return g;
}

/// Hessians
/**
 * It returns the hessians for this UDP.
 *
 * The hessians are represented in a sparse form as required by
 * problem::hessians().
 *
 * @param x the decision vector.
 *
 * @return the hessians of the fitness function
 */
std::vector<vector_double> rastrigin::hessians(const vector_double &x) const
{
    auto n = x.size();
    vector_double h(n);
    const auto omega = 2. * pagmo::detail::pi();
    for (decltype(n) i = 0u; i < n; ++i) {
        h[i] = 2 + 10.0 * omega * omega * std::cos(omega * x[i]);
    }
    return {h};
}

/// Hessians sparsity (only the diagonal elements are non zero)
/**
 * It returns the hessian sparisty structure for this UDP.
 *
 * The hessian sparisty is represented in the form required by
 * problem::hessians_sparsity().
 *
 * @return the hessians of the fitness function
 */
std::vector<sparsity_pattern> rastrigin::hessians_sparsity() const
{
    sparsity_pattern hs;
    auto n = m_dim;
    for (decltype(n) i = 0u; i < n; ++i) {
        hs.push_back({i, i});
    }
    return {hs};
}

/// Optimal solution
/**
 * @return the decision vector corresponding to the best solution for this problem.
 */
vector_double rastrigin::best_known() const
{
    return vector_double(m_dim, 0.);
}

// Object serialization
template <typename Archive>
void rastrigin::serialize(Archive &ar, unsigned)
{
    ar &m_dim;
}

} // namespace pagmo

PAGMO_S11N_PROBLEM_IMPLEMENT(pagmo::rastrigin)
