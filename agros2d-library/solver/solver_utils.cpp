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
#include "sparse_io.h"

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

#include "matio/matio.h"

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_dgp.h>
#include <deal.II/fe/fe_nothing.h>
#include <deal.II/fe/mapping_q.h>

void writeMatioVector(dealii::Vector<double> &vec, const QString &name, const QString &varName)
{
    size_t dims[2];
    dims[0] = vec.size();
    dims[1] = 1;

    mat_t *mat = Mat_CreateVer(name.toStdString().c_str(), "", MAT_FT_MAT5);

    double *data = new double[vec.size()];
    for (unsigned int i = 0; i < vec.size(); i++)
        data[i] = vec[i];

    matvar_t *matvar = Mat_VarCreate(varName.toStdString().c_str(), MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, data, MAT_F_DONT_COPY_DATA);

    Mat_VarWrite(mat, matvar, MAT_COMPRESSION_ZLIB);
    Mat_VarFree(matvar);
    Mat_Close(mat);

    delete [] data;
}

void writeMatioMatrix(dealii::SparseMatrix<double> &mtx, const QString &name, const QString &varName)
{
    mat_sparse_t sparse;
    sparse.nzmax = mtx.n_nonzero_elements();
    sparse.nir = mtx.n_nonzero_elements();
    sparse.njc = mtx.n() + 1;
    sparse.ndata = mtx.n_nonzero_elements();

    size_t dims[2];
    dims[0] = mtx.m();
    dims[1] = mtx.n();

    mat_t *mat = Mat_CreateVer(name.toStdString().c_str(), "", MAT_FT_MAT5);

    double *data = new double[mtx.n_nonzero_elements()];
    int *ir = new int[mtx.n_nonzero_elements()];
    int *jc = new int[mtx.n() + 1];

    int index = 0;
    int rowIndex = 0;
    for (unsigned int i = 0; i < mtx.n(); i++)
    {
        jc[rowIndex] = index + 0;
        rowIndex++;

        dealii::SparseMatrix<double>::iterator it = mtx.begin(i);
        for (; it != mtx.end(i); ++it)
        {
            if (it->is_valid_entry())
            {
                data[index] = it->value();
                ir[index] = it->column() + 0;
                index++;
            }
        }
    }

    // by convention, we define jc[n+1] = nzz + 1
    jc[mtx.n()] = mtx.n_nonzero_elements(); // indices from 0

    // transpose
    csr2csc(mtx.n(), mtx.n_nonzero_elements(), data, ir, jc);

    sparse.data = data;
    sparse.jc = jc;
    sparse.ir = ir;

    matvar_t *matvar = Mat_VarCreate(varName.toStdString().c_str(), MAT_C_SPARSE, MAT_T_DOUBLE, 2, dims, &sparse, MAT_F_DONT_COPY_DATA);

    Mat_VarWrite(mat, matvar, MAT_COMPRESSION_ZLIB);
    Mat_VarFree(matvar);
    Mat_Close(mat);

    delete [] data;
    delete [] jc;
    delete [] ir;
}

ProblemSolver::ProblemSolver(ProblemComputation *parentProblem) : m_computation(parentProblem)
{
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
    QList<FieldInfo *> fieldInfosSorted = m_computation->fieldInfos().values();

    // sort fields (very small arrays -> sufficiently fast)
    bool swapped = false;
    do
    {
        swapped = false;
        foreach (CouplingInfo *couplingInfo, m_computation->couplingInfos().values())
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
        targetfieldInfo->setFrequency(m_computation->config()->value(ProblemConfig::Frequency).value<Value>().number());

        SolverDeal *solverDeal = m_solverDeal[targetfieldInfo->fieldId()];

        // look for coupling sources
        foreach (FieldInfo* sourceFieldInfo, fieldInfosSorted)
        {
            if (m_computation->hasCoupling(sourceFieldInfo, targetfieldInfo))
            {
                FieldSolutionID solutionID(sourceFieldInfo->fieldId(),
                                           m_computation->solutionStore()->lastTimeStep(sourceFieldInfo),
                                           m_computation->solutionStore()->lastAdaptiveStep(sourceFieldInfo));

                MultiArray sourceSolution = m_computation->solutionStore()->multiArray(solutionID);
                solverDeal->setCouplingSource(sourceFieldInfo->fieldId(), sourceSolution);
            }
        }

        solverDeal->solveProblem();
    }
}

dealii::hp::FECollection<2> *ProblemSolver::feCollection(const FieldInfo *fieldInfo)
{
    if (!m_feCollectionCache.contains(fieldInfo->fieldId()))
    {
        dealii::hp::FECollection<2> *feCollection = new dealii::hp::FECollection<2>();

        // qDebug() << fieldInfo->name();
        QMap<int, Module::Space> spaces = fieldInfo->spaces();

        // first position of feCollection, quadratureFormulas and quadratureFormulasFace belongs to NONE space
        // this will be used for implementation of different meshes
        std::vector<const dealii::FiniteElement<2> *> fes;
        std::vector<unsigned int> multiplicities;
        foreach (int key, spaces.keys())
        {
            dealii::FiniteElement<2> *fe = new dealii::FE_Nothing<2>();
            fes.push_back(fe);
            m_fesCache[fieldInfo->fieldId()].push_back(fe);
            multiplicities.push_back(1);
        }
        feCollection->push_back(dealii::FESystem<2>(fes, multiplicities));

        // fe collections        
        for (unsigned int degree = 1; degree <= DEALII_MAX_ORDER + 1; degree++)
        {
            std::vector<const dealii::FiniteElement<2> *> fes;
            std::vector<unsigned int> multiplicities;

            foreach (int key, spaces.keys())
            {
                dealii::FiniteElement<2> *fe = nullptr;
                Module::Space space = spaces[key];

                if (space.type() == "h1")
                    fe = new dealii::FE_Q<2>(degree + space.orderAdjust());
                else if (spaces.value(key).type() == "l2")
                    fe = new dealii::FE_DGP<2>(degree + space.orderAdjust());
                    // fe = new dealii::FE_Q<2>(degree + space.orderAdjust());

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
