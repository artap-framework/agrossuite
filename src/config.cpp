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

#include "config.h"
#include "scene.h"

Config::Config() : eleConfig(NULL)
{
    logMessage("Config::Config()");

    load();
}

Config::~Config()
{
    logMessage("Config::~Config()");

    save();
}

void Config::load()
{
    loadWorkspace();
    loadPostprocessor(NULL);
    loadAdvanced();
}

void Config::loadWorkspace()
{
    QSettings settings;

    // general
    guiStyle = settings.value("General/GUIStyle").toString();
    language = settings.value("General/Language", QLocale::system().name()).toString();
    defaultPhysicField = (PhysicField) settings.value("General/DefaultPhysicField", PhysicField_Electrostatic).toInt();
    if (defaultPhysicField == PhysicField_Undefined) defaultPhysicField = PhysicField_Electrostatic;

    collaborationServerURL = settings.value("General/CollaborationServerURL", QString("http://agros2d.org/collaboration/")).toString();

    checkVersion = settings.value("General/CheckVersion", true).toBool();
    showConvergenceChart = settings.value("General/ShowConvergenceChart", true).toBool();
    enabledApplicationLog = settings.value("General/EnabledApplicationLog", true).toBool();
    enabledProgressLog = settings.value("General/EnabledProgressLog", true).toBool();
    lineEditValueShowResult = settings.value("General/LineEditValueShowResult", false).toBool();

    // zoom
    zoomToMouse = settings.value("Geometry/ZoomToMouse", true).toBool();

    // delete files
    deleteTriangleMeshFiles = settings.value("Solver/DeleteTriangleMeshFiles", true).toBool();
    deleteHermes2DMeshFile = settings.value("Solver/DeleteHermes2DMeshFile", true).toBool();

    // colors
    colorBackground = settings.value("SceneViewSettings/ColorBackground", COLORBACKGROUND).value<QColor>();
    colorGrid = settings.value("SceneViewSettings/ColorGrid", COLORGRID).value<QColor>();
    colorCross = settings.value("SceneViewSettings/ColorCross", COLORCROSS).value<QColor>();
    colorNodes = settings.value("SceneViewSettings/ColorNodes", COLORNODES).value<QColor>();
    colorEdges = settings.value("SceneViewSettings/ColorEdges", COLOREDGES).value<QColor>();
    colorLabels = settings.value("SceneViewSettings/ColorLabels", COLORLABELS).value<QColor>();
    colorContours = settings.value("SceneViewSettings/ColorContours", COLORCONTOURS).value<QColor>();
    colorVectors = settings.value("SceneViewSettings/ColorVectors", COLORVECTORS).value<QColor>();
    colorInitialMesh = settings.value("SceneViewSettings/ColorInitialMesh", COLORINITIALMESH).value<QColor>();
    colorSolutionMesh = settings.value("SceneViewSettings/ColorSolutionMesh", COLORSOLUTIONMESH).value<QColor>();
    colorHighlighted = settings.value("SceneViewSettings/ColorHighlighted", COLORHIGHLIGHTED).value<QColor>();
    colorSelected = settings.value("SceneViewSettings/ColorSelected", COLORSELECTED).value<QColor>();
    colorCrossed = settings.value("SceneViewSettings/ColorCrossed", COLORCROSSED).value<QColor>();
    colorNotConnected = settings.value("SceneViewSettings/ColorCrossed", COLORNOTCONNECTED).value<QColor>();

    // geometry
    nodeSize = settings.value("SceneViewSettings/NodeSize", 6.0).toDouble();
    edgeWidth = settings.value("SceneViewSettings/EdgeWidth", 2.0).toDouble();
    labelSize = settings.value("SceneViewSettings/LabelSize", 6.0).toDouble();

    // font
    sceneFont = settings.value("SceneViewSettings/SceneFont", FONT).value<QFont>();

    // mesh
    angleSegmentsCount = settings.value("SceneViewSettings/AngleSegmentsCount", 3).toInt();
    curvilinearElements = settings.value("SceneViewSettings/CurvilinearElements", true).toBool();

    // grid
    showGrid = settings.value("SceneViewSettings/ShowGrid", SHOWGRID).toBool();
    gridStep = settings.value("SceneViewSettings/GridStep", GRIDSTEP).toDouble();

    // rulers
    showRulers = settings.value("SceneViewSettings/ShowRulers", SHOWRULERS).toBool();
    // snap to grid
    snapToGrid = settings.value("SceneViewSettings/SnapToGrid", SNAPTOGRID).toBool();

    // axes
    showAxes = settings.value("SceneViewSettings/ShowAxes", SHOWAXES).toBool();

    // label
    showLabel = settings.value("SceneViewSettings/ShowLabel", SHOWLABEL).toBool();

    // linearizer quality
    linearizerQuality = settings.value("SceneViewSettings/LinearizerQuality", LINEARIZER_QUALITY).toDouble();

    // 3d
    scalarView3DLighting = settings.value("SceneViewSettings/ScalarView3DLighting", false).toBool();
    scalarView3DAngle = settings.value("SceneViewSettings/ScalarView3DAngle", 270).toDouble();
    scalarView3DBackground = settings.value("SceneViewSettings/ScalarView3DBackground", true).toBool();
    scalarView3DHeight = settings.value("SceneViewSettings/ScalarView3DHeight", 4.0).toDouble();

    // deformations
    deformScalar = settings.value("SceneViewSettings/DeformScalar", true).toBool();
    deformContour = settings.value("SceneViewSettings/DeformContour", true).toBool();
    deformVector = settings.value("SceneViewSettings/DeformVector", true).toBool();
}

