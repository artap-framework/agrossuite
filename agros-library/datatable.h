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

#ifndef DATATABLE_H
#define DATATABLE_H

#include "util/util.h"
#include "util/enums.h"
#include "util/spline.h"

class AGROS_LIBRARY_API PiecewiseLinear
{
public:
    PiecewiseLinear(std::vector<double> points, std::vector<double> values);
    double value(double x);
    double derivative(double x);

private:
    int leftIndex(double x);

    std::vector<double> m_points;
    std::vector<double> m_values;

    std::vector<double> m_derivatives;
    int m_size;
};

// for testing.. returns average value. Simple "linearization" of the problem
class AGROS_LIBRARY_API ConstantTable
{
public:
    ConstantTable(std::vector<double> points, std::vector<double> values);
    double value(double x) const;
    double derivative(double x) const;

private:    
    double m_value;
};

class AGROS_LIBRARY_API DataTable
{
public:
    DataTable();
    DataTable(std::vector<double> points, std::vector<double> values);
    DataTable(const DataTable& origin);
    DataTable& operator=(const DataTable& origin);

    void setValues(std::vector<double> points, std::vector<double> values);
    void setValues(double *keys, double *values, int count);

    void setType(DataTableType type);
    void setSplineFirstDerivatives(bool fd);
    void setExtrapolateConstant(bool ec);

    double value(double x) const;
    double derivative(double x) const;
    inline int size() const { return m_numPoints; }
    inline bool isEmpty() const {return m_isEmpty; }
    DataTableType type() const {return m_type;}
    bool splineFirstDerivatives() const {return m_splineFirstDerivatives; }
    bool extrapolateConstant() const {return m_extrapolateConstant; }

    void clear();

    double minKey() const;
    double maxKey() const;
    double minValue() const;
    double maxValue() const;

    inline std::vector<double> pointsVector() const { return m_points; }
    inline std::vector<double> valuesVector() const { return m_values; }

    void checkTable();

    QString toString() const;
    QString toStringX() const;
    QString toStringY() const;
    QString toStringSetting() const;
    void fromString(const QString &str);

private:
    void inValidate();
    void validate();

    void setImplicit();
    void propertiesFromString(const QString &str);

    std::vector<double> m_points;
    std::vector<double> m_values;

    bool m_valid;

    DataTableType m_type;
    bool m_splineFirstDerivatives;
    bool m_extrapolateConstant;

    QSharedPointer<CubicSpline> m_spline;
    QSharedPointer<PiecewiseLinear> m_linear;
    QSharedPointer<ConstantTable> m_constant;

    // efficiency reasons
    int m_numPoints;
    bool m_isEmpty;
};


#endif // DATATABLE_H
