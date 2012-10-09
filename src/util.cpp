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

#ifdef WITH_UNITY
#include <unity.h>
#endif

#include "util.h"
#include "scene.h"
#include "pythonlabagros.h"
#include "style/manhattanstyle.h"

bool verbose = false;

static QHash<PhysicField, QString> physicFieldList;
static QHash<PhysicFieldVariable, QString> physicFieldVariableList;
static QHash<PhysicFieldVariableComp, QString> physicFieldVariableCompList;
static QHash<PhysicFieldBC, QString> physicFieldBCList;
static QHash<Mode, QString> modeList;
static QHash<SceneViewPostprocessorShow, QString> sceneViewPostprocessorShowList;
static QHash<AdaptivityType, QString> adaptivityTypeList;
static QHash<AnalysisType, QString> analysisTypeList;
static QHash<MeshType, QString> meshTypeList;
static QHash<LinearityType, QString> linearityTypeList;
static QHash<MatrixSolverType, QString> matrixSolverTypeList;

QString analysisTypeToStringKey(AnalysisType analysisType) { return analysisTypeList[analysisType]; }
AnalysisType analysisTypeFromStringKey(const QString &analysisType) { return analysisTypeList.key(analysisType); }

QString meshTypeToStringKey(MeshType meshType) { return meshTypeList[meshType]; }
MeshType meshTypeFromStringKey(const QString &meshType) { return meshTypeList.key(meshType); }

QString physicFieldToStringKey(PhysicField physicField) { return physicFieldList[physicField]; }
PhysicField physicFieldFromStringKey(const QString &physicField) { return physicFieldList.key(physicField); }

QString physicFieldVariableToStringKey(PhysicFieldVariable physicFieldVariable) { return physicFieldVariableList[physicFieldVariable]; }
PhysicFieldVariable physicFieldVariableFromStringKey(const QString &physicFieldVariable) { return physicFieldVariableList.key(physicFieldVariable); }

QString physicFieldVariableCompToStringKey(PhysicFieldVariableComp physicFieldVariableComp) { return physicFieldVariableCompList[physicFieldVariableComp]; }
PhysicFieldVariableComp physicFieldVariableCompFromStringKey(const QString &physicFieldVariableComp) { return physicFieldVariableCompList.key(physicFieldVariableComp); }

QString physicFieldBCToStringKey(PhysicFieldBC physicFieldBC) { return physicFieldBCList[physicFieldBC]; }
PhysicFieldBC physicFieldBCFromStringKey(const QString &physicFieldBC) { return physicFieldBCList.key(physicFieldBC); }

QString modeToStringKey(Mode mode) { return modeList[mode]; }
Mode modeFromStringKey(const QString &mode) { return modeList.key(mode); }

QString sceneViewPostprocessorShowToStringKey(SceneViewPostprocessorShow sceneViewPostprocessorShow) { return sceneViewPostprocessorShowList[sceneViewPostprocessorShow]; }
SceneViewPostprocessorShow sceneViewPostprocessorShowFromStringKey(const QString &sceneViewPostprocessorShow) { return sceneViewPostprocessorShowList.key(sceneViewPostprocessorShow); }

QString adaptivityTypeToStringKey(AdaptivityType adaptivityType) { return adaptivityTypeList[adaptivityType]; }
AdaptivityType adaptivityTypeFromStringKey(const QString &adaptivityType) { return adaptivityTypeList.key(adaptivityType); }

QString linearityTypeToStringKey(LinearityType linearityType) { return linearityTypeList[linearityType]; }
LinearityType linearityTypeFromStringKey(const QString &linearityType) { return linearityTypeList.key(linearityType); }

QString matrixSolverTypeToStringKey(MatrixSolverType matrixSolverType) { return matrixSolverTypeList[matrixSolverType]; }
MatrixSolverType matrixSolverTypeFromStringKey(const QString &matrixSolverType) { return matrixSolverTypeList.key(matrixSolverType); }