void Config::loadPostprocessor(QDomElement *config)
{
    if (config)
        eleConfig = config;

    // contour
    contoursCount = readConfig("SceneViewSettings/ContoursCount", CONTOURSCOUNT);
    contourWidth = readConfig("SceneViewSettings/ContourWidth", CONTOURWIDTH);

    // scalar view
    showScalarScale = readConfig("SceneViewSettings/ShowScalarScale", true);
    paletteType = (PaletteType) readConfig("SceneViewSettings/PaletteType", PALETTETYPE);
    paletteFilter = readConfig("SceneViewSettings/PaletteFilter", PALETTEFILTER);
    paletteSteps = readConfig("SceneViewSettings/PaletteSteps", PALETTESTEPS);
    scalarRangeLog = readConfig("SceneViewSettings/ScalarRangeLog", SCALARFIELDRANGELOG);
    scalarRangeBase = readConfig("SceneViewSettings/ScalarRangeBase", SCALARFIELDRANGEBASE);
    scalarDecimalPlace = readConfig("SceneViewSettings/ScalarDecimalPlace", SCALARDECIMALPLACE);

    // vector view
    vectorProportional = readConfig("SceneViewSettings/VectorProportional", VECTORPROPORTIONAL);
    vectorColor = readConfig("SceneViewSettings/VectorColor", VECTORCOLOR);
    vectorCount = readConfig("SceneViewSettings/VectorNumber", VECTORNUMBER);
    vectorScale = readConfig("SceneViewSettings/VectorScale", VECTORSCALE);

    // order view
    showOrderScale = readConfig("SceneViewSettings/ShowOrderScale", true);
    orderPaletteOrderType = (PaletteOrderType) readConfig("SceneViewSettings/OrderPaletteOrderType", ORDERPALETTEORDERTYPE);
    orderLabel = readConfig("SceneViewSettings/OrderLabel", ORDERLABEL);

    // particle tracing
    particleIncludeGravitation = readConfig("SceneViewSettings/ParticleIncludeGravitation", PARTICLEINCLUDEGRAVITATION);
    particleMass = readConfig("SceneViewSettings/ParticleMass", PARTICLEMASS);
    particleConstant = readConfig("SceneViewSettings/ParticleConstant", PARTICLECONSTANT);
    particleStart.x = readConfig("SceneViewSettings/ParticleStartX", PARTICLESTARTX);
    particleStart.y = readConfig("SceneViewSettings/ParticleStartY", PARTICLESTARTY);
    particleStartVelocity.x = readConfig("SceneViewSettings/ParticleStartVelocityX", PARTICLESTARTVELOCITYX);
    particleStartVelocity.y = readConfig("SceneViewSettings/ParticleStartVelocityY", PARTICLESTARTVELOCITYY);
    particleNumberOfParticles = readConfig("SceneViewSettings/ParticleNumberOfParticles", PARTICLENUMBEROFPARTICLES);
    particleStartingRadius = readConfig("SceneViewSettings/ParticleStartingRadius", PARTICLESTARTINGRADIUS);
    particleReflectOnDifferentMaterial = readConfig("SceneViewSettings/ParticleReflectOnDifferentMaterial", PARTICLEREFLECTONDIFFERENTMATERIAL);
    particleReflectOnBoundary = readConfig("SceneViewSettings/ParticleReflectOnBoundary", PARTICLEREFLECTONBOUNDARY);
    particleCoefficientOfRestitution = readConfig("SceneViewSettings/ParticleCoefficientOfRestitution", PARTICLECOEFFICIENTOFRESTITUTION);
    particleMaximumRelativeError = readConfig("SceneViewSettings/ParticleMaximumRelativeError", PARTICLEMAXIMUMRELATIVEERROR);
    particleShowPoints = readConfig("SceneViewSettings/ParticleShowPoints", PARTICLESHOWPOINTS);
    particleColorByVelocity = readConfig("SceneViewSettings/ParticleColorByVelocity", PARTICLECOLORBYVELOCITY);
    particleMaximumNumberOfSteps = readConfig("SceneViewSettings/ParticleMaximumNumberOfSteps", PARTICLEMAXIMUMNUMBEROFSTEPS);
    particleMinimumStep = readConfig("SceneViewSettings/ParticleMinimumStep", PARTICLEMINIMUMSTEP);
    particleDragDensity = readConfig("SceneViewSettings/ParticleDragDensity", PARTICLEDRAGDENSITY);
    particleDragCoefficient = readConfig("SceneViewSettings/ParticleDragCoefficient", PARTICLEDRAGCOEFFICIENT);
    particleDragReferenceArea = readConfig("SceneViewSettings/ParticleDragReferenceArea", PARTICLEDRAGREFERENCEAREA);

    eleConfig = NULL;
}

