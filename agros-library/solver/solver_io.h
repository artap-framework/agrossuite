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

#ifndef SOLVER_IO_H
#define SOLVER_IO_H

#include <QtCore>

#undef signals
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/sparse_direct.h>
#include <deal.II/numerics/solution_transfer.h>

#define signals public

bool readMatioVector(dealii::Vector<double> &vec, const QString &name, const QString &varName);
void writeMatioVector(dealii::Vector<double> &vec, const QString &name, const QString &varName);
void writeMatioMatrix(dealii::SparseMatrix<double> &mtx, const QString &name, const QString &varName);
void writeMatioMatrix(std::vector<dealii::Vector<double> > vecs, const QString &name, const QString &varName);
void writeMatioMatrix(std::vector<dealii::Vector<int> > vecs, const QString &name, const QString &varName);

#endif // SOLVER_UTILS_H
