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

#include "solver.h"
#include "linear_solver.h"
#include "estimators.h"

#include "util.h"
#include "util/global.h"
#include "util/constants.h"

#include "field.h"
#include "problem.h"
#include "solver/problem_config.h"
//#include "module.h"
#include "coupling.h"
#include "scene.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "solutionstore.h"
#include "plugin_interface.h"
#include "logview.h"
#include "plugin_interface.h"
#include "weak_form.h"
#include "bdf2.h"

#include "pythonlab/pythonengine.h"

QMap<QString, SolverDeal *> ProblemSolver::m_solverDeal;

ProblemSolver::ProblemSolver()
{
}

void ProblemSolver::clear()
{
    foreach (QString fieldId, m_solverDeal.keys())
        if (m_solverDeal[fieldId])
            delete m_solverDeal[fieldId];
    m_solverDeal.clear();
}

void ProblemSolver::init()
{
    clear();

    foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        m_solverDeal[fieldInfo->fieldId()] = fieldInfo->plugin()->solverDeal(fieldInfo);
    }
}

void ProblemSolver::solveProblem()
{
    QList<FieldInfo *> fieldInfosSorted = Agros2D::problem()->fieldInfos().values();

    // sort fields (very small arrays -> sufficiently fast)
    bool swapped = false;
    do
    {
        swapped = false;
        foreach (CouplingInfo *couplingInfo, Agros2D::problem()->couplingInfos().values())
        {
            if (couplingInfo->couplingType() == CouplingType_Weak)
            {
                int sourceIndex = fieldInfosSorted.indexOf(couplingInfo->sourceField());
                int targetIndex = fieldInfosSorted.indexOf(couplingInfo->targetField());

                if (targetIndex < sourceIndex)
                {
                    fieldInfosSorted.move(sourceIndex, 0);
                    swapped = true;
                }
            }
        }
    }
    while (swapped);

    foreach (FieldInfo* targetfieldInfo, fieldInfosSorted)
    {
        // frequency
        // TODO: find some better place, where some values are initialized
        targetfieldInfo->setFrequency(Agros2D::problem()->config()->value(ProblemConfig::Frequency).toDouble());

        SolverDeal *solverDeal = m_solverDeal[targetfieldInfo->fieldId()];

        // look for coupling sources
        foreach (FieldInfo* sourceFieldInfo, fieldInfosSorted)
        {
            if (Agros2D::problem()->hasCoupling(sourceFieldInfo, targetfieldInfo))
            {
                FieldSolutionID solutionID(sourceFieldInfo,
                                           Agros2D::solutionStore()->lastTimeStep(sourceFieldInfo),
                                           Agros2D::solutionStore()->lastAdaptiveStep(sourceFieldInfo));

                MultiArray sourceSolution = Agros2D::solutionStore()->multiArray(solutionID);
                solverDeal->setCouplingSource(sourceFieldInfo->fieldId(), sourceSolution);
            }
        }

        solverDeal->solveProblem();
    }
}