void initLists()
{
    logMessage("initLists()");

    // Analysis Type
    analysisTypeList.insert(AnalysisType_Undefined, "");
    analysisTypeList.insert(AnalysisType_SteadyState, "steadystate");
    analysisTypeList.insert(AnalysisType_Transient, "transient");
    analysisTypeList.insert(AnalysisType_Harmonic, "harmonic");

    // Mesh Type
    meshTypeList.insert(MeshType_Triangle, "triangle");
    meshTypeList.insert(MeshType_QuadFineDivision, "quad_fine_division");
    meshTypeList.insert(MeshType_QuadRoughDivision, "quad_rough_division");
    meshTypeList.insert(MeshType_QuadJoin, "quad_join");

    // PHYSICFIELD
    physicFieldList.insert(PhysicField_Undefined, "");
    physicFieldList.insert(PhysicField_General, "general");
    physicFieldList.insert(PhysicField_Electrostatic, "electrostatic");
    physicFieldList.insert(PhysicField_Current, "current");
    physicFieldList.insert(PhysicField_Heat, "heat");
    physicFieldList.insert(PhysicField_Elasticity, "elasticity");
    physicFieldList.insert(PhysicField_Magnetic, "magnetic");
    physicFieldList.insert(PhysicField_Flow, "flow");
    physicFieldList.insert(PhysicField_Acoustic, "acoustic");

    // PHYSICFIELDVARIABLE
    physicFieldVariableList.insert(PhysicFieldVariable_Undefined, "");

    physicFieldVariableList.insert(PhysicFieldVariable_Variable, "general_variable");
    physicFieldVariableList.insert(PhysicFieldVariable_General_Gradient, "general_gradient");
    physicFieldVariableList.insert(PhysicFieldVariable_General_Constant, "general_constant");

    physicFieldVariableList.insert(PhysicFieldVariable_Electrostatic_Potential, "electrostatic_potential");
    physicFieldVariableList.insert(PhysicFieldVariable_Electrostatic_ElectricField, "electrostatic_electric_field");
    physicFieldVariableList.insert(PhysicFieldVariable_Electrostatic_Displacement, "electrostatic_displacement");
    physicFieldVariableList.insert(PhysicFieldVariable_Electrostatic_EnergyDensity, "electrostatic_energy_density");
    physicFieldVariableList.insert(PhysicFieldVariable_Electrostatic_Permittivity, "electrostatic_permittivity");

    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_VectorPotentialReal, "magnetic_vector_potential_real");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_VectorPotentialImag, "magnetic_vector_potential_imag");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_VectorPotential, "magnetic_vector_potential");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_FluxDensityReal, "magnetic_flux_density_real");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_FluxDensityImag, "magnetic_flux_density_imag");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_FluxDensity, "magnetic_flux_density");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_MagneticFieldReal, "magnetic_magnetic_field_real");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_MagneticFieldImag, "magnetic_magnetic_field_imag");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_MagneticField, "magnetic_magnetic_field");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityReal, "magnetic_current_density_real");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityImag, "magnetic_current_density_imag");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensity, "magnetic_current_density");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformReal, "magnetic_current_density_induced_transform_real");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformImag, "magnetic_current_density_induced_transform_imag");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityInducedTransform, "magnetic_current_density_induced_transform");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityReal, "magnetic_current_density_induced_velocity_real");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityImag, "magnetic_current_density_induced_velocity_imag");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocity, "magnetic_current_density_induced_velocity");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityTotalReal, "magnetic_current_density_total_real");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityTotalImag, "magnetic_current_density_total_imag");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_CurrentDensityTotal, "magnetic_current_density_total");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_PowerLosses, "magnetic_power_losses");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_LorentzForce, "magnetic_lorentz_force");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_EnergyDensity, "magnetic_energy_density");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_Permeability, "magnetic_permeability");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_Conductivity, "magnetic_conductivity");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_Remanence, "magnetic_remanence");
    physicFieldVariableList.insert(PhysicFieldVariable_Magnetic_Velocity, "magnetic_velocity");

    physicFieldVariableList.insert(PhysicFieldVariable_Current_Potential, "current_potential");
    physicFieldVariableList.insert(PhysicFieldVariable_Current_ElectricField, "current_electric_field");
    physicFieldVariableList.insert(PhysicFieldVariable_Current_CurrentDensity, "current_current_density");
    physicFieldVariableList.insert(PhysicFieldVariable_Current_Losses, "current_power_losses");
    physicFieldVariableList.insert(PhysicFieldVariable_Current_Conductivity, "current_conductivity");

    physicFieldVariableList.insert(PhysicFieldVariable_Heat_Temperature, "heat_temperature");
    physicFieldVariableList.insert(PhysicFieldVariable_Heat_TemperatureGradient, "heat_temperature_gradient");
    physicFieldVariableList.insert(PhysicFieldVariable_Heat_Flux, "heat_heat_flux");
    physicFieldVariableList.insert(PhysicFieldVariable_Heat_Conductivity, "heat_conductivity");

    physicFieldVariableList.insert(PhysicFieldVariable_Elasticity_VonMisesStress, "elasticity_von_mises_stress");
    physicFieldVariableList.insert(PhysicFieldVariable_Elasticity_Displacement, "elasticity_displacement");
    physicFieldVariableList.insert(PhysicFieldVariable_Elasticity_StrainXX, "strain_xx");
    physicFieldVariableList.insert(PhysicFieldVariable_Elasticity_StrainYY, "strain_yy");
    physicFieldVariableList.insert(PhysicFieldVariable_Elasticity_StrainZZ, "strain_zz");
    physicFieldVariableList.insert(PhysicFieldVariable_Elasticity_StrainXY, "strain_xy");
    physicFieldVariableList.insert(PhysicFieldVariable_Elasticity_StressXX, "stress_xx");
    physicFieldVariableList.insert(PhysicFieldVariable_Elasticity_StressYY, "stress_yy");
    physicFieldVariableList.insert(PhysicFieldVariable_Elasticity_StressZZ, "stress_zz");
    physicFieldVariableList.insert(PhysicFieldVariable_Elasticity_StressXY, "stress_xy");

    physicFieldVariableList.insert(PhysicFieldVariable_Acoustic_Pressure, "acoustic_pressure");
    physicFieldVariableList.insert(PhysicFieldVariable_Acoustic_PressureReal, "acoustic_pressure_real");
    physicFieldVariableList.insert(PhysicFieldVariable_Acoustic_PressureImag, "acoustic_pressure_imag");
    physicFieldVariableList.insert(PhysicFieldVariable_Acoustic_LocalVelocity, "acoustic_local_velocity");
    physicFieldVariableList.insert(PhysicFieldVariable_Acoustic_LocalAcceleration, "acoustic_local_acceleration");
    physicFieldVariableList.insert(PhysicFieldVariable_Acoustic_Density, "acoustic_density");
    physicFieldVariableList.insert(PhysicFieldVariable_Acoustic_Speed, "acoustic_speed");

    // PHYSICFIELDVARIABLECOMP
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Undefined, "");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Scalar, "scalar");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Magnitude, "magnitude");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_X, "x");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Y, "y");

    // PHYSICFIELDBC
    physicFieldBCList.insert(PhysicFieldBC_Undefined, "");
    physicFieldBCList.insert(PhysicFieldBC_None, "none");
    physicFieldBCList.insert(PhysicFieldBC_General_Value, "general_value");
    physicFieldBCList.insert(PhysicFieldBC_General_Derivative, "general_derivative");
    physicFieldBCList.insert(PhysicFieldBC_Electrostatic_Potential, "electrostatic_potential");
    physicFieldBCList.insert(PhysicFieldBC_Electrostatic_SurfaceCharge, "electrostatic_surface_charge_density");
    physicFieldBCList.insert(PhysicFieldBC_Magnetic_VectorPotential, "magnetic_vector_potential");
    physicFieldBCList.insert(PhysicFieldBC_Magnetic_SurfaceCurrent, "magnetic_surface_current_density");
    physicFieldBCList.insert(PhysicFieldBC_Heat_Temperature, "heat_temperature");
    physicFieldBCList.insert(PhysicFieldBC_Heat_Flux, "heat_heat_flux");
    physicFieldBCList.insert(PhysicFieldBC_Current_Potential, "current_potential");
    physicFieldBCList.insert(PhysicFieldBC_Current_InwardCurrentFlow, "current_inward_current_flow");
    physicFieldBCList.insert(PhysicFieldBC_Elasticity_Fixed, "elasticity_fixed");
    physicFieldBCList.insert(PhysicFieldBC_Elasticity_Free, "elasticity_free");
    physicFieldBCList.insert(PhysicFieldBC_Flow_Velocity, "flow_velocity");
    physicFieldBCList.insert(PhysicFieldBC_Flow_Pressure, "flow_pressure");
    physicFieldBCList.insert(PhysicFieldBC_Flow_Outlet, "flow_outlet");
    physicFieldBCList.insert(PhysicFieldBC_Flow_Wall, "flow_wall");
    physicFieldBCList.insert(PhysicFieldBC_RF_ElectricField, "rf_electric_field");
    physicFieldBCList.insert(PhysicFieldBC_RF_SurfaceCurrent, "rf_surface_current");
    physicFieldBCList.insert(PhysicFieldBC_RF_Scattering, "rf_scattering");
    physicFieldBCList.insert(PhysicFieldBC_RF_MatchedBoundary, "rf_matched_boundary");
    physicFieldBCList.insert(PhysicFieldBC_RF_Port, "rf_port");
    physicFieldBCList.insert(PhysicFieldBC_Acoustic_Pressure, "acoustic_pressure");
    physicFieldBCList.insert(PhysicFieldBC_Acoustic_NormalAcceleration, "acoustic_normal_acceleration");
    physicFieldBCList.insert(PhysicFieldBC_Acoustic_Impedance, "acoustic_impedance");
    physicFieldBCList.insert(PhysicFieldBC_Acoustic_MatchedBoundary, "acoustic_matched_boundary");

    // TEMODE
    modeList.insert(Mode_0, "mode_0");
    modeList.insert(Mode_01, "mode_01");
    modeList.insert(Mode_02, "mode_02");

    // SCENEVIEW_POSTPROCESSOR_SHOW
    sceneViewPostprocessorShowList.insert(SceneViewPostprocessorShow_Undefined, "");
    sceneViewPostprocessorShowList.insert(SceneViewPostprocessorShow_None, "none");
    sceneViewPostprocessorShowList.insert(SceneViewPostprocessorShow_ScalarView, "scalar");
    sceneViewPostprocessorShowList.insert(SceneViewPostprocessorShow_ScalarView3D, "scalar3d");
    sceneViewPostprocessorShowList.insert(SceneViewPostprocessorShow_Order, "order");

    // ADAPTIVITYTYPE
    adaptivityTypeList.insert(AdaptivityType_Undefined, "");
    adaptivityTypeList.insert(AdaptivityType_None, "disabled");
    adaptivityTypeList.insert(AdaptivityType_H, "h-adaptivity");
    adaptivityTypeList.insert(AdaptivityType_P, "p-adaptivity");
    adaptivityTypeList.insert(AdaptivityType_HP, "hp-adaptivity");

    // MatrixSolverType
    matrixSolverTypeList.insert(SOLVER_UMFPACK, "umfpack");
    matrixSolverTypeList.insert(SOLVER_PETSC, "petsc");
    matrixSolverTypeList.insert(SOLVER_MUMPS, "mumps");
    matrixSolverTypeList.insert(SOLVER_SUPERLU, "superlu");
    matrixSolverTypeList.insert(SOLVER_AMESOS, "trilinos_amesos");
    matrixSolverTypeList.insert(SOLVER_AZTECOO, "trilinos_aztecoo");

    // LinearityType
    linearityTypeList.insert(LinearityType_Undefined, "");
    linearityTypeList.insert(LinearityType_Linear, "linear");
    linearityTypeList.insert(LinearityType_Picard, "picard");
    linearityTypeList.insert(LinearityType_Newton, "newton");
}

