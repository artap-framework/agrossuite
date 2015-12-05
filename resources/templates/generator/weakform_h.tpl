// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#ifndef {{CLASS}}_WEAKFORM_H
#define {{CLASS}}_WEAKFORM_H

#include "util.h"
#include "solver/plugin_interface.h"
#include "solver/marker.h"
#include "solver/solver_utils.h"
#include "{{ID}}_interface.h"

// typedef std::tuple<typename dealii::hp::DoFHandler<2>::active_cell_iterator, typename dealii::hp::DoFHandler<2>::active_cell_iterator> IteratorTuple;

class SolverDeal{{CLASS}} : public SolverDeal
{
public:
    class AssemblyScratchData{{CLASS}} : public SolverDeal::AssemblyScratchData
    {
    public:
        AssemblyScratchData{{CLASS}}(Computation *computation,
                                        const dealii::hp::FECollection<2> &feCollection,
                                        const dealii::hp::MappingCollection<2> &mappingCollection,
                                        const dealii::hp::QCollection<2> &quadratureFormulas,
                                        const dealii::hp::QCollection<2-1> &faceQuadratureFormulas,
                                        const dealii::Vector<double> &solutionNonlinearPrevious = dealii::Vector<double>(),
                                        bool assembleMatrix = true,
                                        bool assembleRHS = true) :
            SolverDeal::AssemblyScratchData::AssemblyScratchData(feCollection, mappingCollection, quadratureFormulas, faceQuadratureFormulas,
                                                                 solutionNonlinearPrevious, assembleMatrix, assembleRHS)
            {{#COUPLING_SOURCE}}, {{COUPLING_SOURCE_ID}}_hp_fe_values(nullptr)
            {{/COUPLING_SOURCE}}
        {
            // coupling sources
            {{#COUPLING_SOURCE}}if (computation->hasField("{{COUPLING_SOURCE_ID}}"))
            {
                const SolverDeal *{{COUPLING_SOURCE_ID}}_solver = computation->problemSolver()->solver("{{COUPLING_SOURCE_ID}}");
                {{COUPLING_SOURCE_ID}}_hp_fe_values = new dealii::hp::FEValues<2>({{COUPLING_SOURCE_ID}}_solver->mappingCollection(), {{COUPLING_SOURCE_ID}}_solver->feCollection(), {{COUPLING_SOURCE_ID}}_solver->quadratureFormulas(), dealii::update_values | dealii::update_gradients);
            }
            {{/COUPLING_SOURCE}}
        }

        AssemblyScratchData{{CLASS}}(const AssemblyScratchData{{CLASS}} &scratch_data) :
            SolverDeal::AssemblyScratchData::AssemblyScratchData(scratch_data)
            {{#COUPLING_SOURCE}}, {{COUPLING_SOURCE_ID}}_hp_fe_values(nullptr)
            {{/COUPLING_SOURCE}}
        {
            {{#COUPLING_SOURCE}}if (scratch_data.{{COUPLING_SOURCE_ID}}_hp_fe_values)
            {
                {{COUPLING_SOURCE_ID}}_hp_fe_values = new dealii::hp::FEValues<2>(scratch_data.{{COUPLING_SOURCE_ID}}_hp_fe_values->get_mapping_collection(),
                                                                                  scratch_data.{{COUPLING_SOURCE_ID}}_hp_fe_values->get_fe_collection(),
                                                                                  scratch_data.{{COUPLING_SOURCE_ID}}_hp_fe_values->get_quadrature_collection(),
                                                                                  dealii::update_values | dealii::update_gradients);
            }
            {{/COUPLING_SOURCE}}
        }

        ~AssemblyScratchData{{CLASS}}()
        {
            {{#COUPLING_SOURCE}}if ({{COUPLING_SOURCE_ID}}_hp_fe_values)
                delete {{COUPLING_SOURCE_ID}}_hp_fe_values;
            {{/COUPLING_SOURCE}}
        }

        // coupling sources
        {{#COUPLING_SOURCE}}dealii::hp::FEValues<2> *{{COUPLING_SOURCE_ID}}_hp_fe_values;
        {{/COUPLING_SOURCE}}
    };

    class Assemble{{CLASS}} : public AssembleNonlinear
    {
    public:
        Assemble{{CLASS}}(Computation *computation, SolverDeal *solverDeal, dealii::Triangulation<2> &triangulation) : AssembleNonlinear(computation, solverDeal, triangulation) {}

        virtual void assembleSystem(const dealii::Vector<double> &solutionNonlinearPrevious,
                                    bool assembleMatrix = true,
                                    bool assembleRHS = true);
        virtual void assembleDirichlet(bool calculateDirichletLiftValue);

    protected:
        // virtual void localAssembleSystem(const dealii::SynchronousIterators<IteratorTuple> &iter,
        virtual void localAssembleSystem(const DoubleCellIterator &iter,
                                        AssemblyScratchData{{CLASS}} &scratch,
                                        AssemblyCopyData &copy_data);
        virtual void copyLocalToGlobal(const AssemblyCopyData &copy_data);
    };

    SolverDeal{{CLASS}}(Computation *computation, const FieldInfo *fieldInfo) : SolverDeal(computation, fieldInfo) {}

    virtual shared_ptr<SolverDeal::AssembleBase> createAssembleBase(dealii::Triangulation<2> &triangulation)
    {
        return shared_ptr<SolverDeal::AssembleBase>(new SolverDeal{{CLASS}}::Assemble{{CLASS}}(m_computation, this, triangulation));
    }
};

#endif // {{CLASS}}_INTERFACE_H
