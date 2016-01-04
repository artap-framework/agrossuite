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

#include "study_sweep.h"

#include "study.h"
#include "parameter.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

const QString COMPUTATIONS = "computations";

StudySweepAnalysis::StudySweepAnalysis() : Study()
{
}

void StudySweepAnalysis::solve()
{
    // parameter space
    ParameterSpace space = ParameterSpace(m_parameters);
    space.random(10);

    foreach (ParametersType set, space.sets())
    {
        // computation
        QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(true, false);
        m_computations.append(computation);

        foreach (QString parameter, set.keys())
            computation->config()->setParameter(parameter, set[parameter]);

        // solve and evaluate
        computation->solve();
        foreach (Functional functional, m_functionals)
            bool successfulRun = functional.evaluateExpression(computation);

        computation->saveResults();
    }
}

void StudySweepAnalysis::load(QJsonObject &object)
{
    // computations
    QJsonArray computationsJson = object[COMPUTATIONS].toArray();
    for (int i = 0; i < computationsJson.size(); i++)
    {
        QMap<QString, QSharedPointer<Computation> > computations = Agros2D::computations();
        m_computations.append(computations[computationsJson[i].toString()]);
    }

    Study::load(object);
}

void StudySweepAnalysis::save(QJsonObject &object)
{
    // computations
    QJsonArray computationsJson;
    foreach (QSharedPointer<Computation> computation, m_computations)
    {
        computationsJson.append(computation->problemDir());
    }
    object[COMPUTATIONS] = computationsJson;

    Study::save(object);
}

// gui
void StudySweepAnalysis::fillTreeView(QTreeWidget *trvComputations)
{
    foreach (QSharedPointer<Computation> computation, m_computations)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(trvComputations);
        item->setText(0, computation->problemDir());
        item->setText(1, (computation->isSolved() ? tr("solved") : "-"));
        item->setData(0, Qt::UserRole, computation->problemDir());
    }
}