QString physicFieldVariableString(PhysicFieldVariable physicFieldVariable)
{
    logMessage("physicFieldVariableString()");

    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Variable:
        return QObject::tr("Variable");
    case PhysicFieldVariable_General_Gradient:
        return QObject::tr("Gradient");
    case PhysicFieldVariable_General_Constant:
        return QObject::tr("Constant");

    case PhysicFieldVariable_Electrostatic_Potential:
        return QObject::tr("Scalar potential");
    case PhysicFieldVariable_Electrostatic_ElectricField:
        return QObject::tr("Electric field");
    case PhysicFieldVariable_Electrostatic_Displacement:
        return QObject::tr("Electric displacement");
    case PhysicFieldVariable_Electrostatic_EnergyDensity:
        return QObject::tr("Energy density");
    case PhysicFieldVariable_Electrostatic_Permittivity:
        return QObject::tr("Permittivity");

    case PhysicFieldVariable_Magnetic_VectorPotentialReal:
        return QObject::tr("Vector potential - real");
    case PhysicFieldVariable_Magnetic_VectorPotentialImag:
        return QObject::tr("Vector potential - imag");
    case PhysicFieldVariable_Magnetic_VectorPotential:
        return QObject::tr("Vector potential");
    case PhysicFieldVariable_Magnetic_FluxDensityReal:
        return QObject::tr("Flux density - real");
    case PhysicFieldVariable_Magnetic_FluxDensityImag:
        return QObject::tr("Flux density - imag");
    case PhysicFieldVariable_Magnetic_FluxDensity:
        return QObject::tr("Flux density");
    case PhysicFieldVariable_Magnetic_MagneticFieldReal:
        return QObject::tr("Magnetic field - real");
    case PhysicFieldVariable_Magnetic_MagneticFieldImag:
        return QObject::tr("Magnetic field - imag");
    case PhysicFieldVariable_Magnetic_MagneticField:
        return QObject::tr("Magnetic field");
    case PhysicFieldVariable_Magnetic_CurrentDensityReal:
        return QObject::tr("Current density - external - real");
    case PhysicFieldVariable_Magnetic_CurrentDensityImag:
        return QObject::tr("Current density - external - imag");
    case PhysicFieldVariable_Magnetic_CurrentDensity:
        return QObject::tr("Current density - external");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalReal:
        return QObject::tr("Current density - total - real");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalImag:
        return QObject::tr("Current density - total - imag");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotal:
        return QObject::tr("Current density - total");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformReal:
        return QObject::tr("Current density - induced transform - real");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformImag:
        return QObject::tr("Current density - induced transform - imag");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransform:
        return QObject::tr("Current density - induced transform");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityReal:
        return QObject::tr("Current density - induced velocity - real");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityImag:
        return QObject::tr("Current density - induced velocity - imag");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocity:
        return QObject::tr("Current density - induced velocity");
    case PhysicFieldVariable_Magnetic_PowerLosses:
        return QObject::tr("Power losses");
    case PhysicFieldVariable_Magnetic_LorentzForce:
        return QObject::tr("Lorentz force");
    case PhysicFieldVariable_Magnetic_EnergyDensity:
        return QObject::tr("Energy density");
    case PhysicFieldVariable_Magnetic_Permeability:
        return QObject::tr("Permeability");
    case PhysicFieldVariable_Magnetic_Conductivity:
        return QObject::tr("Conductivity");
    case PhysicFieldVariable_Magnetic_Remanence:
        return QObject::tr("Remanent flux density");
    case PhysicFieldVariable_Magnetic_Velocity:
        return QObject::tr("Velocity");

    case PhysicFieldVariable_Current_Potential:
        return QObject::tr("Scalar potential");
    case PhysicFieldVariable_Current_ElectricField:
        return QObject::tr("Electric field");
    case PhysicFieldVariable_Current_CurrentDensity:
        return QObject::tr("Current density");
    case PhysicFieldVariable_Current_Losses:
        return QObject::tr("Power losses");
    case PhysicFieldVariable_Current_Conductivity:
        return QObject::tr("Conductivity");

    case PhysicFieldVariable_Heat_Temperature:
        return QObject::tr("Temperature");
    case PhysicFieldVariable_Heat_TemperatureGradient:
        return QObject::tr("Temperature gradient");
    case PhysicFieldVariable_Heat_Flux:
        return QObject::tr("Heat flux");
    case PhysicFieldVariable_Heat_Conductivity:
        return QObject::tr("Conductivity");

    case PhysicFieldVariable_Elasticity_VonMisesStress:
        return QObject::tr("Von Mises stress");
    case PhysicFieldVariable_Elasticity_Displacement:
        return QObject::tr("Displacement");
    case PhysicFieldVariable_Elasticity_StrainXX:
        return QObject::tr("Normal strain ") + Util::scene()->problemInfo()->labelX() + Util::scene()->problemInfo()->labelX();
    case PhysicFieldVariable_Elasticity_StrainYY:
        return QObject::tr("Normal strain ") + Util::scene()->problemInfo()->labelY() + Util::scene()->problemInfo()->labelY();
    case PhysicFieldVariable_Elasticity_StrainZZ:
        return QObject::tr("Normal strain ") + Util::scene()->problemInfo()->labelZ() + Util::scene()->problemInfo()->labelZ();
    case PhysicFieldVariable_Elasticity_StrainXY:
        return QObject::tr("Shear strain ") + Util::scene()->problemInfo()->labelX() + Util::scene()->problemInfo()->labelY();
    case PhysicFieldVariable_Elasticity_StressXX:
        return QObject::tr("Normal stress ") + Util::scene()->problemInfo()->labelX() + Util::scene()->problemInfo()->labelX();
    case PhysicFieldVariable_Elasticity_StressYY:
        return QObject::tr("Normal stress ") + Util::scene()->problemInfo()->labelY() + Util::scene()->problemInfo()->labelY();
    case PhysicFieldVariable_Elasticity_StressZZ:
        return QObject::tr("Normal stress ") + Util::scene()->problemInfo()->labelZ() + Util::scene()->problemInfo()->labelZ();
    case PhysicFieldVariable_Elasticity_StressXY:
        return QObject::tr("Shear stress ") + Util::scene()->problemInfo()->labelX() + Util::scene()->problemInfo()->labelY();
    case PhysicFieldVariable_Acoustic_Pressure:
        return QObject::tr("Acoustic pressure");
    case PhysicFieldVariable_Acoustic_PressureReal:
        return QObject::tr("Acoustic pressure - real");
    case PhysicFieldVariable_Acoustic_PressureImag:
        return QObject::tr("Acoustic pressure - imag");
    case PhysicFieldVariable_Acoustic_PressureLevel:
        return QObject::tr("Sound pressure level");
    case PhysicFieldVariable_Acoustic_LocalVelocity:
        return QObject::tr("Local velocity");
    case PhysicFieldVariable_Acoustic_LocalAcceleration:
        return QObject::tr("Local acceleration");
    case PhysicFieldVariable_Acoustic_Density:
        return QObject::tr("Density");
    case PhysicFieldVariable_Acoustic_Speed:
        return QObject::tr("Speed of sound");
    case PhysicFieldVariable_Acoustic_Energy:
        return QObject::tr("Sound energy");
    case PhysicFieldVariable_Acoustic_EnergyLevel:
        return QObject::tr("Sound energy level");
        std::cerr << "Physical field '" + QString::number(physicFieldVariable).toStdString() + "' is not implemented. physicFieldVariableString(PhysicFieldVariable physicFieldVariable)" << endl;
        throw;
    }
}

