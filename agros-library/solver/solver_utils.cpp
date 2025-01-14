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

#include "solver_utils.h"
#include "solver.h"

#include "util/constants.h"
#include "util/sparse_io.h"

#include "field.h"
#include "problem.h"
#include "coupling.h"
#include "solutionstore.h"
#include "plugin_interface.h"
#include "plugin_interface.h"

#include "matio/matio.h"

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_dgp.h>
#include <deal.II/fe/fe_nothing.h>
#include <deal.II/fe/mapping_q.h>

ProblemSolver::ProblemSolver(Computation *parentProblem) : m_computation(parentProblem)
{
}

ProblemSolver::~ProblemSolver()
{
    clear();
}

void ProblemSolver::clear()
{
    foreach (QString fieldId, m_solverDeal.keys())
    {
        if (m_solverDeal.contains(fieldId))
            delete m_solverDeal[fieldId];

        if (m_fesCache.contains(fieldId))
            for (unsigned int i = 0; i < m_fesCache[fieldId].size(); i++)
                delete m_fesCache[fieldId][i];

        if (m_feCollectionCache.contains(fieldId))
            delete m_feCollectionCache[fieldId];

        if (m_mappingCollectionCache.contains(fieldId))
            delete m_mappingCollectionCache[fieldId];
    }
    m_solverDeal.clear();
    m_fesCache.clear();
    m_feCollectionCache.clear();
    m_mappingCollectionCache.clear();
}

void ProblemSolver::init()
{
    clear();

    foreach (FieldInfo* fieldInfo, m_computation->fieldInfos())
    {
        m_solverDeal[fieldInfo->fieldId()] = fieldInfo->plugin()->solverDeal(m_computation, fieldInfo);
    }
}

void ProblemSolver::solveProblem()
{
    QStringList fieldInfosSorted = m_computation->fieldInfos().keys();

    // sort fields (very small arrays -> sufficiently fast)
    bool swapped = false;
    do
    {
        swapped = false;
        foreach (CouplingInfo *couplingInfo, m_computation->couplingInfos().values())
        {
            if (couplingInfo->couplingType() == CouplingType_Weak)
            {
                int sourceIndex = fieldInfosSorted.indexOf(couplingInfo->sourceFieldId());
                int targetIndex = fieldInfosSorted.indexOf(couplingInfo->targetFieldId());

                if (targetIndex < sourceIndex)
                {
                    fieldInfosSorted.move(sourceIndex, 0);
                    swapped = true;
                }
            }
        }
    }
    while (swapped);

    foreach (QString targetfieldId, fieldInfosSorted)
    {
        // solver deal.II
        SolverDeal *solverDeal = m_solverDeal[targetfieldId];

        // couplings
        QStringList couplings = m_computation->fieldInfo(targetfieldId)->plugin()->couplings();

        // look for coupling sources
        foreach (QString sourceFieldId, fieldInfosSorted)
        {            
            if (couplings.contains(sourceFieldId))
            {
                FieldSolutionID solutionID(sourceFieldId,
                                           m_computation->solutionStore()->lastTimeStep(m_computation->fieldInfo(sourceFieldId)),
                                           m_computation->solutionStore()->lastAdaptiveStep(m_computation->fieldInfo(sourceFieldId)));

                solverDeal->setCouplingSource(sourceFieldId,
                                              m_computation->solutionStore()->multiArray(solutionID));
            }
        }

        solverDeal->solveProblem();
    }
}

dealii::hp::FECollection<2> *ProblemSolver::feCollection(const FieldInfo *fieldInfo)
{
    if (!m_feCollectionCache.contains(fieldInfo->fieldId()))
    {
        QMap<int, PluginModuleAnalysis::Equation> spaces;
        foreach (PluginModuleAnalysis analysis, fieldInfo->plugin()->moduleJson()->analyses)
            if (analysis.type == fieldInfo->analysisType())
                spaces = analysis.configs;
        assert(spaces.size() > 0);

        dealii::hp::FECollection<2> *feCollection = new dealii::hp::FECollection<2>();

        // first position of feCollection, quadratureFormulas and quadratureFormulasFace belongs to NONE space
        // this will be used for implementation of different meshes
        std::vector<const dealii::FiniteElement<2> *> fesSpaces;
        std::vector<unsigned int> multiplicitiesSpaces;
        foreach (int key, spaces.keys())
        {
            dealii::FiniteElement<2> *fe = new dealii::FE_Nothing<2>();
            fesSpaces.push_back(fe);
            m_fesCache[fieldInfo->fieldId()].push_back(fe);
            multiplicitiesSpaces.push_back(1);
        }
        feCollection->push_back(dealii::FESystem<2>(fesSpaces, multiplicitiesSpaces));

        // fe collections
        for (unsigned int degree = 1; degree <= DEALII_MAX_ORDER + 1; degree++)
        {
            std::vector<const dealii::FiniteElement<2> *> fes;
            std::vector<unsigned int> multiplicities;

            foreach (int key, spaces.keys())
            {
                dealii::FiniteElement<2> *fe = nullptr;
                PluginModuleAnalysis::Equation equation = spaces[key];

                if (equation.type == "h1")
                    fe = new dealii::FE_Q<2>(degree + equation.orderIncrease);
                else if (equation.type == "l2")
                    fe = new dealii::FE_DGP<2>(degree + equation.orderIncrease);

                fes.push_back(fe);
                m_fesCache[fieldInfo->fieldId()].push_back(fe);
                multiplicities.push_back(1);
            }

            feCollection->push_back(dealii::FESystem<2>(fes, multiplicities));
        }

        m_feCollectionCache[fieldInfo->fieldId()] = feCollection;
    }

    return m_feCollectionCache[fieldInfo->fieldId()];
}

dealii::hp::MappingCollection<2> *ProblemSolver::mappingCollection(const FieldInfo *fieldInfo)
{
    if (!m_mappingCollectionCache.contains(fieldInfo->fieldId()))
    {
        dealii::hp::MappingCollection<2> *mappingCollection = new dealii::hp::MappingCollection<2>();

        for (unsigned int degree = 1; degree <= DEALII_MAX_ORDER + 1; degree++)
            mappingCollection->push_back(dealii::MappingQ<2>(1, true));

        m_mappingCollectionCache[fieldInfo->fieldId()] = mappingCollection;
    }

    return m_mappingCollectionCache[fieldInfo->fieldId()];
}
