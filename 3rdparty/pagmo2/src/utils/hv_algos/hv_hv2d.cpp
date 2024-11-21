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

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <pagmo/exceptions.hpp>
#include <pagmo/types.hpp>
#include <pagmo/utils/hv_algos/hv_algorithm.hpp>
#include <pagmo/utils/hv_algos/hv_hv2d.hpp>
#include <pagmo/utils/hv_algos/hv_hv3d.hpp>

namespace pagmo
{

// Constructor
hv2d::hv2d(const bool initial_sorting) : m_initial_sorting(initial_sorting) {}

/// Compute hypervolume method.
/**
 * This method should be used both as a solution to 2-dimensional cases, and as a general termination method for
 * algorithms that reduce n-dimensional problem to 2D.
 *
 * Computational complexity: n*log(n)
 *
 * @param points vector of points containing the 2-dimensional points for which we compute the hypervolume
 * @param r_point reference point for the points
 *
 * @return hypervolume
 */
double hv2d::compute(std::vector<vector_double> &points, const vector_double &r_point) const
{
    if (points.size() == 0u) {
        return 0.0;
    } else if (points.size() == 1u) {
        return hv_algorithm::volume_between(points[0], r_point);
    }

    if (m_initial_sorting) {
        std::sort(points.begin(), points.end(),
                  [](const vector_double &v1, const vector_double &v2) { return v1[1] < v2[1]; });
    }

    double hypervolume = 0.0;

    // width of the sweeping line
    double w = r_point[0] - points[0][0];
    for (decltype(points.size()) idx = 0u; idx < points.size() - 1u; ++idx) {
        hypervolume += (points[idx + 1u][1] - points[idx][1]) * w;
        w = std::max(w, r_point[0] - points[idx + 1u][0]);
    }
    hypervolume += (r_point[1] - points[points.size() - 1u][1]) * w;

    return hypervolume;
}

/// Compute hypervolume method.
/**
 * This method should be used both as a solution to 2-dimensional cases, and as a general termination method for
 * algorithms that reduce n-dimensional problem to 2d.
 * This method is overloaded to work with arrays of double, in order to provide other algorithms that internally
 * work with arrays (such as hv_algorithm::wfg) with an efficient computation.
 *
 * Computational complexity: n*log(n)
 *
 * @param points array of 2-dimensional points
 * @param n_points number of points
 * @param r_point 2-dimensional reference point for the points
 *
 * @return hypervolume
 */
double hv2d::compute(double **points, vector_double::size_type n_points, double *r_point) const
{
    if (n_points == 0u) {
        return 0.0;
    } else if (n_points == 1u) {
        return volume_between(points[0], r_point, 2);
    }

    if (m_initial_sorting) {
        std::sort(points, points + n_points, [](double *a, double *b) { return a[1] < b[1]; });
    }

    double hypervolume = 0.0;

    // width of the sweeping line
    double w = r_point[0] - points[0][0];
    for (decltype(n_points) idx = 0; idx < n_points - 1u; ++idx) {
        hypervolume += (points[idx + 1u][1] - points[idx][1]) * w;
        w = std::max(w, r_point[0] - points[idx + 1u][0]);
    }
    hypervolume += (r_point[1] - points[n_points - 1u][1]) * w;

    return hypervolume;
}

/// Contributions method
/**
 * Computes the contributions of each point by invoking the HV3D algorithm with mock third dimension.
 *
 * @param points vector of points containing the 2-dimensional points for which we compute the hypervolume
 * @param r_point reference point for the points
 * @return vector of exclusive contributions by every point
 */
std::vector<double> hv2d::contributions(std::vector<vector_double> &points, const vector_double &r_point) const
{
    std::vector<vector_double> new_points(points.size(), vector_double(3, 0.0));
    vector_double new_r(r_point);
    new_r.push_back(1.0);

    for (decltype(points.size()) i = 0u; i < points.size(); ++i) {
        new_points[i][0] = points[i][0];
        new_points[i][1] = points[i][1];
        new_points[i][2] = 0.0;
    }
    // Set sorting to off since contributions are sorted by third dimension
    return hv3d(false).contributions(new_points, new_r);
}

/// Clone method.
/**
 * @return a pointer to a new object cloning this
 */
std::shared_ptr<hv_algorithm> hv2d::clone() const
{
    return std::shared_ptr<hv_algorithm>(new hv2d(*this));
}

/// Verify input method.
/**
 * Verifies whether the requested data suits the hv2d algorithm.
 *
 * @param points vector of points containing the d dimensional points for which we compute the hypervolume
 * @param r_point reference point for the vector of points
 *
 * @throws value_error when trying to compute the hypervolume for the dimension other than 3 or non-maximal
 * reference point
 */
void hv2d::verify_before_compute(const std::vector<vector_double> &points, const vector_double &r_point) const
{
    if (r_point.size() != 2u) {
        pagmo_throw(std::invalid_argument, "Algorithm hv2d works only for 2-dimensional cases.");
    }

    hv_algorithm::assert_minimisation(points, r_point);
}

/// Algorithm name
/**
 * @return The name of this particular algorithm
 */
std::string hv2d::get_name() const
{
    return "hv2d algorithm";
}

} // namespace pagmo