QString physicFieldVariableShortcutString(PhysicFieldVariable physicFieldVariable)
{
    logMessage("physicFieldVariableShortcutString()");

    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Variable:
        return QObject::tr("V");
    case PhysicFieldVariable_General_Gradient:
        return QObject::tr("G");
    case PhysicFieldVariable_General_Constant:
        return QObject::tr("C");

    case PhysicFieldVariable_Electrostatic_Potential:
        return QObject::tr("V");
    case PhysicFieldVariable_Electrostatic_ElectricField:
        return QObject::tr("E");
    case PhysicFieldVariable_Electrostatic_Displacement:
        return QObject::tr("D");
    case PhysicFieldVariable_Electrostatic_EnergyDensity:
        return QObject::tr("we");
    case PhysicFieldVariable_Electrostatic_Permittivity:
        return QObject::tr("epsr");

    case PhysicFieldVariable_Magnetic_VectorPotentialReal:
        return QObject::tr("Are");
    case PhysicFieldVariable_Magnetic_VectorPotentialImag:
        return QObject::tr("Aim");
    case PhysicFieldVariable_Magnetic_VectorPotential:
        return QObject::tr("A");
    case PhysicFieldVariable_Magnetic_FluxDensityReal:
        return QObject::tr("Bre");
    case PhysicFieldVariable_Magnetic_FluxDensityImag:
        return QObject::tr("Bim");
    case PhysicFieldVariable_Magnetic_FluxDensity:
        return QObject::tr("B");
    case PhysicFieldVariable_Magnetic_MagneticFieldReal:
        return QObject::tr("Hre");
    case PhysicFieldVariable_Magnetic_MagneticFieldImag:
        return QObject::tr("Him");
    case PhysicFieldVariable_Magnetic_MagneticField:
        return QObject::tr("H");
    case PhysicFieldVariable_Magnetic_CurrentDensityReal:
        return QObject::tr("Jere");
    case PhysicFieldVariable_Magnetic_CurrentDensityImag:
        return QObject::tr("Jeim");
    case PhysicFieldVariable_Magnetic_CurrentDensity:
        return QObject::tr("Je");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalReal:
        return QObject::tr("Jtre");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalImag:
        return QObject::tr("Jtim");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotal:
        return QObject::tr("Jt");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformReal:
        return QObject::tr("Jitre");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformImag:
        return QObject::tr("Jitim");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransform:
        return QObject::tr("Jit");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityReal:
        return QObject::tr("Jivre");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityImag:
        return QObject::tr("Jivim");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocity:
        return QObject::tr("Jiv");
    case PhysicFieldVariable_Magnetic_PowerLosses:
        return QObject::tr("pj");
    case PhysicFieldVariable_Magnetic_LorentzForce:
        return QObject::tr("FL");
    case PhysicFieldVariable_Magnetic_EnergyDensity:
        return QObject::tr("wm");
    case PhysicFieldVariable_Magnetic_Permeability:
        return QObject::tr("mur");
    case PhysicFieldVariable_Magnetic_Conductivity:
        return QObject::tr("g");
    case PhysicFieldVariable_Magnetic_Remanence:
        return QObject::tr("Br");
    case PhysicFieldVariable_Magnetic_Velocity:
        return QObject::tr("v");

    case PhysicFieldVariable_Current_Potential:
        return QObject::tr("V");
    case PhysicFieldVariable_Current_ElectricField:
        return QObject::tr("E");
    case PhysicFieldVariable_Current_CurrentDensity:
        return QObject::tr("J");
    case PhysicFieldVariable_Current_Losses:
        return QObject::tr("pj");
    case PhysicFieldVariable_Current_Conductivity:
        return QObject::tr("g");

    case PhysicFieldVariable_Heat_Temperature:
        return QObject::tr("T");
    case PhysicFieldVariable_Heat_TemperatureGradient:
        return QObject::tr("G");
    case PhysicFieldVariable_Heat_Flux:
        return QObject::tr("F");
    case PhysicFieldVariable_Heat_Conductivity:
        return QObject::tr("k");

    case PhysicFieldVariable_Elasticity_VonMisesStress:
        return QObject::tr("E");
    case PhysicFieldVariable_Elasticity_Displacement:
        return QObject::tr("d");
    case PhysicFieldVariable_Elasticity_StrainXX:
        return QObject::tr("e") + Util::scene()->problemInfo()->labelX().toLower() + Util::scene()->problemInfo()->labelX().toLower();
    case PhysicFieldVariable_Elasticity_StrainYY:
        return QObject::tr("e") + Util::scene()->problemInfo()->labelY().toLower() + Util::scene()->problemInfo()->labelY().toLower();
    case PhysicFieldVariable_Elasticity_StrainZZ:
        return QObject::tr("e") + Util::scene()->problemInfo()->labelZ().toLower() + Util::scene()->problemInfo()->labelZ().toLower();
    case PhysicFieldVariable_Elasticity_StrainXY:
        return QObject::tr("e") + Util::scene()->problemInfo()->labelX().toLower() + Util::scene()->problemInfo()->labelY().toLower();
    case PhysicFieldVariable_Elasticity_StressXX:
        return QObject::tr("s") + Util::scene()->problemInfo()->labelX().toLower() + Util::scene()->problemInfo()->labelX().toLower();
    case PhysicFieldVariable_Elasticity_StressYY:
        return QObject::tr("s") + Util::scene()->problemInfo()->labelY().toLower() + Util::scene()->problemInfo()->labelY().toLower();
    case PhysicFieldVariable_Elasticity_StressZZ:
        return QObject::tr("s") + Util::scene()->problemInfo()->labelZ().toLower() + Util::scene()->problemInfo()->labelZ().toLower();
    case PhysicFieldVariable_Elasticity_StressXY:
        return QObject::tr("s") + Util::scene()->problemInfo()->labelX().toLower() + Util::scene()->problemInfo()->labelY().toLower();    
    case PhysicFieldVariable_Acoustic_Pressure:
        return QObject::tr("p");
    case PhysicFieldVariable_Acoustic_PressureReal:
        return QObject::tr("p");
    case PhysicFieldVariable_Acoustic_PressureImag:
        return QObject::tr("p");
    case PhysicFieldVariable_Acoustic_PressureLevel:
        return QObject::tr("Lp");
    case PhysicFieldVariable_Acoustic_LocalVelocity:
        return QObject::tr("v");
    case PhysicFieldVariable_Acoustic_LocalAcceleration:
        return QObject::tr("a");
    case PhysicFieldVariable_Acoustic_Density:
        return QObject::tr("rho");
    case PhysicFieldVariable_Acoustic_Speed:
        return QObject::tr("v");
    case PhysicFieldVariable_Acoustic_Energy:
        return QObject::tr("W");
    case PhysicFieldVariable_Acoustic_EnergyLevel:
        return QObject::tr("Lw");
    default:
        std::cerr << "Physical field '" + QString::number(physicFieldVariable).toStdString() + "' is not implemented. physicFieldVariableShortcutString(PhysicFieldVariable physicFieldVariable)" << endl;
        throw;
    }
}

