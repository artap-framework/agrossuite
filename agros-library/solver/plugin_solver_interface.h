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

#ifndef PLUGINSOLVERINTERFACE_H
#define PLUGINSOLVERINTERFACE_H

// Windows DLL export/import definitions
#ifdef Q_WS_WIN
// windows
// DLL build
#ifdef AGROS_LIBRARY_DLL
#define AGROS_LIBRARY_API __declspec(dllexport)
// DLL usage
#else
#define AGROS_LIBRARY_API __declspec(dllimport)
#endif
#else
// linux
#define AGROS_LIBRARY_API
#endif

#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/vector.h>
#include <QtPlugin>

class AGROS_LIBRARY_API PluginSolverInterface
{
public:
    PluginSolverInterface();
    virtual ~PluginSolverInterface();

    virtual QString name() const = 0;
    virtual QStringList methods() const;
    void setMethod(const QString &meth) { method = meth; }
    void setParameters(const QString &params) { parameters = params; }

    virtual void solve(dealii::SparseMatrix<double> &system,
                       dealii::Vector<double> &rhs,
                       dealii::Vector<double> &sln) = 0;

protected:
    std::vector<int> Ap;
    std::vector<int> Ai;
    std::vector<double> Ax;

    QString method;
    QString parameters;

    void prepare_crs(const dealii::SparseMatrix<double> &matrix);
    void sort_arrays(const dealii::SparseMatrix<double> &matrix);
};

QT_BEGIN_NAMESPACE
#define PluginSolverInterface_IID "org.agros.PluginSolverInterface"
Q_DECLARE_INTERFACE(PluginSolverInterface,
                    PluginSolverInterface_IID)
QT_END_NAMESPACE

#endif // PLUGINSOLVERINTERFACE_H
