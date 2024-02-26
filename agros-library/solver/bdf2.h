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

#ifndef BDF2_H
#define BDF2_H

#include "util/util.h"
#include "plugin_interface.h"

class Material;

// todo: zrychlit, odebrat asserty, uchovavat rovnou alpha/gamma, atd

class AGROS_LIBRARY_API BDF2Table
{
public:
    BDF2Table() : m_order(0) {}
    virtual ~BDF2Table() {}

    void setOrderAndPreviousSteps(int order, QList<double> previousStepsLengths);
    int order() const { return m_order;}

    inline double matrixFormCoefficient() const { return m_alpha[0]; }
    inline double vectorFormCoefficient(int order) const { return m_alpha[order + 1]; }

    static void test(bool varyLength = false);

protected:
    double testCalcValue(double step, QList<double> values, double fVal);

    virtual void recalculate() = 0;

    int m_order;
    double th[10];
    double m_alpha[10];
};

class AGROS_LIBRARY_API BDF2ATable : public BDF2Table
{
public:
    BDF2ATable() : BDF2Table() { setOrderAndPreviousSteps(1, QList<double>()); }
protected:
    virtual void recalculate();
};

#endif // BDF2_H