QString physicFieldVariableUnitsString(PhysicFieldVariable physicFieldVariable)
{
    logMessage("physicFieldVariableUnitsString()");

    switch (physicFieldVariable)
    {
    case PhysicFieldVariable_Variable:
        return QObject::tr("");
    case PhysicFieldVariable_General_Gradient:
        return QObject::tr("");
    case PhysicFieldVariable_Electrostatic_Potential:
        return QObject::tr("V");
    case PhysicFieldVariable_Electrostatic_ElectricField:
        return QObject::tr("V/m");
    case PhysicFieldVariable_Electrostatic_Displacement:
        return QObject::tr("C/m2");
    case PhysicFieldVariable_Electrostatic_EnergyDensity:
        return QObject::tr("J/m3");
    case PhysicFieldVariable_Electrostatic_Permittivity:
        return QObject::tr("-");
    case PhysicFieldVariable_Magnetic_VectorPotentialReal:
        return QObject::tr("Wb/m");
    case PhysicFieldVariable_Magnetic_VectorPotentialImag:
        return QObject::tr("Wb/m");
    case PhysicFieldVariable_Magnetic_VectorPotential:
        return QObject::tr("Wb/m");
    case PhysicFieldVariable_Magnetic_FluxDensityReal:
        return QObject::tr("T");
    case PhysicFieldVariable_Magnetic_FluxDensityImag:
        return QObject::tr("T");
    case PhysicFieldVariable_Magnetic_FluxDensity:
        return QObject::tr("T");
    case PhysicFieldVariable_Magnetic_MagneticFieldReal:
        return QObject::tr("A/m");
    case PhysicFieldVariable_Magnetic_MagneticFieldImag:
        return QObject::tr("A/m");
    case PhysicFieldVariable_Magnetic_MagneticField:
        return QObject::tr("A/m");
    case PhysicFieldVariable_Magnetic_CurrentDensityReal:
        return QObject::tr("A/m");
    case PhysicFieldVariable_Magnetic_CurrentDensityImag:
        return QObject::tr("A/m");
    case PhysicFieldVariable_Magnetic_CurrentDensity:
        return QObject::tr("A/m");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalReal:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotalImag:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_CurrentDensityTotal:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformReal:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransformImag:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedTransform:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityReal:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocityImag:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_CurrentDensityInducedVelocity:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Magnetic_PowerLosses:
        return QObject::tr("W/m3");
    case PhysicFieldVariable_Magnetic_LorentzForce:
        return QObject::tr("N/m3");
    case PhysicFieldVariable_Magnetic_EnergyDensity:
        return QObject::tr("J/m3");
    case PhysicFieldVariable_Magnetic_Permeability:
        return QObject::tr("-");
    case PhysicFieldVariable_Magnetic_Conductivity:
        return QObject::tr("S/m");
    case PhysicFieldVariable_Magnetic_Remanence:
        return QObject::tr("T");
    case PhysicFieldVariable_Magnetic_Velocity:
        return QObject::tr("m/s");
    case PhysicFieldVariable_Current_Potential:
        return QObject::tr("V");
    case PhysicFieldVariable_Current_ElectricField:
        return QObject::tr("V/m");
    case PhysicFieldVariable_Current_CurrentDensity:
        return QObject::tr("A/m2");
    case PhysicFieldVariable_Current_Losses:
        return QObject::tr("W/m3");
    case PhysicFieldVariable_Current_Conductivity:
        return QObject::tr("S/m");
    case PhysicFieldVariable_Heat_Temperature:
        return QObject::tr("deg.");
    case PhysicFieldVariable_Heat_TemperatureGradient:
        return QObject::tr("K/m");
    case PhysicFieldVariable_Heat_Flux:
        return QObject::tr("W/m2");
    case PhysicFieldVariable_Heat_Conductivity:
        return QObject::tr("W/m.K");
    case PhysicFieldVariable_Elasticity_VonMisesStress:
        return QObject::tr("Pa");
    case PhysicFieldVariable_Elasticity_Displacement:
        return QObject::tr("m");
    case PhysicFieldVariable_Elasticity_StrainXX:
        return QObject::tr("-");
    case PhysicFieldVariable_Elasticity_StrainYY:
        return QObject::tr("-");
    case PhysicFieldVariable_Elasticity_StrainZZ:
        return QObject::tr("-");
    case PhysicFieldVariable_Elasticity_StrainXY:
        return QObject::tr("-");
    case PhysicFieldVariable_Elasticity_StressXX:
        return QObject::tr("Pa");
    case PhysicFieldVariable_Elasticity_StressYY:
        return QObject::tr("Pa");
    case PhysicFieldVariable_Elasticity_StressZZ:
        return QObject::tr("Pa");
    case PhysicFieldVariable_Elasticity_StressXY:
        return QObject::tr("Pa");
    case PhysicFieldVariable_Acoustic_Pressure:
        return QObject::tr("Pa");
    case PhysicFieldVariable_Acoustic_PressureReal:
        return QObject::tr("Pa");
    case PhysicFieldVariable_Acoustic_PressureImag:
        return QObject::tr("Pa");
    case PhysicFieldVariable_Acoustic_PressureLevel:
        return QObject::tr("dB");
    case PhysicFieldVariable_Acoustic_LocalVelocity:
        return QObject::tr("m/s");
    case PhysicFieldVariable_Acoustic_LocalAcceleration:
        return QObject::tr("m/s2");
    case PhysicFieldVariable_Acoustic_Density:
        return QObject::tr("kg/m3");
    case PhysicFieldVariable_Acoustic_Speed:
        return QObject::tr("m/s");
    case PhysicFieldVariable_Acoustic_Energy:
        return QObject::tr("J");
    case PhysicFieldVariable_Acoustic_EnergyLevel:
        return QObject::tr("dB");
    default:
        std::cerr << "Physical field '" + QString::number(physicFieldVariable).toStdString() + "' is not implemented. physicFieldVariableUnits(PhysicFieldVariable physicFieldVariable)" << endl;
        throw;
    }
}

QString physicFieldString(PhysicField physicField)
{
    logMessage("physicFieldString()");

    switch (physicField)
    {
    case PhysicField_General:
        return QObject::tr("General");
    case PhysicField_Electrostatic:
        return QObject::tr("Electrostatic field");
    case PhysicField_Magnetic:
        return QObject::tr("Magnetic field");
    case PhysicField_Current:
        return QObject::tr("Current field");
    case PhysicField_Heat:
        return QObject::tr("Heat transfer");
    case PhysicField_Elasticity:
        return QObject::tr("Structural mechanics");
    case PhysicField_Flow:
        return QObject::tr("Incompressible flow");    
    case PhysicField_Acoustic:
        return QObject::tr("Acoustics");
    default:
        std::cerr << "Physical field '" + QString::number(physicField).toStdString() + "' is not implemented. physicFieldString(PhysicField physicField)" << endl;
        throw;
    }
}

QString analysisTypeString(AnalysisType analysisType)
{
    logMessage("analysisTypeString()");

    switch (analysisType)
    {
    case AnalysisType_SteadyState:
        return QObject::tr("Steady state");
    case AnalysisType_Transient:
        return QObject::tr("Transient");
    case AnalysisType_Harmonic:
        return QObject::tr("Harmonic");
    default:
        std::cerr << "Analysis type '" + QString::number(analysisType).toStdString() + "' is not implemented. analysisTypeString(AnalysisType analysisType)" << endl;
        throw;
    }
}

QString teModeString(Mode teMode)
{
    logMessage("TEModeString()");

    switch (teMode)
    {
    case Mode_0:
        return QObject::tr("TE Mode 0");
    case Mode_01:
        return QObject::tr("TE Mode 01");
    case Mode_02:
        return QObject::tr("TE Mode 02");
    default:
        std::cerr << "TE mode '" + QString::number(teMode).toStdString() + "' is not implemented. TEModeString(TEMode teMode)" << endl;
        throw;
    }
}


QString physicFieldBCString(PhysicFieldBC physicFieldBC)
{
    logMessage("physicFieldBCString()");

    switch (physicFieldBC)
    {
    case PhysicFieldBC_None:
        return QObject::tr("none");
    case PhysicFieldBC_General_Value:
        return QObject::tr("Value");
    case PhysicFieldBC_General_Derivative:
        return QObject::tr("Derivative");
    case PhysicFieldBC_Electrostatic_Potential:
        return QObject::tr("Fixed voltage");
    case PhysicFieldBC_Electrostatic_SurfaceCharge:
        return QObject::tr("Surface charge density");
    case PhysicFieldBC_Magnetic_VectorPotential:
        return QObject::tr("Vector potential");
    case PhysicFieldBC_Magnetic_SurfaceCurrent:
        return QObject::tr("Surface current density");
    case PhysicFieldBC_Heat_Temperature:
        return QObject::tr("Temperature");
    case PhysicFieldBC_Heat_Flux:
        return QObject::tr("Heat flux");
    case PhysicFieldBC_Current_Potential:
        return QObject::tr("Potential");
    case PhysicFieldBC_Current_InwardCurrentFlow:
        return QObject::tr("Inward current flow");
    case PhysicFieldBC_Elasticity_Fixed:
        return QObject::tr("Fixed");
    case PhysicFieldBC_Elasticity_Free:
        return QObject::tr("Free");
    case PhysicFieldBC_Flow_Outlet:
        return QObject::tr("Outlet");
    case PhysicFieldBC_Flow_Wall:
        return QObject::tr("Wall");
    case PhysicFieldBC_Flow_Velocity:
        return QObject::tr("Velocity");
    case PhysicFieldBC_Flow_Pressure:
        return QObject::tr("Pressure");
    case PhysicFieldBC_RF_ElectricField:
        return QObject::tr("Electric field");
    case PhysicFieldBC_RF_SurfaceCurrent:
        return QObject::tr("Surface current");
    case PhysicFieldBC_RF_Scattering:
        return QObject::tr("Scattering boundary");
    case PhysicFieldBC_RF_MatchedBoundary:
        return QObject::tr("Matched boundary");
    case PhysicFieldBC_RF_Port:
        return QObject::tr("Port");
    case PhysicFieldBC_Acoustic_Pressure:
        return QObject::tr("Acoustic pressure");
    case PhysicFieldBC_Acoustic_NormalAcceleration:
        return QObject::tr("Normal acceleration");
    case PhysicFieldBC_Acoustic_Impedance:
        return QObject::tr("Impedance boundary condition");
    case PhysicFieldBC_Acoustic_MatchedBoundary:
        return QObject::tr("Matched boundary");
    default:
        std::cerr << "Physical field '" + QString::number(physicFieldBC).toStdString() + "' is not implemented. physicFieldBCString(PhysicFieldBC physicFieldBC)" << endl;
        throw;
    }
}

QString physicFieldVariableCompString(PhysicFieldVariableComp physicFieldVariableComp)
{
    logMessage("physicFieldVariableCompString()");

    switch (physicFieldVariableComp)
    {
    case PhysicFieldVariableComp_Scalar:
        return QObject::tr("Scalar");
    case PhysicFieldVariableComp_Magnitude:
        return QObject::tr("Magnitude");
    case PhysicFieldVariableComp_X:
        return Util::scene()->problemInfo()->labelX();
    case PhysicFieldVariableComp_Y:
        return Util::scene()->problemInfo()->labelY();
    default:
        return QObject::tr("Undefined");
    }
}

