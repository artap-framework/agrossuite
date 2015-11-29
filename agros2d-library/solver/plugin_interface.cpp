#include "plugin_interface.h"
#include "field.h"
#include "util/global.h"

IntegralValue::IntegralScratchData::IntegralScratchData(const dealii::hp::FECollection<2> &feCollection,
                                                        const dealii::hp::QCollection<2> &quadratureFormulas,
                                                        const dealii::hp::QCollection<2-1> &faceQuadratureFormulas)
    :
      hp_fe_values(feCollection,
                   quadratureFormulas,
                   dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_JxW_values),
      hp_fe_face_values(feCollection,
                        faceQuadratureFormulas,
                        dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_normal_vectors | dealii::update_JxW_values)
{}

IntegralValue::IntegralScratchData::IntegralScratchData(const IntegralScratchData &scratch_data)
    :
      hp_fe_values(scratch_data.hp_fe_values.get_fe_collection(),
                   scratch_data.hp_fe_values.get_quadrature_collection(),
                   dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_JxW_values),
      hp_fe_face_values(scratch_data.hp_fe_face_values.get_fe_collection(),
                        scratch_data.hp_fe_face_values.get_quadrature_collection(),
                        dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_normal_vectors | dealii::update_JxW_values)
{}
