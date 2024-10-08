// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef __AGROS_SPLINE_H
#define __AGROS_SPLINE_H

#include "util/util.h"

class AGROS_LIBRARY_API CubicSpline
{
public:
    struct AGROS_LIBRARY_API SplineCoeff
    {
      double a, b, c, d;    // four coefficients of a cubic spline.
    };

    /// Constructor (general case).
    CubicSpline(std::vector<double> points, std::vector<double> values,
                double bc_left, double bc_right,
                bool first_der_left = true, bool first_der_right = true,
                bool extend_der_left = true, bool extend_der_right = true);

    CubicSpline(double const_value);

    /// Destructor.
    ~CubicSpline();

    void free();

    /// Calculates coefficients.
    void calculate_coeffs();

    /// One-dimensional function value.
    double value(double x) const;

    /// One-dimensional function derivative value.
    double derivative(double x) const;

protected:
    /// Uses a bisection method to locale interval where a given point lies.
    /// Returns false if point lies outside.
    bool find_interval(double x_in, int& m) const;

    /// Extrapolate the value of the spline outside of its interval of definition.
    double extrapolate_value(double point_end, double value_end, double derivative_end, double x_in) const;
    /// Grid points, ordered.
    std::vector<double> points;

    /// Values at the grid points.
    std::vector<double> values;

    /// Boundary conditions.
    double bc_left, bc_right;

    /// Flags that determine the meaning of the boundary constants.
    /// first_der_left == true means that the left BC is the first derivative.
    /// first_der_left == false means that the left BC is the second derivative.
    /// (same on the right)
    bool first_der_left, first_der_right;
    /// If extend_der_left == true then the spline is extended to the left of the
    /// interval of definition as a linear function whose slope matches the derivative
    /// at the left-most point. Otherwise the spline is extended as a constant value
    /// that matches the spline value at the left-most point. Analogously for the
    /// flag extend_der_right.
    bool extrapolate_der_left, extrapolate_der_right;

    /// Values and derivatives at end points for extrapolation purposes.
    double point_left, value_left, derivative_left;
    double point_right, value_right, derivative_right;

    /// A set of four coefficients a, b, c, d for an elementary cubic spline.
    std::vector<SplineCoeff> coeffs;

    /// Gets derivative at a point that lies in interval 'm'.
    double get_derivative_from_interval(double x_in, int m) const;

    /// Gets value at a point that lies in interval 'm'.
    double get_value_from_interval(double x_in, int m) const;

    bool is_const;
    double const_value;
};

#endif