QString problemTypeString(ProblemType problemType)
{
    logMessage("problemTypeString()");

    return ((problemType == ProblemType_Planar) ? QObject::tr("Planar") : QObject::tr("Axisymmetric"));
}

QString adaptivityTypeString(AdaptivityType adaptivityType)
{
    logMessage("adaptivityTypeString()");

    switch (adaptivityType)
    {
    case AdaptivityType_None:
        return QObject::tr("Disabled");
    case AdaptivityType_H:
        return QObject::tr("h-adaptivity");
    case AdaptivityType_P:
        return QObject::tr("p-adaptivity");
    case AdaptivityType_HP:
        return QObject::tr("hp-adaptivity");
    default:
        std::cerr << "Adaptivity type '" + QString::number(adaptivityType).toStdString() + "' is not implemented. adaptivityTypeString(AdaptivityType adaptivityType)" << endl;
        throw;
    }
}

QString meshTypeString(MeshType meshType)
{
    logMessage("meshTypeString()");

    switch (meshType)
    {
    case MeshType_Triangle:
        return QObject::tr("Triangle");
    case MeshType_QuadFineDivision:
        return QObject::tr("Quad fine div.");
    case MeshType_QuadRoughDivision:
        return QObject::tr("Quad rough div.");
    case MeshType_QuadJoin:
        return QObject::tr("Quad join");
    default:
        std::cerr << "Mesh type '" + QString::number(meshType).toStdString() + "' is not implemented. meshTypeString(MeshType meshType)" << endl;
        throw;
    }
}

QString linearityTypeString(LinearityType linearityType)
{
    logMessage("linearityTypeString()");

    switch (linearityType)
    {
    case LinearityType_Linear:
        return QObject::tr("Linear");
    case LinearityType_Picard:
        return QObject::tr("Picard’s method");
    case LinearityType_Newton:
        return QObject::tr("Newton’s method");
    default:
        std::cerr << "Linearity type '" + QString::number(linearityType).toStdString() + "' is not implemented. linearityTypeString(LinearityType linearityType)" << endl;
        throw;
    }
}

QString matrixSolverTypeString(MatrixSolverType matrixSolverType)
{
    logMessage("matrixSolverTypeString()");

    switch (matrixSolverType)
    {
    case SOLVER_UMFPACK:
        return QObject::tr("UMFPACK");
    case SOLVER_PETSC:
        return QObject::tr("PETSc");
    case SOLVER_MUMPS:
        return QObject::tr("MUMPS");
    case SOLVER_SUPERLU:
        return QObject::tr("SuperLU");
    case SOLVER_AMESOS:
        return QObject::tr("Trilinos/Amesos");
    case SOLVER_AZTECOO:
        return QObject::tr("Trilinos/AztecOO");
    default:
        std::cerr << "Matrix solver type '" + QString::number(matrixSolverType).toStdString() + "' is not implemented. matrixSolverTypeString(MatrixSolverType matrixSolverType)" << endl;
        throw;
    }
}

void fillComboBoxPhysicField(QComboBox *cmbPhysicField)
{
    logMessage("fillComboBoxPhysicField()");

    cmbPhysicField->clear();
    cmbPhysicField->addItem(physicFieldString(PhysicField_General), PhysicField_General);
    cmbPhysicField->addItem(physicFieldString(PhysicField_Electrostatic), PhysicField_Electrostatic);
    cmbPhysicField->addItem(physicFieldString(PhysicField_Magnetic), PhysicField_Magnetic);
    cmbPhysicField->addItem(physicFieldString(PhysicField_Current), PhysicField_Current);
    cmbPhysicField->addItem(physicFieldString(PhysicField_Heat), PhysicField_Heat);
    cmbPhysicField->addItem(physicFieldString(PhysicField_Elasticity), PhysicField_Elasticity);
    cmbPhysicField->addItem(physicFieldString(PhysicField_Acoustic), PhysicField_Acoustic);

    // default physic field
    cmbPhysicField->setCurrentIndex(cmbPhysicField->findData(Util::config()->defaultPhysicField));
    if (cmbPhysicField->currentIndex() == -1)
        cmbPhysicField->setCurrentIndex(0);
}

void setGUIStyle(const QString &styleName)
{
    logMessage("setGUIStyle()");

    QStyle *style = NULL;
    if (styleName == "Manhattan")
    {
        QString styleName = "";
        QStringList styles = QStyleFactory::keys();

#ifdef Q_WS_X11
        // kde 3
        if (getenv("KDE_FULL_SESSION") != NULL)
            styleName = "Plastique";
        // kde 4
        if (getenv("KDE_SESSION_VERSION") != NULL)
        {
            if (styles.contains("Oxygen"))
                styleName = "Oxygen";
            else
                styleName = "Plastique";
        }

        // gtk+
        if (styleName == "")
            styleName = "GTK+";
#else
        styleName = "Plastique";
#endif

        style = new ManhattanStyle(styleName);
    }
    else
    {
        // standard style
        style = QStyleFactory::create(styleName);
    }

    QApplication::setStyle(style);
    if (QApplication::desktopSettingsAware())
    {
        QApplication::setPalette(QApplication::palette());
    }
}

void setLanguage(const QString &locale)
{
    logMessage("setLanguage()");
    // qDebug() << "Locale: " << locale;

    // non latin-1 chars
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));

    QTranslator *qtTranslator = new QTranslator();
    QTranslator *appTranslator = new QTranslator();

    QString country = locale.section('_', 0, 0);
    if (QFile::exists(QLibraryInfo::location(QLibraryInfo::TranslationsPath) + "/qt_" + country + ".qm"))
        qtTranslator->load(QLibraryInfo::location(QLibraryInfo::TranslationsPath) + "/qt_" + country + ".qm");
    else if (QFile::exists(datadir() + "/lang/qt_" + country + ".qm"))
        qtTranslator->load(datadir() + "/lang/qt_" + country + ".qm");
    else
        qDebug() << "Qt language file not found.";

    if (QFile::exists(datadir() + "/lang/" + locale + ".qm"))
        appTranslator->load(datadir() + "/lang/" + locale + ".qm");
    else if (QFile::exists(datadir() + "/lang/en_US.qm"))
        appTranslator->load(datadir() + "/lang/en_US.qm");
    else
        qDebug() << "Language file not found.";

    QApplication::installTranslator(qtTranslator);
    QApplication::installTranslator(appTranslator);
}

QStringList availableLanguages()
{
    logMessage("availableLanguages()");

    QDir dir;
    dir.setPath(datadir() + "/lang");

    // add all translations
    QStringList filters;
    filters << "*.qm";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    // remove extension
    QStringList list = dir.entryList();
    list.replaceInStrings(".qm", "");

    // remove system translations
    foreach (QString str, list)
        if (str.startsWith("qt_"))
            list.removeOne(str);

    return list;
}

QIcon icon(const QString &name)
{
    QString fileName;

#ifdef Q_WS_WIN
    if (QFile::exists(":/images/" + name + "-windows.png")) return QIcon(":/images/" + name + "-windows.png");
#endif

#ifdef Q_WS_X11
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
    return QIcon::fromTheme(name, QIcon(":images/" + name + ".png"));
#endif

    QDir dir;

    QString style = "";
    QStringList styles = QStyleFactory::keys();

    // kde 3
    if (getenv("KDE_FULL_SESSION") != NULL)
    {}
    // kde 4
    if (getenv("KDE_SESSION_VERSION") != NULL)
    {
        // oxygen
        fileName = "/usr/share/icons/oxygen/22x22/actions/" + name;
        if (QFile::exists(fileName + ".svg")) return QIcon(fileName + ".svg");
        if (QFile::exists(fileName + ".png")) return QIcon(fileName + ".png");
    }
    // gtk+
    if (style == "")
    {
        // humanity (disabled - corrupted svg reader - Qt 4.6 has new method QIcon::fromTheme)
        // fileName = "/usr/share/icons/Humanity/actions/24/" + name;
        // if (QFile::exists(fileName + ".svg")) return QIcon(fileName + ".svg");
        // if (QFile::exists(fileName + ".png")) return QIcon(fileName + ".png");
    }
#endif

    if (QFile::exists(":images/" + name + ".svg")) return QIcon(":images/" + name + ".svg");
    if (QFile::exists(":images/" + name + ".png")) return QIcon(":images/" + name + ".png");

    return QIcon();
}