void Config::loadAdvanced()
{
    QSettings settings;

    // adaptivity
    maxDofs = settings.value("Adaptivity/MaxDofs", MAX_DOFS).toInt();
    isoOnly = settings.value("Adaptivity/IsoOnly", ADAPTIVITY_ISOONLY).toBool();
    convExp = settings.value("Adaptivity/ConvExp", ADAPTIVITY_CONVEXP).toDouble();
    threshold = settings.value("Adaptivity/Threshold", ADAPTIVITY_THRESHOLD).toDouble();
    strategy = settings.value("Adaptivity/Strategy", ADAPTIVITY_STRATEGY).toInt();
    meshRegularity = settings.value("Adaptivity/MeshRegularity", ADAPTIVITY_MESHREGULARITY).toInt();
    projNormType = (ProjNormType) settings.value("Adaptivity/ProjNormType", ADAPTIVITY_PROJNORMTYPE).toInt();

    // command argument
    commandTriangle = settings.value("Commands/Triangle", COMMANDS_TRIANGLE).toString();
    // add quadratic elements (added points on the middle of edge used by rough triangle division)
    if (!commandTriangle.contains("-o2"))
        commandTriangle = COMMANDS_TRIANGLE;
    commandFFmpeg = settings.value("Commands/FFmpeg", COMMANDS_FFMPEG).toString();

    // global script
    globalScript = settings.value("Python/GlobalScript", "").toString();
}

void Config::save()
{
    saveWorkspace();
    saveAdvanced();
}

