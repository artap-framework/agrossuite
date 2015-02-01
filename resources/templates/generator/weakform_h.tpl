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
#include "{{ID}}_interface.h"

class SolverDeal{{CLASS}} : public SolverDeal
{
public:
    SolverDeal{{CLASS}}(const FieldInfo *fieldInfo)
        : SolverDeal(fieldInfo) {}

    virtual void assembleSystem();
    virtual void assembleDirichlet(bool useDirichletLift);

    struct AssemblyScratchData
        {
            AssemblyScratchData(const dealii::hp::FECollection<2> &feCollection,
                                 dealii::hp::QCollection<2> quadratureFormulas,
                                 dealii::hp::QCollection<2-1> faceQuadratureFormulas);
            AssemblyScratchData(const AssemblyScratchData &scratch_data);

            dealii::hp::FEValues<2> hp_fe_values;
            dealii::hp::FEFaceValues<2> hp_fe_face_values;
        };

        struct AssemblyCopyData
        {
            AssemblyCopyData(const dealii::hp::FECollection<2> &feCollection,
                             dealii::hp::QCollection<2> quadratureFormulas,
                             dealii::hp::QCollection<2-1> faceQuadratureFormulas,
                             const FieldInfo *fieldInfo);

            bool isAssembled;

            dealii::FullMatrix<double> cell_matrix;
            dealii::FullMatrix<double> cell_mass_matrix; // transient
            dealii::Vector<double> cell_rhs;

            std::vector<dealii::types::global_dof_index> local_dof_indices;

            /*
            // volume value and grad cache
            std::vector<dealii::Vector<double> > shape_value;
            std::vector<std::vector<dealii::Tensor<1,2> > > shape_grad;
            // surface cache
            std::vector<std::vector<dealii::Point<2> > > shape_face_point;
            std::vector<std::vector<dealii::Vector<double> > > shape_face_value;
            std::vector<std::vector<double> > shape_face_JxW;
            // std::vector<std::vector<dealii::Tensor<1,2> > > shape_face_grad;

            // previous values and grads
            std::vector<dealii::Vector<double> > solution_value_previous;
            std::vector<std::vector<dealii::Tensor<1,2> > > solution_grad_previous;
            */
        };

    protected:
        void localAssembleSystem(const typename dealii::hp::DoFHandler<2>::active_cell_iterator &cell,
                                    AssemblyScratchData &scratch,
                                    AssemblyCopyData &copy_data);
        void copyLocalToGlobal(const AssemblyCopyData &copy_data);
};

#endif // {{CLASS}}_INTERFACE_H