QString datadir()
{
    logMessage("datadir()");

    // windows and local installation
    if (QFile::exists(QApplication::applicationDirPath() + "/functions.py"))
        return QApplication::applicationDirPath();

    // linux
    if (QFile::exists(QApplication::applicationDirPath() + "/../share/agros2d/functions.py"))
        return QApplication::applicationDirPath() + "/../share/agros2d";

    qCritical() << "Datadir not found.";
    exit(1);
}

QString tempProblemDir()
{
    logMessage("tempProblemDir()");

    QDir(QDir::temp().absolutePath()).mkpath("agros2d/" + QString::number(QApplication::applicationPid()));

    return QString("%1/agros2d/%2").arg(QDir::temp().absolutePath()).arg(QApplication::applicationPid());
}

QString tempProblemFileName()
{
    logMessage("tempProblemFileName()");

    return tempProblemDir() + "/temp";
}

QTime milisecondsToTime(int ms)
{
    logMessage("milisecondsToTime()");

    // store the current ms remaining
    int tmp_ms = ms;

    // the amount of days left
    int days = floorf(tmp_ms/86400000);
    // adjust tmp_ms to leave remaining hours, minutes, seconds
    tmp_ms = tmp_ms - (days * 86400000);

    // calculate the amount of hours remaining
    int hours = floorf(tmp_ms/3600000);
    // adjust tmp_ms to leave the remaining minutes and seconds
    tmp_ms = tmp_ms - (hours * 3600000);

    // the amount of minutes remaining
    int mins = floorf(tmp_ms/60000);
    //adjust tmp_ms to leave only the remaining seconds
    tmp_ms = tmp_ms - (mins * 60000);

    // seconds remaining
    int secs = floorf(tmp_ms/1000);

    // milliseconds remaining
    tmp_ms = tmp_ms - (secs * 1000);

    return QTime(hours, mins, secs, tmp_ms);
}

bool removeDirectory(const QDir &dir)
{
    logMessage("removeDirectory()");

    bool error = false;

    if (dir.exists())
    {
        QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
        int count = entries.size();
        for (int idx = 0; idx < count; idx++)
        {
            QFileInfo entryInfo = entries[idx];
            QString path = entryInfo.absoluteFilePath();
            if (entryInfo.isDir())
            {
                error = removeDirectory(QDir(path));
            }
            else
            {
                QFile file(path);
                if (!file.remove())
                {
                    error = true;
                    break;
                }
            }
        }
        if (!dir.rmdir(dir.absolutePath()))
            error = true;
    }

    return error;
}

void msleep(unsigned long msecs)
{
    logMessage("msleep()");

    QWaitCondition w;
    QMutex sleepMutex;
    sleepMutex.lock();
    w.wait(&sleepMutex, msecs);
    sleepMutex.unlock();
}

// verbose
void setVerbose(bool verb)
{
    verbose = verb;
}

QString formatLogMessage(QtMsgType type, const QString &msg)
{
    QString msgType = "";

    switch (type) {
    case QtDebugMsg:
        msgType = "Debug";
        break;
    case QtWarningMsg:
        msgType = "Warning";
        break;
    case QtCriticalMsg:
        msgType = "Critical";
        break;
    case QtFatalMsg:
        msgType = "Fatal";
        break;
    }

    QString str = QString("%1 %2: %3").
            arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz")).
            arg(msgType).
            arg(msg);

    return str;
}

void appendToFile(const QString &fileName, const QString &str)
{
    QFile file(fileName);

    if (file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream outFile(&file);
        outFile << str << endl;

        file.close();
    }
}

void logOutput(QtMsgType type, const char *msg)
{
    QString str = formatLogMessage(type, msg);

    // string
    fprintf(stderr, "%s\n", str.toStdString().c_str());

    if (Util::singleton() && Util::config()->enabledApplicationLog)
    {
        QString location = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
        QDir("/").mkpath(location);

        appendToFile(location + "/app.log", str);
    }

    if (type == QtFatalMsg)
        abort();
}

void logMessage(const QString &msg)
{
    if (verbose)
    {
        QString location = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
        QDir("/").mkpath(location);

        appendToFile(location + "/app.log", formatLogMessage(QtDebugMsg, msg));
    }
}

void showPage(const QString &str)
{
    logMessage("showPage()");

    if (str.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile(datadir() + "/doc/help/index.html"));
    else
        QDesktopServices::openUrl(QUrl::fromLocalFile(datadir() + "/doc/help/" + str));
}


QString readFileContent(const QString &fileName)
{
    logMessage("readFileContent()");

    QString content;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        content = stream.readAll();
        file.close();
        return content;
    }
    return NULL;
}

void writeStringContent(const QString &fileName, QString *content)
{
    logMessage("writeStringContent()");

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << *content;

        file.waitForBytesWritten(0);
        file.close();
    }
}

QByteArray readFileContentByteArray(const QString &fileName)
{
    logMessage("eadFileContentByteArray()");

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray content = file.readAll();
        file.close();
        return content;
    }
    return NULL;
}

void writeStringContentByteArray(const QString &fileName, QByteArray content)
{
    logMessage("writeStringContentByteArray()");

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(content);

        file.waitForBytesWritten(0);
        file.close();
    }
}

Point centerPoint(const Point &pointStart, const Point &pointEnd, double angle)
{
    double distance = (pointEnd - pointStart).magnitude();
    Point t = (pointEnd - pointStart) / distance;
    double R = distance / (2.0*sin(angle/180.0*M_PI / 2.0));

    Point p = Point(distance/2.0, sqrt(sqr(R) - sqr(distance)/4.0 > 0.0 ? sqr(R) - sqr(distance)/4.0 : 0.0));
    Point center = pointStart + Point(p.x*t.x - p.y*t.y, p.x*t.y + p.y*t.x);

    return center;
}

//   Function tests if angle specified by the third parameter is between angles
//   angle_1 and angle_2
bool isBetween(double angle_1, double angle_2, double angle)
{
    // tolerance 1e-3 degree
    double tol = 1e-3 * M_PI / 180;

    // angle_1 has to be lower then angle_2
    if (angle_2 < angle_1)
    {
        double temp = angle_1;
        angle_1 = angle_2;
        angle_2 = temp;
    }

    // angle_1 is in the third quadrant, angle_2 is in the second quadrant
    if ((angle_1 <= - (M_PI/2.0 - tol) ) && (angle_2 >= (M_PI/2.0 - tol)))
    {
        if((angle <= angle_1) || (angle >= angle_2))
        {
           return true;
        }

    } else
    {
        if((angle >= angle_1) && (angle <= angle_2))
        {
           return true;
        }
    }
    return false;
}

bool intersection(Point p, Point p1s, Point p1e)
{
    Point dv_line = p1e - p1s;
    Point dv_point = p1e - p;
    if ((dv_line.angle() - dv_point.angle()) < EPS_ZERO)
        return true;
    else
        return false;
}

Point *intersection(Point p1s, Point p1e, Point p2s, Point p2e)
{
    double denom = (p2e.y-p2s.y)*(p1e.x-p1s.x) - (p2e.x-p2s.x)*(p1e.y-p1s.y);

    double nume_a = ((p2e.x-p2s.x)*(p1s.y-p2s.y) - (p2e.y-p2s.y)*(p1s.x-p2s.x));
    double nume_b = ((p1e.x-p1s.x)*(p1s.y-p2s.y) - (p1e.y-p1s.y)*(p1s.x-p2s.x));

    double ua = nume_a / denom;
    double ub = nume_b / denom;

    if ((p2e != p1s) && (p1e != p2s) && (p1e != p2e) && (p1s != p2s))
    {
        if ((abs(denom) > EPS_ZERO) && (ua >= 0.0) && (ua <= 1.0) && (ub >= 0.0) && (ub <= 1.0))
        {
            double xi = p1s.x + ua*(p1e.x - p1s.x);
            double yi = p1s.y + ua*(p1e.y - p1s.y);

            return new Point(xi, yi);
        }
    }

    return NULL;
}

