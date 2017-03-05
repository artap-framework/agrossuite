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

#ifndef AGROS_MANIFOLD_H
#define AGROS_MANIFOLD_H

#include "deal.II/grid/manifold.h"
#include "deal.II/grid/tria.h"
#include "util/util.h"
#include "util/point.h"

template <int dim, int spacedim = dim>
class AgrosManifold : public dealii::Manifold < dim, spacedim >
{
public:
    AgrosManifold();

};

template <int dim, int spacedim = dim>
class AgrosManifoldSurface : public AgrosManifold < dim, spacedim >
{
public:
    AgrosManifoldSurface(int marker, Point center, double radius);

    virtual dealii::Point<spacedim> project_to_manifold(const std::vector<dealii::Point<spacedim> > &surrounding_points, const dealii::Point<spacedim> &candidate) const;
private:
    int marker;
    Point center;
    double radius;
};

template <int dim, int spacedim = dim>
class AgrosManifoldVolume : public AgrosManifold < dim, spacedim >
{
public:
    AgrosManifoldVolume(int element_i, AgrosManifoldSurface<dim, spacedim>* first_manifold);

    virtual dealii::Point<spacedim> project_to_manifold(const std::vector<dealii::Point<spacedim> > &surrounding_points, const dealii::Point<spacedim> &candidate) const;

    void push_surfManifold(AgrosManifoldSurface<dim, spacedim>* next_manifold);
private:
    std::vector<AgrosManifoldSurface<dim, spacedim>*> surfManifolds;
    int element_i;
};


#endif // AGROS_MANIFOLD_H