void Config::saveWorkspace()
{
    QSettings settings;

    // general
    settings.setValue("General/GUIStyle", guiStyle);
    settings.setValue("General/Language", language);
    settings.setValue("General/DefaultPhysicField", defaultPhysicField);

    settings.setValue("General/CollaborationServerURL", collaborationServerURL);

    settings.setValue("General/CheckVersion", checkVersion);
    settings.setValue("General/ShowConvergenceChart", showConvergenceChart);
    settings.setValue("General/EnabledApplicationLog", enabledApplicationLog);
    settings.setValue("General/EnabledProgressLog", enabledProgressLog);
    settings.setValue("General/LineEditValueShowResult", lineEditValueShowResult);

    // zoom
    settings.setValue("General/ZoomToMouse", zoomToMouse);

    // delete files
    settings.setValue("Solver/DeleteTriangleMeshFiles", deleteTriangleMeshFiles);
    settings.setValue("Solver/DeleteHermes2DMeshFile", deleteHermes2DMeshFile);

    // colors
    settings.setValue("SceneViewSettings/ColorBackground", colorBackground);
    settings.setValue("SceneViewSettings/ColorGrid", colorGrid);
    settings.setValue("SceneViewSettings/ColorCross", colorCross);
    settings.setValue("SceneViewSettings/ColorNodes", colorNodes);
    settings.setValue("SceneViewSettings/ColorEdges", colorEdges);
    settings.setValue("SceneViewSettings/ColorLabels", colorLabels);
    settings.setValue("SceneViewSettings/ColorContours", colorContours);
    settings.setValue("SceneViewSettings/ColorVectors", colorVectors);
    settings.setValue("SceneViewSettings/ColorInitialMesh", colorInitialMesh);
    settings.setValue("SceneViewSettings/ColorSolutionMesh", colorSolutionMesh);
    settings.setValue("SceneViewSettings/ColorInitialMesh", colorHighlighted);
    settings.setValue("SceneViewSettings/ColorSolutionMesh", colorSelected);

    // geometry
    settings.setValue("SceneViewSettings/NodeSize", nodeSize);
    settings.setValue("SceneViewSettings/EdgeWidth", edgeWidth);
    settings.setValue("SceneViewSettings/LabelSize", labelSize);

    // font
    settings.setValue("SceneViewSettings/SceneFont", sceneFont);

    // mesh
    settings.setValue("SceneViewSettings/AngleSegmentsCount", angleSegmentsCount);
    settings.setValue("SceneViewSettings/CurvilinearElements", curvilinearElements);

    // grid
    settings.setValue("SceneViewSettings/ShowGrid", showGrid);
    settings.setValue("SceneViewSettings/GridStep", gridStep);

    // scene font
    settings.setValue("SceneViewSettings/SceneFont", sceneFont);

    // rulers
    settings.setValue("SceneViewSettings/ShowRulers", showRulers);
    // snap to grid
    settings.setValue("SceneViewSettings/SnapToGrid", snapToGrid);

    // axes
    settings.setValue("SceneViewSettings/ShowAxes", showAxes);

    // label
    settings.setValue("SceneViewSettings/ShowLabel", showLabel);

    // linearizer quality
    settings.setValue("SceneViewSettings/LinearizerQuality", linearizerQuality);

    // 3d
    settings.setValue("SceneViewSettings/ScalarView3DLighting", scalarView3DLighting);
    settings.setValue("SceneViewSettings/ScalarView3DAngle", scalarView3DAngle);
    settings.setValue("SceneViewSettings/ScalarView3DBackground", scalarView3DBackground);
    settings.setValue("SceneViewSettings/ScalarView3DHeight", scalarView3DHeight);

    // deformations
    settings.setValue("SceneViewSettings/DeformScalar", deformScalar);
    settings.setValue("SceneViewSettings/DeformContour", deformContour);
    settings.setValue("SceneViewSettings/DeformVector", deformVector);
}

