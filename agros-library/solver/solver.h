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

#ifndef SOLVER_H
#define SOLVER_H

#include "util/util.h"
#include "util/global.h"
#include "solutiontypes.h"
#include "scene.h"
#include "linear_solver.h"
#include "estimators.h"

#undef signals
#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>

#include <deal.II/hp/dof_handler.h>
#include <deal.II/hp/fe_collection.h>
#include <deal.II/hp/q_collection.h>
#include <deal.II/hp/fe_values.h>
#include <deal.II/hp/mapping_collection.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/dofs/dof_tools.h>
#include <deal.II/lac/solver.h>
#include <deal.II/lac/precondition.h>
#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/sparse_direct.h>
#include <deal.II/numerics/solution_transfer.h>
#include <deal.II/base/time_stepping.h>
#include <deal.II/base/config.h>
#define signals public

class FieldInfo;
class SceneBoundary;

class AGROS_LIBRARY_API SolverDeal
{
public:
    class AGROS_LIBRARY_API AssembleBase
    {
    public:
        AssembleBase(Computation *computation, SolverDeal *solverDeal, dealii::Triangulation<2> &triangulation);

        // current solution
        dealii::hp::DoFHandler<2> doFHandler;

        dealii::AffineConstraints<double> constraintsHangingNodes;
        dealii::AffineConstraints<double> constraintsDirichlet;
        dealii::AffineConstraints<double> constraintsZeroDirichlet;
        dealii::AffineConstraints<double> constraintsAll;
        dealii::SparsityPattern sparsityPattern;

        // matrix, rhs and solution
        dealii::SparseMatrix<double> systemMatrix;
        dealii::Vector<double> systemRHS;
        dealii::Vector<double> solution;

        // estimator
        std::shared_ptr<ErrorEstimator> errorEstimator;

        // linear solver
        SolverLinearSolver linearSolver;

        // hanging nodes constraints, Dirichlet ones and sparsity pattern.
        void recreateConstraints(bool zeroDirichletLift);

        // if useDirichletLift == false, zero dirichlet boundary condition is used this is used for later iterations of the Newton method
        // this function, however, has to be called to ensure zero dirichlet boundary
        virtual void setup(bool useDirichletLift);

        // assembling        
        virtual void assembleSystem(const dealii::Vector<double> &solutionNonlinearPrevious = dealii::Vector<double>(),
                                    bool assembleMatrix = true,
                                    bool assembleRHS = true) = 0;
        virtual void assembleDirichlet(bool calculateDirichletLiftValue) = 0;

        // BDF methods
        void transientBDF(const double timeStep,
                          dealii::Vector<double> &solution,
                          const std::vector<dealii::Vector<double> > solutions,
                          const BDF2Table &bdf2Table);
        void transientWriteSystemToDisk(std::vector<dealii::Vector<double> > vecs = std::vector<dealii::Vector<double> >());

        virtual void solve();

        void solveProblemLinear();

        // linear solver
        void solveLinearSystem(dealii::SparseMatrix<double> &system,
                               dealii::Vector<double> &rhs,
                               dealii::Vector<double> &sln,
                               bool reuseDecomposition = false);

        inline SolverDeal *solverDeal() { return m_solverDeal; }

    protected:
        // local references
        Computation *m_computation;
        const FieldInfo *m_fieldInfo;
        SolverDeal *m_solverDeal;

        // transient mass matrix
        dealii::SparseMatrix<double> transientMassMatrix;
        dealii::SparseMatrix<double> transientTotalMatrix;

        friend class WeightedResidualScratchData;
    };

    class AGROS_LIBRARY_API AssembleCache
    {
    public:
        AssembleCache() : dofs_per_cell(-1), n_q_points(-1) {}

        void clear();

        // volume value and grad cache
        std::vector<std::vector<double> > shape_value;
        std::vector<std::vector<dealii::Tensor<1,2> > > shape_grad;
        // surface cache
        std::vector<std::vector<dealii::Point<2> > > shape_face_point;
        std::vector<std::vector<std::vector<double> > > shape_face_value;
        std::vector<std::vector<double> > shape_face_JxW;

        // previous values and grads
        std::vector<dealii::Vector<double> > solution_value_previous;
        std::vector<std::vector<dealii::Tensor<1, 2> > > solution_grad_previous;

        // previous values and grads
        std::vector<std::vector<dealii::Vector<double> > > solution_value_previous_face;
        std::vector<std::vector<std::vector<dealii::Tensor<1, 2> > > > solution_grad_previous_face;

        int dofs_per_cell;
        int n_q_points;
    };

    SolverDeal(Computation *computation, const FieldInfo *fieldInfo);
    virtual ~SolverDeal();

    // solve problem
    void solveProblem();

    // steady state
    void solveSteadyState();

    // transient
    void solveTransient();
    inline void set_time(const double new_time) { m_time = new_time; }
    inline double get_time() const { return m_time; }

    // quadrature cache
    inline const dealii::hp::QCollection<2> &quadratureFormulas() const { return m_quadratureFormulas; }
    inline const dealii::hp::QCollection<2-1> &quadratureFormulasFace() const { return m_quadratureFormulasFace; }

    inline MultiArray &couplingSource(const QString &fieldID) { return m_couplingSources[fieldID]; }
    inline void setCouplingSource(const QString &fieldID, const MultiArray &sourceSolution) { m_couplingSources[fieldID] = sourceSolution; }

    // assemble base class
    virtual shared_ptr<SolverDeal::AssembleBase> createAssembleBase(dealii::Triangulation<2> &triangulation) = 0;

protected:   
    class AGROS_LIBRARY_API AssemblyScratchData
    {
    public:
        AssemblyScratchData(const dealii::hp::FECollection<2> &feCollection,
                            const dealii::hp::MappingCollection<2> &mappingCollection,
                            const dealii::hp::QCollection<2> &quadratureFormulas,
                            const dealii::hp::QCollection<2-1> &faceQuadratureFormulas,
                            const dealii::Vector<double> &solutionNonlinearPrevious = dealii::Vector<double>(),
                            bool assembleMatrix = true,
                            bool assembleRHS = true);
        AssemblyScratchData(const AssemblyScratchData &scratch_data);

        dealii::hp::FEValues<2> hp_fe_values;
        dealii::hp::FEFaceValues<2> hp_fe_face_values;

        // previous nonlinear solution
        dealii::Vector<double> solutionNonlinearPrevious;

        // conditional assembling
        bool assembleMatrix;
        bool assembleRHS;
    };

    class AGROS_LIBRARY_API AssemblyCopyData
    {
    public:
        AssemblyCopyData();

        bool isAssembled;

        dealii::FullMatrix<double> cell_matrix;
        dealii::FullMatrix<double> cell_mass_matrix; // transient
        dealii::Vector<double> cell_rhs;

        std::vector<dealii::types::global_dof_index> local_dof_indices;
    };

    // local reference
    Computation *m_computation;
    const FieldInfo *m_fieldInfo;

    // quadrature cache
    dealii::hp::QCollection<2> m_quadratureFormulas;
    dealii::hp::QCollection<2-1> m_quadratureFormulasFace;

    // weak coupling sources
    QMap<QString, MultiArray> m_couplingSources;

    // adaptivity
    void prepareGridRefinement(shared_ptr<SolverDeal::AssembleBase> primal,
                               shared_ptr<SolverDeal::AssembleBase> dual = nullptr,
                               int maxHIncrease = -1, int maxPIncrease = -1);

    // transient
    double m_time;

    friend class AssembleBase;
};

#endif // SOLVER_H
