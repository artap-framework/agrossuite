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

#include "mesh/agros_manifold.h"
#include <deal.II/base/std_cxx14/memory.h>

template <int dim, int spacedim>
AgrosManifold<dim, spacedim>::AgrosManifold() : dealii::Manifold<dim, spacedim>()
{

}

template <int dim, int spacedim>
AgrosManifoldSurface<dim, spacedim>::AgrosManifoldSurface(int marker, Point center, double radius) : AgrosManifold<dim, spacedim>(), center(center), radius(radius)
{
}

template <int dim, int spacedim>
dealii::Point<spacedim> AgrosManifoldSurface<dim, spacedim>::project_to_manifold(const std::vector<dealii::Point<spacedim> > &surrounding_points, const dealii::Point<spacedim> &candidate) const
{
    Point p = Point(candidate(0), candidate(1));
    Point c = this->center;
    double r = this->radius;

    Point result = prolong_point_to_arc(p, c, r);
    return dealii::Point<2>(result.x, result.y);
}

template <int dim, int spacedim>
std::unique_ptr<dealii::Manifold<dim, spacedim> > AgrosManifoldSurface<dim, spacedim>::clone() const
{
    return dealii::std_cxx14::make_unique<AgrosManifoldSurface<dim,spacedim> >(marker, center, radius);
}

template <int dim, int spacedim>
AgrosManifoldVolume<dim, spacedim>::AgrosManifoldVolume(int element_i, AgrosManifoldSurface<dim, spacedim>* first_manifold) : element_i(element_i)
{
    this->surfManifolds.push_back(first_manifold);
    // std::cout << "------------ constructor ------------" << std::endl;
    // std::cout << "element_i: " << element_i << std::endl;
    // std::cout << "-----------------------------------" << std::endl;
}

template <int dim, int spacedim>
void AgrosManifoldVolume<dim, spacedim>::push_surfManifold(AgrosManifoldSurface<dim, spacedim>* next_manifold)
{
    this->surfManifolds.push_back(next_manifold);
}

template <int dim, int spacedim>
std::unique_ptr<dealii::Manifold<dim, spacedim> > AgrosManifoldVolume<dim, spacedim>::clone() const
{
    return dealii::std_cxx14::make_unique<AgrosManifoldVolume<dim,spacedim> >(element_i, surfManifolds.front());
}

template <int dim, int spacedim>
dealii::Point<spacedim> AgrosManifoldVolume<dim, spacedim>::project_to_manifold(const std::vector<dealii::Point<spacedim> > &surrounding_points, const dealii::Point<spacedim> &candidate) const
{
    return candidate;
}

template class AgrosManifoldVolume < 2, 2 > ;
template class AgrosManifoldSurface < 2, 2 > ;