void Config::savePostprocessor(QDomElement *config)
{
    if (config)
        eleConfig = config;

    // contour
    writeConfig("SceneViewSettings/ContoursCount", contoursCount);
    writeConfig("SceneViewSettings/ContourWidth", contourWidth);

    // scalar view
    writeConfig("SceneViewSettings/ShowScalarScale", showScalarScale);
    writeConfig("SceneViewSettings/PaletteType", paletteType);
    writeConfig("SceneViewSettings/PaletteFilter", paletteFilter);
    writeConfig("SceneViewSettings/PaletteSteps", paletteSteps);
    writeConfig("SceneViewSettings/ScalarRangeLog", scalarRangeLog);
    writeConfig("SceneViewSettings/ScalarRangeBase", scalarRangeBase);
    writeConfig("SceneViewSettings/ScalarDecimalPlace", scalarDecimalPlace);

    // vector view
    writeConfig("SceneViewSettings/VectorProportional", vectorProportional);
    writeConfig("SceneViewSettings/VectorColor", vectorColor);
    writeConfig("SceneViewSettings/VectorNumber", vectorCount);
    writeConfig("SceneViewSettings/VectorScale", vectorScale);

    // order view
    writeConfig("SceneViewSettings/ShowOrderScale", showOrderScale);
    writeConfig("SceneViewSettings/OrderPaletteOrderType", orderPaletteOrderType);
    writeConfig("SceneViewSettings/OrderLabel", orderLabel);

    // particle tracing
    writeConfig("SceneViewSettings/ParticleIncludeGravitation", particleIncludeGravitation);
    writeConfig("SceneViewSettings/ParticleMass", particleMass);
    writeConfig("SceneViewSettings/ParticleConstant", particleConstant);
    writeConfig("SceneViewSettings/ParticleStartX", particleStart.x);
    writeConfig("SceneViewSettings/ParticleStartY", particleStart.y);
    writeConfig("SceneViewSettings/ParticleStartVelocityX", particleStartVelocity.x);
    writeConfig("SceneViewSettings/ParticleStartVelocityY", particleStartVelocity.y);
    writeConfig("SceneViewSettings/ParticleNumberOfParticles", particleNumberOfParticles);
    writeConfig("SceneViewSettings/ParticleStartingRadius", particleStartingRadius);
    writeConfig("SceneViewSettings/ParticleReflectOnDifferentMaterial", particleReflectOnDifferentMaterial);
    writeConfig("SceneViewSettings/ParticleReflectOnBoundary", particleReflectOnBoundary);
    writeConfig("SceneViewSettings/ParticleCoefficientOfRestitution", particleCoefficientOfRestitution);
    writeConfig("SceneViewSettings/ParticleMaximumRelativeError", particleMaximumRelativeError);
    writeConfig("SceneViewSettings/ParticleShowPoints", particleShowPoints);
    writeConfig("SceneViewSettings/ParticleColorByVelocity", particleColorByVelocity);
    writeConfig("SceneViewSettings/ParticleMaximumNumberOfSteps", particleMaximumNumberOfSteps);
    writeConfig("SceneViewSettings/ParticleMinimumStep", particleMinimumStep);
    writeConfig("SceneViewSettings/ParticleDragDensity", particleDragDensity);
    writeConfig("SceneViewSettings/ParticleDragCoefficient", particleDragCoefficient);
    writeConfig("SceneViewSettings/ParticleDragReferenceArea", particleDragReferenceArea);

    eleConfig = NULL;
}

void Config::saveAdvanced()
{
    QSettings settings;

    // adaptivity
    settings.setValue("Adaptivity/MaxDofs", maxDofs);
    settings.setValue("Adaptivity/IsoOnly", isoOnly);
    settings.setValue("Adaptivity/ConvExp", convExp);
    settings.setValue("Adaptivity/Threshold", threshold);
    settings.setValue("Adaptivity/Strategy", strategy);
    settings.setValue("Adaptivity/MeshRegularity", meshRegularity);
    settings.setValue("Adaptivity/ProjNormType", projNormType);

    // command argument
    settings.setValue("Commands/Triangle", commandTriangle);
    settings.setValue("Commands/FFmpeg", commandFFmpeg);

    // global script
    settings.setValue("Python/GlobalScript", globalScript);
}

bool Config::readConfig(const QString &key, bool defaultValue)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        if (eleConfig->hasAttribute(att))
            return (eleConfig->attribute(att).toInt() == 1) ? true : false;
    }

    return defaultValue;
}

int Config::readConfig(const QString &key, int defaultValue)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        if (eleConfig->hasAttribute(att))
            return eleConfig->attribute(att).toInt();
    }

    return defaultValue;
}

double Config::readConfig(const QString &key, double defaultValue)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        if (eleConfig->hasAttribute(att))
            return eleConfig->attribute(att).toDouble();
    }

    return defaultValue;
}

void Config::writeConfig(const QString &key, bool value)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        eleConfig->setAttribute(att, value);
    }
}

void Config::writeConfig(const QString &key, int value)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        eleConfig->setAttribute(att, value);
    }
}

void Config::writeConfig(const QString &key, double value)
{
    if (eleConfig)
    {
        QString att = key; att.replace("/", "_");
        eleConfig->setAttribute(att, value);
    }
}
