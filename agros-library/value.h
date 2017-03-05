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

#ifndef VALUE_H
#define VALUE_H

#include "util/util.h"
#include "util/point.h"
#include "solver/module.h"

#include "datatable.h"

class DataTable;
class FieldInfo;
class ProblemBase;

class AGROS_LIBRARY_API Value
{
public:
    Value();

    Value(ProblemBase *problem,
          const double value = 0.0);

    Value(ProblemBase *problem,
          const QString &value,
          const DataTable &table = DataTable());

    Value(ProblemBase *problem,
          const QString &value,
          std::vector<double> x,
          std::vector<double> y,
          DataTableType type = DataTableType_PiecewiseLinear,
          bool splineFirstDerivatives = true,
          bool extrapolateConstant = true);

    Value(const Value& origin);
    Value& operator=(const Value& origin);

    ~Value();

    // expression
    void setNumber(double value);
    double number() const;
    double numberAtPoint(const Point &point) const;
    double numberAtTime(const double time) const;
    double numberAtTimeAndPoint(const double time, const Point &point) const;

    bool isNumber() const;
    bool isEvaluated() const;
    inline QString error() const { return m_error; }
    inline bool isTimeDependent() const { return m_isTimeDependent; }
    inline bool isCoordinateDependent() const { return m_isCoordinateDependent; }

    // table
    double numberFromTable(double key) const;
    double derivativeFromTable(double key) const;

    bool hasTable() const;

    void setText(const QString &str);
    inline QString text() const { return m_text; }

    QString toString() const;
    void parseFromString(const QString &str);

    inline DataTable table() const { return m_table; }

    inline ProblemBase *problem() const { return m_problem; }

private:
    // problem
    ProblemBase *m_problem;

    // expression
    double m_number;
    QString m_text;
    bool m_isTimeDependent;
    bool m_isCoordinateDependent;

    mutable exprtk::expression<double> *m_exprtkExpr;
    mutable bool m_isEvaluated;
    mutable QString m_error;

    // table
    DataTable m_table;

    void lexicalAnalysis();

    friend class ValueLineEdit;
    friend class ValueTimeDialog;
    friend class PointValue;
};

class AGROS_LIBRARY_API PointValue
{
public:
    PointValue(ProblemBase *problem = nullptr, const Point &point = Point());
    PointValue(const Value &x, const Value &y);

    // PointValue(const PointValue& origin);
    PointValue& operator=(const PointValue &origin);

    void setPoint(double x, double y);
    void setPoint(const Point &point);
    void setPoint(const Value &x, const Value &y);
    void setPoint(const QString &x, const QString &y);

    inline Point point() const { return Point(m_x.number(), m_y.number()); }
    inline Value x() const { return m_x; }
    inline Value &x() { return m_x; }
    inline Value y() const { return m_y; }
    inline Value &y() { return m_y; }

    inline double numberX() { return m_x.number(); }
    inline double numberY() { return m_y.number(); }

    QString toString() const;

private:
    Value m_x;
    Value m_y;
};

Q_DECLARE_METATYPE(Value)

#endif // VALUE_H
