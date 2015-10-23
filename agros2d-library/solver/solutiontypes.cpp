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

#include "solutiontypes.h"

#include "util/global.h"

#include "scene.h"
#include "field.h"
#include "problem.h"
#include "logview.h"

QString FieldSolutionID::toString()
{
    QString str = QString("%1_%2_%3").
            arg(fieldId).
            arg(timeStep).
            arg(adaptivityStep);

    return str;
}

// *********************************************************************************************

MultiArray::MultiArray() : m_triangulation(nullptr), m_doFHandler(nullptr)
{
}

MultiArray::MultiArray(dealii::hp::DoFHandler<2> *doFHandler,
                       dealii::Vector<double> &solution)
    : m_triangulation(nullptr), // &Agros2D::problem()->calculationMesh()
      m_doFHandler(doFHandler),
      m_solution(solution)
{
}

MultiArray::~MultiArray()
{
    // clear must be called explicitely
}

void MultiArray::clear()
{        
    // if (&Agros2D::problem()->calculationMesh() != m_triangulation)
    if (m_triangulation)
        delete m_triangulation;
    delete m_doFHandler;
}

void MultiArray::append(dealii::Triangulation<2> *triangulation, dealii::hp::DoFHandler<2> *doFHandler, dealii::Vector<double> &solution)
{    
    m_triangulation = triangulation;
    m_doFHandler = doFHandler;
    m_solution = solution;
}