QList<Point> intersection(Point p1s, Point p1e, Point center1, double radius1, double angle1,
                          Point p2s, Point p2e, Point center2, double radius2, double angle2)
{
    QList<Point> out;

    Point dif = p1e - p1s;      // direction vector of the line
    double a = sqr(dif.x) + sqr(dif.y);   // square of the length of direction vector
    double tol = 1e-7 * sqrt(a);

    // Crossing of two arcs
    if ((angle1 > 0.0) && (angle2 > 0.0))
    {
        if (((p1s == p2e) && (p1e == p2s)) || ((p1e == p2e) && (p1s == p2s)))
        {
            // Crossing of arcs is not possible
        }
        else
        {
            // Calculate distance between centres of circle
            float distance = (center1 - center2).magnitude();
            float dx = center2.x - center1.x;
            float dy = center2.y - center1.y;

            if ((distance < (radius1 + radius2)))
            {
                // Determine the distance from point 0 to point 2.
                double a = ((radius1*radius1) - (radius2*radius2) + (distance*distance)) / (2.0 * distance);

                // Determine the coordinates of point 2.
                Point middle;
                middle.x = center1.x + (dx * a/distance);
                middle.y = center1.y + (dy * a/distance);

                // Determine the distance from point 2 to either of the
                // intersection points.
                double h = std::sqrt((radius1 * radius1) - (a*a));

                // Now determine the offsets of the intersection points from
                // point 2.
                double rx = -dy * (h/distance);
                double ry =  dx * (h/distance);

                // Determine the absolute intersection points.
                Point p1(middle.x + rx, middle.y + ry);
                Point p2(middle.x - rx, middle.y - ry);

                // Angles of starting and end points due to center of the arc 1
                double angle1_1 = (p1e - center1).angle();
                double angle2_1 = (p1s - center1).angle();

                // Angles of starting and end points due to center of the arc 2
                double angle1_2 = (p2e - center2).angle();
                double angle2_2 = (p2s - center2).angle();

                // Angles of intersection points due to center of the arc 1
                double iangle1_1 = (p1 - center1).angle();
                double iangle2_1 = (p2 - center1).angle();

                // Angles of intersection points due to center of the arc 2
                double iangle1_2 = (p1 - center2).angle();
                double iangle2_2 = (p2 - center2).angle();

                // Test if intersection 1 lies on arc
                if ((isBetween(angle1_1, angle2_1, iangle1_1) && isBetween(angle1_2 , angle2_2, iangle1_2)))
                {
                    if (((p1 - p1s).magnitude() > tol) &&
                            ((p1 - p1e).magnitude() > tol) &&
                            ((p1 - p2e).magnitude() > tol) &&
                            ((p1 - p2s).magnitude() > tol))
                        out.append(p1);
                }

                // Test if intersection 1 lies on arc
                if (isBetween(angle1_1, angle2_1, iangle2_1) && isBetween(angle1_2, angle2_2, iangle2_2))
                {
                    if(((p2 - p1s).magnitude() > tol) &&
                            ((p2 - p1e).magnitude() > tol) &&
                            ((p2 - p2e).magnitude() > tol) &&
                            ((p2 - p2s).magnitude() > tol))
                        out.append(p2);
                }
            }
        }
    }
    else
    {
        if (angle2 > 0.0)
            // crossing of arc and line
        {
            double b = 2 * (dif.x * (p1s.x - center2.x) + dif.y * (p1s.y - center2.y));
            double c = sqr(p1s.x) + sqr(p1s.y) + sqr(center2.x) + sqr(center2.y) - 2.0 * (center2.x * p1s.x + center2.y * p1s.y) - sqr(radius2);

            double bb4ac = sqr(b) - 4 * a * c;

            double mu1 = (-b + sqrt(bb4ac)) / (2*a);
            double mu2 = (-b - sqrt(bb4ac)) / (2*a);

            // possible intersection points
            Point p1 = p1s + dif * mu1;
            Point p2 = p1s + dif * mu2;

            double t1;
            double t2;

            if (abs(dif.x - dif.y) > tol)
            {
                 t1 = (p1.x - p1s.x - p1.y + p1s.y) / (dif.x - dif.y); // tangent
                 t2 = (p2.x - p1s.x - p2.y + p1s.y) / (dif.x - dif.y); // tangent
            }
            else
            {
                t1 = (p1.x - p1s.x) / dif.x; // tangent
                t2 = (p2.x - p1s.x) / dif.x; // tangent
            }

            double angle_1 = (p2e - center2).angle();
            double angle_2 = (p2s - center2).angle();
            double iangle1 = (p1 - center2).angle();
            double iangle2 = (p2 - center2).angle();

            if ((t1 >= 0) && (t1 <= 1))
            {
                // 1 solution: One Point in the circle
                if (isBetween(angle_1, angle_2, iangle1) && ((p1 - p2s).magnitude() > tol) && ((p1 - p2e).magnitude() > tol))
                    out.append(p1);
            }

            if ((t2 >= 0) && (t2 <= 1))
            {
                // 1 solution: One Point in the circle
                if (isBetween(angle_1, angle_2, iangle2) && ((p2 -  p2s).magnitude() > tol) && ((p2 - p2e).magnitude() > tol))
                    out.append(p2);
            }
        }
        else
        {
            // straight line
            Point *point = intersection(p1s, p1e, p2s, p2e);
            if (point)
                out.append(Point(point->x, point->y));
            delete point;
        }
    }
    return out;
}

static CheckVersion *checkVersion = NULL;
void checkForNewVersion(bool quiet)
{
    logMessage("checkForNewVersion()");

    // download version
    QUrl url("http://agros2d.org/version/version.xml");
    if (checkVersion == NULL)
        checkVersion = new CheckVersion(url);

    checkVersion->run(quiet);
}

CheckVersion::CheckVersion(QUrl url) : QObject()
{
    logMessage("CheckVersion::CheckVersion()");

    m_url = url;

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(downloadFinished(QNetworkReply *)));
}

CheckVersion::~CheckVersion()
{
    logMessage("CheckVersion::~CheckVersion()");

    delete m_manager;
}

void CheckVersion::run(bool quiet)
{
    logMessage("CheckVersion::run()");

    m_quiet = quiet;
    m_networkReply = m_manager->get(QNetworkRequest(m_url));

    connect(m_networkReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(showProgress(qint64,qint64)));
    connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(handleError(QNetworkReply::NetworkError)));
}

void CheckVersion::downloadFinished(QNetworkReply *networkReply)
{
    logMessage("CheckVersion::downloadFinished()");

    QString text = networkReply->readAll();

    if (!text.isEmpty())
    {
        QDomDocument doc;
        doc.setContent(text);

        // main document
        QDomElement eleDoc = doc.documentElement();

        // problems
        QDomNode eleVersion = eleDoc.toElement().elementsByTagName("version").at(0);

        int beta = eleVersion.toElement().attribute("beta").toInt() == 1;
        int major = eleVersion.toElement().attribute("major").toInt();
        int minor = eleVersion.toElement().attribute("minor").toInt();
        int sub = eleVersion.toElement().attribute("sub").toInt();
        int git = eleVersion.toElement().attribute("git").toInt();
        int year = eleVersion.toElement().attribute("year").toInt();
        int month = eleVersion.toElement().attribute("month").toInt();
        int day = eleVersion.toElement().attribute("day").toInt();

        QDomNode eleUrl = eleDoc.toElement().elementsByTagName("url").at(0);

        if (!m_quiet && git == 0)
        {
            QMessageBox::critical(QApplication::activeWindow(), tr("New version"), tr("File is corrupted or network is disconnected."));
            return;
        }

        QString downloadUrl = eleUrl.toElement().text();
        if (git > VERSION_GIT)
        {
            QString str(tr("<b>New version available.</b><br/><br/>"
                           "Actual version: %1<br/>"
                           "New version: %2<br/><br/>"
                           "URL: <a href=\"%3\">%3</a>").
                        arg(QApplication::applicationVersion()).
                        arg(versionString(major, minor, sub, git, year, month, day, beta)).
                        arg(downloadUrl));

            QMessageBox::information(QApplication::activeWindow(), tr("New version"), str);
        }
        else if (!m_quiet)
        {
            QMessageBox::information(QApplication::activeWindow(), tr("New version"), tr("You are using actual version."));
        }
    }
}

void CheckVersion::showProgress(qint64 dl, qint64 all)
{
    logMessage("CheckVersion::showProgress()");

    // qDebug() << QString("\rDownloaded %1 bytes of %2).").arg(dl).arg(all);
}

void CheckVersion::handleError(QNetworkReply::NetworkError error)
{
    logMessage("CheckVersion::handleError()");

    qDebug() << "An error ocurred (code #" << error << ").";
}
