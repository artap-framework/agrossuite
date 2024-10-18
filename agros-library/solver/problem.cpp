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

#include <boost/config.hpp>
#include <boost/archive/tmpdir.hpp>
// #include <boost/archive/binary_iarchive.hpp>
// #include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "problem.h"

#include <QtWidgets/QApplication>
#include <QtXml/QDomDocument>
#include <util/script_generator.h>

#include "problem_config.h"
#include "problem_result.h"
#include "plugin_interface.h"

#include "util/global.h"
#include "util/constants.h"
#include "util/conf.h"

#include "field.h"
#include "solutionstore.h"

#include "scene.h"
#include "scenemarker.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "coupling.h"
#include "logview.h"

#include "mesh/meshgenerator_triangle.h"

#include "optilab/study.h"

#include "../3rdparty/quazip/JlCompress.h"

#define VERSION "version"

#define LIST "list"
#define ANGLE "angle"
#define SEGMENTS "segments"
#define X "x"
#define Y "y"
#define Z "Z"
#define AREA "area"
#define VALUE "value"

#define ID "id"
#define NAME "name"
#define TYPE "type"
#define GEOMETRY "geometry"
#define NODES "nodes"
#define FACES "faces"
#define LABELS "labels"
#define SETTINGS "settings"
#define CONFIG "config"
#define FIELDS "fields"
#define FIELDID "fieldid"
#define BOUNDARIES "boundaries"
#define BOUNDARY_FACES "boundary_faces"
#define BOUNDARY_TYPES "boundary_types"
#define MATERIALS "materials"
#define MATERIAL_LABELS "material_labels"
#define MATERIAL_TYPES "material_types"
#define REFINEMENT_ORDER "refinement_order"

#define COUPLINGS "couplings"
#define SOURCE_FIELDID "source_field"
#define TARGET_FIELDID "target_field"

#define STUDIES "studies"
#define RECIPES "recipes"

PostDeal::PostDeal(Computation* computation) :
    m_computation(computation),
    m_isProcessed(false),
    m_activeViewField(nullptr),
    m_activeTimeStep(NOT_FOUND_SO_FAR),
    m_activeAdaptivityStep(NOT_FOUND_SO_FAR)
{
    // connect(m_computation->scene(), SIGNAL(cleared()), this, SLOT(clear()));
}

PostDeal::~PostDeal()
{
    clear();
}

void PostDeal::processRangeContour()
{
    if (m_computation->isSolved() && m_activeViewField && (m_computation->setting()->value(
        PostprocessorSetting::ShowContourView).toBool()))
    {
        Agros::log()->printMessage(tr("Post View"),
                                   tr("Contour view (%1)").arg(
                                       m_computation->setting()->value(PostprocessorSetting::ContourVariable).
                                                      toString()));

        QString variableName = m_computation->setting()->value(PostprocessorSetting::ContourVariable).toString();
        Module::LocalVariable variable = m_activeViewField->localVariable(
            m_computation->config()->coordinateType(), variableName);

        m_contourValues.clear();

        if (variable.isScalar())
            viewScalarFilter(m_activeViewField->localVariable(m_computation->config()->coordinateType(),
                                                              m_computation->setting()->value(
                                                                  PostprocessorSetting::ContourVariable).toString()),
                             PhysicFieldVariableComp_Scalar,
                             m_contourValues,
                             (m_activeViewField->hasDeformableShape() && m_computation->setting()->value(
                                 PostprocessorSetting::DeformContour).toBool()));

        else
            viewScalarFilter(m_activeViewField->localVariable(m_computation->config()->coordinateType(),
                                                              m_computation->setting()->value(
                                                                  PostprocessorSetting::ContourVariable).toString()),
                             PhysicFieldVariableComp_Magnitude,
                             m_contourValues,
                             (m_activeViewField->hasDeformableShape() && m_computation->setting()->value(
                                 PostprocessorSetting::DeformContour).toBool()));
    }
}

void PostDeal::processRangeScalar()
{
    if (m_computation->setting()->value(PostprocessorSetting::ScalarRangeAuto).toBool())
    {
        m_computation->setting()->setValue(PostprocessorSetting::ScalarRangeMin, 0.0);
        m_computation->setting()->setValue(PostprocessorSetting::ScalarRangeMax, 0.0);
    }

    if ((m_computation->isSolved()) && (m_activeViewField)
        && ((m_computation->setting()->value(PostprocessorSetting::ShowScalarView).toBool())
            || (((SceneViewPost3DMode)m_computation->setting()->value(PostprocessorSetting::ScalarView3DMode).toInt())
                == SceneViewPost3DMode_ScalarView3D)))
    {
        Agros::log()->printMessage(tr("Post View"),
                                   tr("Scalar view (%1)").arg(
                                       m_computation->setting()->value(
                                           PostprocessorSetting::ScalarVariable).toString()));

        viewScalarFilter(m_activeViewField->localVariable(m_computation->config()->coordinateType(),
                                                          m_computation->setting()->value(
                                                              PostprocessorSetting::ScalarVariable).toString()),
                         (PhysicFieldVariableComp)m_computation->setting()->value(
                             PostprocessorSetting::ScalarVariableComp).toInt(),
                         m_scalarValues,
                         (m_activeViewField->hasDeformableShape() && m_computation->setting()->value(
                             PostprocessorSetting::DeformScalar).toBool()));

        // min and max value
        if (m_computation->setting()->value(PostprocessorSetting::ScalarRangeAuto).toBool())
        {
            double min = numeric_limits<double>::max();
            double max = -numeric_limits<double>::max();

            foreach(PostTriangle triangle, m_scalarValues)
            {
                double valueMin = std::min(std::min(triangle.values[0], triangle.values[1]), triangle.values[2]);
                double valueMax = std::max(std::max(triangle.values[0], triangle.values[1]), triangle.values[2]);

                min = std::min(min, valueMin);
                max = std::max(max, valueMax);
            }

            m_computation->setting()->setValue(PostprocessorSetting::ScalarRangeMin, min);
            m_computation->setting()->setValue(PostprocessorSetting::ScalarRangeMax, max);
        }
    }
}

void PostDeal::processRangeVector()
{
    if ((m_computation->isSolved()) && (m_activeViewField) && (m_computation->setting()->value(
        PostprocessorSetting::ShowVectorView).toBool()))
    {
        foreach(Module::LocalVariable variable,
                m_activeViewField->viewVectorVariables(m_computation->config()->coordinateType()))
        {
            if (variable.id() == m_computation->setting()->value(PostprocessorSetting::VectorVariable).toString())
            {
                break;
            }
        }

        Agros::log()->printMessage(tr("Post View"),
                                   tr("Vector view (%1)").arg(
                                       m_computation->setting()->value(
                                           PostprocessorSetting::VectorVariable).toString()));

        viewScalarFilter(m_activeViewField->localVariable(m_computation->config()->coordinateType(),
                                                          m_computation->setting()->value(
                                                              PostprocessorSetting::VectorVariable).toString()),
                         PhysicFieldVariableComp_X,
                         m_vectorXValues,
                         false);

        viewScalarFilter(m_activeViewField->localVariable(m_computation->config()->coordinateType(),
                                                          m_computation->setting()->value(
                                                              PostprocessorSetting::VectorVariable).toString()),
                         PhysicFieldVariableComp_Y,
                         m_vectorYValues,
                         false);
    }
}

void PostDeal::clear()
{
    clearView();

    m_activeViewField = nullptr;
    m_activeTimeStep = NOT_FOUND_SO_FAR;
    m_activeAdaptivityStep = NOT_FOUND_SO_FAR;
}

void PostDeal::clearView()
{
    m_isProcessed = false;

    m_contourValues.clear();
    m_scalarValues.clear();
    m_vectorXValues.clear();
    m_vectorYValues.clear();
}

void PostDeal::refresh()
{
    m_computation->setIsPostprocessingRunning();
    clearView();

    if (m_computation->isSolved())
        processSolved();

    m_isProcessed = true;
    m_computation->setIsPostprocessingRunning(false);
}

void PostDeal::processSolved()
{
    FieldSolutionID fsid(activeViewField()->fieldId(), activeTimeStep(), activeAdaptivityStep());
    if (m_computation->solutionStore()->contains(fsid))
    {
        processRangeContour();
        processRangeScalar();
        processRangeVector();
    }
}

void PostDeal::viewScalarFilter(Module::LocalVariable physicFieldVariable,
                                PhysicFieldVariableComp physicFieldVariableComp,
                                QList<PostTriangle>& list,
                                bool deform)
{
    // QTime time;
    // time.start();

    dealii::DataPostprocessorScalar<2>* post = activeViewField()->plugin()->filter(m_computation,
        activeViewField(),
        activeTimeStep(),
        activeAdaptivityStep(),
        physicFieldVariable.id(),
        physicFieldVariableComp);

    MultiArray ma = activeMultiSolutionArray();

    PostDataOut data_out(activeViewField(), m_computation);
    data_out.attach_dof_handler(ma.doFHandler());
    data_out.add_data_vector(ma.solution(), *post);
    // deform shape
    if (m_activeViewField->hasDeformableShape() && m_computation->setting()->value(PostprocessorSetting::DeformScalar).
                                                                  toBool())
    {
        std::vector<std::string> solution_names;
        solution_names.push_back("x_displacement");
        solution_names.push_back("y_displacement");

        data_out.add_data_vector(ma.solution(), solution_names);
    }
    data_out.build_patches(2);

    // compute nodes
    data_out.compute_nodes(list, deform);

    // release post object
    delete post;

    // qDebug() << "process - build patches (" << time.elapsed() << "ms )";
}

void PostDeal::setActiveViewField(FieldInfo* fieldInfo)
{
    // previous active field
    FieldInfo* previousActiveViewField = m_activeViewField;

    // set new field
    m_activeViewField = fieldInfo;

    // check for different field
    if (previousActiveViewField != fieldInfo)
    {
        setActiveTimeStep(NOT_FOUND_SO_FAR);
        setActiveAdaptivityStep(NOT_FOUND_SO_FAR);

        // set default variables
        Module::LocalVariable scalarVariable = m_activeViewField->defaultViewScalarVariable(
            m_computation->config()->coordinateType());
        Module::LocalVariable vectorVariable = m_activeViewField->defaultViewVectorVariable(
            m_computation->config()->coordinateType());

        QString scalarVariableDefault = scalarVariable.id();
        PhysicFieldVariableComp scalarVariableCompDefault = scalarVariable.isScalar()
                                                                ? PhysicFieldVariableComp_Scalar
                                                                : PhysicFieldVariableComp_Magnitude;
        QString contourVariableDefault = scalarVariable.id();
        QString vectorVariableDefault = vectorVariable.id();

        foreach(Module::LocalVariable local,
                m_activeViewField->viewScalarVariables(m_computation->config()->coordinateType()))
        {
            if (m_computation->setting()->value(PostprocessorSetting::ScalarVariable).toString() == local.id())
            {
                scalarVariableDefault = m_computation->setting()->value(PostprocessorSetting::ScalarVariable).
                                                       toString();
                scalarVariableCompDefault = (PhysicFieldVariableComp)m_computation->setting()->value(
                    PostprocessorSetting::ScalarVariableComp).toInt();
            }
            if (m_computation->setting()->value(PostprocessorSetting::ContourVariable).toString() == local.id())
            {
                contourVariableDefault = m_computation->setting()->value(PostprocessorSetting::ContourVariable).
                                                        toString();
            }
        }
        foreach(Module::LocalVariable local,
                m_activeViewField->viewScalarVariables(m_computation->config()->coordinateType()))
        {
            if (m_computation->setting()->value(PostprocessorSetting::VectorVariable).toString() == local.id())
            {
                vectorVariableDefault = m_computation->setting()->value(PostprocessorSetting::VectorVariable).
                                                       toString();
            }
        }

        m_computation->setting()->setValue(PostprocessorSetting::ScalarVariable, scalarVariableDefault);
        m_computation->setting()->setValue(PostprocessorSetting::ScalarVariableComp, scalarVariableCompDefault);
        m_computation->setting()->setValue(PostprocessorSetting::ContourVariable, contourVariableDefault);
        m_computation->setting()->setValue(PostprocessorSetting::VectorVariable, vectorVariableDefault);

        // order component
        m_computation->setting()->setValue(PostprocessorSetting::OrderComponent, 1);
    }
}

void PostDeal::setActiveTimeStep(int ts)
{
    m_activeTimeStep = ts;
}

void PostDeal::setActiveAdaptivityStep(int as)
{
    m_activeAdaptivityStep = as;
}

MultiArray PostDeal::activeMultiSolutionArray()
{
    FieldSolutionID fsid(activeViewField()->fieldId(), activeTimeStep(), activeAdaptivityStep());
    if (m_computation->solutionStore()->contains(fsid))
    {
        return m_computation->solutionStore()->multiArray(fsid);
    }
    else
    {
        assert(0);
        return MultiArray();
    }
}

// ************************************************************************************************

PostDataOut::PostDataOut(FieldInfo* fieldInfo, Computation* parentProblem) : dealii::DataOut<2>(),
                                                                             m_computation(parentProblem),
                                                                             m_fieldInfo(fieldInfo)
{
    // select only cells with material without none marker
    this->set_cell_selection([this](const typename dealii::DataOut<2>::cell_iterator& cell)
    {
        if (cell->is_active())
            return !(this->m_computation->scene()->labels->at(cell->material_id() - 1)->marker(this->m_fieldInfo)->
                           isNone());
        else
            return false;
    });
}

void PostDataOut::compute_nodes(QList<PostTriangle>& values, bool deform)
{
    values.clear();

    // min and max value
    double min = numeric_limits<double>::max();
    double max = -numeric_limits<double>::max();
    // double minDeform = numeric_limits<double>::max();
    double maxDeform = -numeric_limits<double>::max();

    for (std::vector<dealii::DataOutBase::Patch<2>>::const_iterator patch = patches.begin(); patch != patches.end(); ++
         patch)
    {
        for (unsigned int i = 0; i < (patch->n_subdivisions + 1) * (patch->n_subdivisions + 1); i++)
        {
            double value = patch->data(0, i);

            min = std::min(min, value);
            max = std::max(max, value);

            if (deform)
            {
                double value = sqrt(patch->data(1, i) * patch->data(1, i) + patch->data(2, i) * patch->data(2, i));

                // minDeform = std::min(min, value);
                maxDeform = std::max(max, value);
            }
        }
    }

    double dmult = 0.0;
    if (deform)
    {
        RectPoint rect = m_computation->scene()->boundingBox();
        dmult = qMax(rect.width(), rect.height()) / maxDeform / 15.0;
    }

    // compute values in patches
    dealii::Point<2> node0, node1, node2, node3;

    // loop over all patches
    for (std::vector<dealii::DataOutBase::Patch<2>>::const_iterator patch = patches.begin(); patch != patches.end(); ++
         patch)
    {
        const unsigned int n_subdivisions = patch->n_subdivisions;
        const unsigned int n = n_subdivisions + 1;
        unsigned int d1 = 1;
        unsigned int d2 = n;

        for (unsigned int i2 = 0; i2 < n - 1; ++i2)
        {
            for (unsigned int i1 = 0; i1 < n - 1; ++i1)
            {
                // compute coordinates for this patch point
                compute_node(node0, &*patch, i1, i2, 0, n_subdivisions);
                compute_node(node1, &*patch, i1, i2 + 1, 0, n_subdivisions);
                compute_node(node2, &*patch, i1 + 1, i2, 0, n_subdivisions);
                compute_node(node3, &*patch, i1 + 1, i2 + 1, 0, n_subdivisions);

                // compute values
                int index0 = i1 * d1 + i2 * d2;
                int index1 = i1 * d1 + (i2 + 1) * d2;
                int index2 = (i1 + 1) * d1 + i2 * d2;
                int index3 = (i1 + 1) * d1 + (i2 + 1) * d2;

                double value0 = patch->data(0, index0);
                double value1 = patch->data(0, index1);
                double value2 = patch->data(0, index2);
                double value3 = patch->data(0, index3);

                if (deform)
                {
                    // node(0) ... value
                    // node(1) ... disp x
                    // node(2) ... disp y

                    double dispx0 = patch->data(1, index0);
                    double dispy0 = patch->data(2, index0);
                    double dispx1 = patch->data(1, index1);
                    double dispy1 = patch->data(2, index1);
                    double dispx2 = patch->data(1, index2);
                    double dispy2 = patch->data(2, index2);
                    double dispx3 = patch->data(1, index3);
                    double dispy3 = patch->data(2, index3);

                    node0 += dmult * dealii::Point<2>(dispx0, dispy0);
                    node1 += dmult * dealii::Point<2>(dispx1, dispy1);
                    node2 += dmult * dealii::Point<2>(dispx2, dispy2);
                    node3 += dmult * dealii::Point<2>(dispx3, dispy3);
                }

                // create triangles
                values.append(PostTriangle(node0, node1, node2, value0, value1, value2));
                values.append(PostTriangle(node1, node3, node2, value1, value3, value2));
            }
        }
    }
}

void PostDataOut::compute_node(dealii::Point<2>& node, const dealii::DataOutBase::Patch<2>* patch,
                               const unsigned int xstep, const unsigned int ystep, const unsigned int zstep,
                               const unsigned int n_subdivisions)
{
    if (patch->points_are_available)
    {
        unsigned int point_no = 0;
        point_no += (n_subdivisions + 1) * ystep;
        for (unsigned int d = 0; d < 2; ++d)
            node[d] = patch->data(patch->data.size(0) - 2 + d, point_no);
    }
    else
    {
        // perform a dim-linear interpolation
        const double stepsize = 1. / n_subdivisions, xfrac = xstep * stepsize;

        node = (patch->vertices[1] * xfrac) + (patch->vertices[0] * (1 - xfrac));
        const double yfrac = ystep * stepsize;

        node *= 1 - yfrac;
        node += ((patch->vertices[3] * xfrac) + (patch->vertices[2] * (1 - xfrac))) * yfrac;
    }
}

// ************************************************************************************************************************

ProblemBase::ProblemBase() :
    m_isMeshing(false),
    m_config(new ProblemConfig(this)),
    m_scene(new Scene(this)),
    m_isNonlinear(false),
    m_hash(0)
{
    m_timeStepLengths.append(0.0);
}

ProblemBase::~ProblemBase()
{
    m_config->clear();

    clearFieldsAndConfig();
    clearFields();

    delete m_scene;
    delete m_config;
}

int ProblemBase::numAdaptiveFields() const
{
    int num = 0;
    foreach(FieldInfo* fieldInfo, m_fieldInfos) if (fieldInfo->adaptivityType() != AdaptivityMethod_None)
        num++;
    return num;
}

int ProblemBase::numTransientFields() const
{
    int num = 0;
    foreach(FieldInfo* fieldInfo, m_fieldInfos) if (fieldInfo->analysisType() == AnalysisType_Transient)
        num++;
    return num;
}

bool ProblemBase::isTransient() const
{
    return numTransientFields() > 0;
}

bool ProblemBase::isHarmonic() const
{
    foreach(FieldInfo* fieldInfo, m_fieldInfos) if (fieldInfo->analysisType() == AnalysisType_Harmonic)
        return true;

    return false;
}

bool ProblemBase::determineIsNonlinear() const
{
    foreach(FieldInfo* fieldInfo, m_fieldInfos) if (fieldInfo->linearityType() != LinearityType_Linear)
        return true;

    return false;
}

QStringList ProblemBase::checkAndApplyParameters(const QMap<QString, ProblemParameter>& parameters)
{
    // store original parameters
    QMap<QString, ProblemParameter> parametersOriginal = m_config->parameters()->items();

    // set new parameters
    m_config->parameters()->set(parameters.values());

    // apply new parameters
    QStringList err = checkAndParameters(true);
    bool successfulRun = err.isEmpty();

    // restore original parameters
    if (!successfulRun)
    {
        m_config->parameters()->set(parametersOriginal.values());

        // apply original parameters
        checkAndParameters();
    }
    else
    {
        // control geometry
        m_scene->invalidate();
    }

    return err;
}

QStringList ProblemBase::checkAndParameters(bool printError)
{
    QStringList str;

    // nodes
    foreach(SceneNode *node, m_scene->nodes->items())
    {
        if (node->pointValue().x().isNumber() && node->pointValue().y().isNumber())
        {
        }
        else
        {
            if (!node->pointValue().x().isEvaluated())
            {
                QString err = QObject::tr("Node %1 - Coord. %2: %3").
                              arg(m_scene->nodes->items().indexOf(node)).
                              arg(m_config->labelX()).
                              arg(node->pointValue().x().error());

                str.append(err);
                if (printError)
                    Agros::log()->printError(QObject::tr("Parameters"), err);
            }
            if (!node->pointValue().y().isEvaluated())
            {
                QString err = tr("Node %1 - Coord. %2: %3").
                              arg(m_scene->nodes->items().indexOf(node)).
                              arg(m_config->labelY()).
                              arg(node->pointValue().y().error());

                str.append(err);
                if (printError)
                    Agros::log()->printError(QObject::tr("Parameters"), err);
            }
        }
    }

    // edges
    foreach(SceneFace *edge, m_scene->faces->items())
    {
        if (edge->angleValue().isNumber())
        {
        }
        else
        {
            if (!edge->angleValue().isEvaluated())
            {
                QString err = QObject::tr("Edge %1: %2").
                              arg(m_scene->faces->items().indexOf(edge)).
                              arg(edge->angleValue().error());

                str.append(err);
                if (printError)
                    Agros::log()->printError(QObject::tr("Parameters"), err);
            }
        }
    }

    // labels
    foreach(SceneLabel *label, m_scene->labels->items())
    {
        if (label->pointValue().x().isNumber() && label->pointValue().y().isNumber())
        {
        }
        else
        {
            if (!label->pointValue().x().isEvaluated())
            {
                QString err = QObject::tr("Label %1 - Coord. %2: %3").
                              arg(m_scene->labels->items().indexOf(label)).
                              arg(m_config->labelX()).
                              arg(label->pointValue().x().error());

                str.append(err);
                if (printError)
                    Agros::log()->printError(QObject::tr("Parameters"), err);
            }
            if (!label->pointValue().y().isEvaluated())
            {
                QString err = QObject::tr("Label %1 - Coord. %2: %3").
                              arg(m_scene->labels->items().indexOf(label)).
                              arg(m_config->labelY()).
                              arg(label->pointValue().y().error());

                str.append(err);
                if (printError)
                    Agros::log()->printError(QObject::tr("Parameters"), err);
            }
        }
    }

    // check materials
    foreach(SceneMaterial* material, m_scene->materials->items())
    {
        foreach(size_t key, material->values().keys())
        {
            if (!material->value(key)->isEvaluated())
            {
                QString err = QObject::tr("Material %1: %2").
                              arg(material->name()).arg(material->value(key).data()->error());

                str.append(err);
                if (printError)
                    Agros::log()->printError(QObject::tr("Parameters"), err);;
            }
        }
    }

    // check boundaries
    foreach(SceneBoundary* boundary, m_scene->boundaries->items())
    {
        foreach(size_t key, boundary->values().keys())
        {
            if (!boundary->value(key)->isEvaluated())
            {
                QString err = QObject::tr("Boundary %1: %2").
                              arg(boundary->name()).arg(boundary->value(key).data()->error());

                str.append(err);
                if (printError)
                    Agros::log()->printError(QObject::tr("Parameters"), err);
            }
        }
    }

    // check frequency
    if (!m_config->value(ProblemConfig::Frequency).value<Value>().isEvaluated())
    {
        QString err = QObject::tr("Frequency %1").
            arg(m_config->value(ProblemConfig::Frequency).value<Value>().error());

        str.append(err);
        if (printError)
            Agros::log()->printError(QObject::tr("Parameters"), err);
    }

    return str;
}

void ProblemBase::clearFields()
{
    // clear scene
    m_scene->clear();

    // clear couplings
    foreach(CouplingInfo* couplingInfo, m_couplingInfos)
        delete couplingInfo;
    m_couplingInfos.clear();

    QMapIterator<QString, FieldInfo*> i(m_fieldInfos);
    while (i.hasNext())
    {
        i.next();
        removeField(i.value());
        delete i.value();
    }
    m_fieldInfos.clear();

    // initial mesh
    m_initialMesh.clear();
    m_initialUnrefinedMesh.clear();
}

void ProblemBase::clearFieldsAndConfig()
{
    // clear config
    m_config->clear();
}

void ProblemBase::addField(FieldInfo* field)
{
    // remove field
    if (hasField(field->fieldId()))
    {
        removeField(m_fieldInfos[field->fieldId()]);
        delete m_fieldInfos[field->fieldId()];
    }

    // add to the collection
    m_fieldInfos[field->fieldId()] = field;

    // couplings
    synchronizeCouplings();
    m_scene->fieldsChange();
}

void ProblemBase::removeField(FieldInfo* field)
{
    // first remove references to markers of this field from all edges and labels
    m_scene->faces->removeFieldMarkers(field);
    m_scene->labels->removeFieldMarkers(field);

    // then remove them from lists of markers - here they are really deleted
    m_scene->boundaries->removeFieldMarkers(field);
    m_scene->materials->removeFieldMarkers(field);

    // remove from the collection
    m_fieldInfos.remove(field->fieldId());

    synchronizeCouplings();
    m_scene->fieldsChange();
}

void ProblemBase::synchronizeCouplings()
{
    // add missing
    foreach(FieldInfo* targetField, m_fieldInfos)
    {
        QStringList couplings = targetField->plugin()->couplings();

        foreach(FieldInfo* sourceField, m_fieldInfos)
        {
            if (sourceField == targetField)
                continue;

            if (couplings.contains(sourceField->fieldId()))
            {
                QPair<QString, QString> fieldInfosPair(sourceField->fieldId(), targetField->fieldId());

                if (!m_couplingInfos.keys().contains(fieldInfosPair))
                {
                    m_couplingInfos[fieldInfosPair] = new CouplingInfo(sourceField->fieldId(),
                                                                       targetField->fieldId(),
                                                                       targetField->plugin()->couplingJson(
                                                                           sourceField->fieldId())->name);
                }
            }
        }
    }

    // remove extra
    foreach(CouplingInfo *couplingInfo, m_couplingInfos)
    {
        if (!(m_fieldInfos.contains(couplingInfo->sourceFieldId()) &&
            m_fieldInfos.contains(couplingInfo->targetFieldId()) &&
            isCouplingAvailable(couplingInfo->sourceFieldId(),
                                m_fieldInfos[couplingInfo->sourceFieldId()]->analysisType(),
                                couplingInfo->targetFieldId(),
                                m_fieldInfos[couplingInfo->targetFieldId()]->analysisType(),
                                CouplingType_Weak)))
        {
            QPair<QString, QString> key = QPair<QString, QString>(couplingInfo->sourceFieldId(),
                                                                  couplingInfo->targetFieldId());
            m_couplingInfos.remove(key);
        }
    }
}

bool ProblemBase::isMeshed() const
{
    if (m_initialMesh.n_active_cells() == 0)
        return false;

    return (m_fieldInfos.size() > 0);
}

bool ProblemBase::mesh()
{
    // TODO: make global check geometry before mesh() and solve()
    if (m_fieldInfos.isEmpty())
    {
        Agros::log()->printError(tr("Mesh"), tr("No fields defined"));
        return false;
    }

    m_isMeshing = true;

    try
    {
        m_initialMesh.clear();
        m_initialUnrefinedMesh.clear();

        Agros::log()->printMessage(QObject::tr("Mesh Generator"), QObject::tr("Initial mesh generation"));

        // check geometry
        m_scene->checkGeometryResult();
        m_scene->checkGeometryAssignement();

        QSharedPointer<MeshGenerator> meshGenerator;
        switch (config()->meshType())
        {
        case MeshType_Triangle_QuadFineDivision:
            meshGenerator = QSharedPointer<MeshGenerator>(new MeshGeneratorTriangle(this));
            break;
        default:
            Agros::log()->printError(tr("Mesh generator error"),
                                     tr("Mesh generator '%1' is not supported.").arg(
                                         meshTypeString(config()->meshType())));
            assert(0);
            break;
        }

        if (meshGenerator->mesh())
        {
            // load mesh
            m_initialMesh.copy_triangulation(meshGenerator->triangulation());
            // this is just a workaround for the problem in deal user data are not preserved on faces after refinement
            m_initialUnrefinedMesh.copy_triangulation(m_initialMesh);

            Agros::log()->printDebug(tr("Mesh Generator"), tr("Reading initial mesh from memory"));

            m_isMeshing = false;
            return true;
        }
    }
    catch (AgrosGeometryException& e)
    {
        // this assumes that all the code in Hermes and Agros is exception-safe
        // todo:  this is almost certainly not the case, at least for Agros. It should be further investigated
        Agros::log()->printError(tr("Geometry"), QString("%1").arg(e.what()));
    }
    catch (AgrosMeshException& e)
    {
        // this assumes that all the code in Hermes and Agros is exception-safe
        // todo:  this is almost certainly not the case, at least for Agros. It should be further investigated
        Agros::log()->printError(tr("Mesh"), QString("%1").arg(e.what()));
    }
    catch (AgrosException& e)
    {
        // todo: dangerous
        // catching all other exceptions. This is not safe at all
        Agros::log()->printWarning(tr("Mesh"), e.what());
    }
    catch (dealii::ExceptionBase& e)
    {
        qDebug() << e.what();
        Agros::log()->printWarning(tr("Mesh (deal.II)"), e.what());
    }
    catch (...)
    {
        // todo: dangerous
        // catching all other exceptions. This is not safe at all
        Agros::log()->printWarning(tr("Mesh"), tr("An unknown exception occurred and has been ignored"));
        qDebug() << "Mesh: An unknown exception occurred and has been ignored";
    }

    m_isMeshing = false;
    return false;
}

void ProblemBase::readInitialMeshFromFile(const QString& problemDir)
{
    try
    {
        // load initial mesh file
        QString fnMesh = QString("%1/%2/mesh_initial.msh").arg(cacheProblemDir()).arg(problemDir);
        std::ifstream ifsMesh(fnMesh.toStdString());
        boost::archive::text_iarchive sbiMesh(ifsMesh);
        m_initialMesh.load(sbiMesh, 0);

        // this is just a workaround for the problem in deal user data are not preserved on faces after refinement
        m_initialUnrefinedMesh.copy_triangulation(m_initialMesh);

        Agros::log()->printDebug(tr("Mesh Generator"), tr("Reading initial mesh from disk"));
    }
    catch (boost::archive::archive_exception const& e)
    {
        qInfo() << e.what();
        throw AgrosException(tr("Reading initial mesh from disk failed: %1").arg(e.what()));
    }
}

void ProblemBase::readProblemFromJson(const QString& fileName)
{
    // QTime time;
    // time.start();

    // clear scene
    m_scene->clear();

    QFile file(fileName.isEmpty() ? problemFileName() : fileName);

    if (!file.exists())
        return;

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << QString("Couldn't open json problem file '%1'.").arg(fileName);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject rootJson = doc.object();

    readProblemFromJsonInternal(rootJson);
    // qDebug() << "readProblemFromJson" << time.elapsed();
}

void ProblemBase::importProblemFromA2D(const QString& fileName)
{
    qInfo() << "Import from A2D file: " << fileName;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << QString("Couldn't open a2d problem file '%1'.").arg(fileName);
        return;
    }

    QDomDocument doc;
    doc.setContent(&file);

    // clear fields and config
    clearFieldsAndConfig();

    try
    {
        // Access elements and attributes using QDomElement, QDomNodeList, etc.
        QDomElement rootElement = doc.documentElement();
        if (rootElement.isNull()) {
            qDebug() << "Invalid XML: No root element found";
            return;
        }

        // problem
        QDomElement problemElement = rootElement.elementsByTagName("problem").at(0).toElement();

        m_config->setValue(ProblemConfig::Type::Coordinate, coordinateTypeFromStringKey(problemElement.attribute("coordinate_type")));
        m_config->setValue(ProblemConfig::Type::Mesh, meshTypeFromStringKey(problemElement.attribute("mesh_type")));

        // parameters
        QDomNodeList configElements = rootElement.elementsByTagName("config").at(0).toElement().elementsByTagName("item");
        for (int i = 0; i < configElements.count(); ++i)
        {
            QDomElement problemConfigElement = configElements.at(i).toElement();

            if (problemConfigElement.attribute("key") == "Problem_StartupScript")
            {
                QList<ProblemParameter> parameters;

                // read script
                QStringList script = problemConfigElement.attribute("value").split("\n");
                for (int j = 0; j < script.count(); j++)
                {
                    if (!script[j].isEmpty())
                    {
                        QStringList parts = script[j].split("=");
                        if (parts.count() != 2)
                        {
                            qWarning() << "Invalid script line: " << script[j];
                            continue;
                        }

                        QString name = parts[0].trimmed();
                        bool toDobleOk = false;
                        double value = parts[1].trimmed().toDouble(&toDobleOk);

                        if (toDobleOk)
                            parameters.append(ProblemParameter(name, value));
                    }
                }

                m_config->parameters()->set(parameters);
            }
        }

        // problem config
        QDomNodeList problemConfigNodes = problemElement.elementsByTagName("problem_config").at(0).toElement().elementsByTagName("problem_item");
        for (int i = 0; i < problemConfigNodes.count(); ++i)
        {
            QDomElement problemConfigElement = problemConfigNodes.at(i).toElement();

            QString problem_key = problemConfigElement.attribute("problem_key");

            if (problem_key == "Frequency")
                m_config->setValue(ProblemConfig::Type::Frequency, Value(this, problemConfigElement.attribute("problem_value").toDouble()));
            else if (problem_key == "TimeMethod")
                m_config->setValue(ProblemConfig::Type::TimeMethod, timeStepMethodFromStringKey(problemConfigElement.attribute("problem_value")));
            else if (problem_key == "TimeMethodTolerance")
                m_config->setValue(ProblemConfig::Type::TimeMethodTolerance,  problemConfigElement.attribute("problem_value").toDouble());
            else if (problem_key == "TimeOrder")
                m_config->setValue(ProblemConfig::Type::TimeOrder, problemConfigElement.attribute("problem_value").toInt());
            else if (problem_key == "TimeSteps")
            {
                m_config->setValue(ProblemConfig::Type::TimeInitialStepSize, problemConfigElement.attribute("problem_value").toInt());
                m_config->setValue(ProblemConfig::Type::TimeConstantTimeSteps, problemConfigElement.attribute("problem_value").toInt());
            }
            else if (problem_key == "TimeTotal")
                m_config->setValue(ProblemConfig::Type::TimeTotal, problemConfigElement.attribute("problem_value").toDouble());
        }

        // geometry
        QDomElement geometryElements = rootElement.elementsByTagName("geometry").at(0).toElement();

        // nodes
        QDomNodeList nodesElements = geometryElements.elementsByTagName("nodes").at(0).childNodes();
        for (int i = 0; i < nodesElements.count(); i++)
        {
            QDomElement nodeElement = nodesElements.at(i).toElement();

            Value x = Value(this, nodeElement.attribute("x").toDouble());
            if (!x.isEvaluated()) throw AgrosException(x.error());

            Value y = Value(this, nodeElement.attribute("y").toDouble());
            if (!y.isEvaluated()) throw AgrosException(y.error());

            m_scene->addNode(new SceneNode(m_scene, PointValue(x, y)));
            if (i != m_scene->nodes->count() - 1)
                throw AgrosException(
                    tr("Node with coordinates (%1, %2) is too close to an existing node.").arg(nodeElement.attribute("x")).arg(
                        nodeElement.attribute("y")));
        }

        // edges
        QDomNodeList edgeElements = geometryElements.elementsByTagName("edges").at(0).childNodes();
        for (int i = 0; i < edgeElements.count(); i++)
        {
            QDomElement edgeElement = edgeElements.at(i).toElement();

            QString edgeId = edgeElement.attribute("id");
            QString edgeType = edgeElement.attribute("type");

            SceneNode* nodeFrom = m_scene->nodes->at(edgeElement.attribute("start").toInt());
            SceneNode* nodeTo = m_scene->nodes->at(edgeElement.attribute("end").toInt());

            Value angle = Value(this, edgeElement.attribute("angle").toDouble());
            if (!angle.isEvaluated()) throw AgrosException(angle.error());

            if (angle.number() < 0.0) angle.setNumber(0.0);
            if (angle.number() > 90.0) angle.setNumber(90.0);

            // int segments = edgeElement.attribute("segments").toInt();
            int segments = angle.number() > 0.0 ? 10 : 0;

            m_scene->addFace(new SceneFace(m_scene, nodeFrom, nodeTo, angle, segments));
        }

        // labels
        QDomNodeList labelElements = geometryElements.elementsByTagName("labels").at(0).childNodes();
        for (int i = 0; i < labelElements.count(); i++)
        {
            QDomElement labelElement = labelElements.at(i).toElement();

            Value x = Value(this, labelElement.attribute("x").toDouble());
            if (!x.isEvaluated()) throw AgrosException(x.error());

            Value y = Value(this, labelElement.attribute("y").toDouble());
            if (!y.isEvaluated()) throw AgrosException(y.error());

            double area = labelElement.attribute("area").toDouble();

            m_scene->addLabel(new SceneLabel(m_scene, PointValue(x, y), area));
        }

        // fields
        QDomNodeList fieldsElements = problemElement.elementsByTagName("fields").at(0).childNodes();
        for (int i = 0; i < fieldsElements.count(); i++)
        {
            QDomElement fieldElement = fieldsElements.at(i).toElement();

            FieldInfo *fieldInfo = new FieldInfo(fieldElement.attribute("field_id"));

            // settings
            fieldInfo->setAnalysisType(analysisTypeFromStringKey(fieldElement.attribute("analysis_type")));
            fieldInfo->setAdaptivityType(adaptivityTypeFromStringKey(fieldElement.attribute("adaptivity_type")));
            fieldInfo->setLinearityType(linearityTypeFromStringKey(fieldElement.attribute("linearity_type")));
            fieldInfo->setMatrixSolver(SOLVER_PLUGIN);

            // config
            QDomNodeList configElements = fieldElement.elementsByTagName("field_config").at(0).childNodes();
            for (int j = 0; j < configElements.size(); j++)
            {
                QDomElement configElement = configElements.at(j).toElement();

                FieldInfo::Type type = fieldInfo->stringKeyToType(configElement.attribute("field_key"));
                if (type != FieldInfo::Type::Unknown)
                {
                    if (fieldInfo->value(type).typeId() == QVariant::StringList)
                        fieldInfo->setValue(type, configElement.attribute("field_value").split("|"));
                    else if (fieldInfo->value(type).typeId() == QVariant::Bool)
                        fieldInfo->setValue(type, configElement.attribute("field_value").toInt());
                    else if (fieldInfo->value(type).typeId() == QVariant::String)
                        fieldInfo->setValue(type, configElement.attribute("field_value"));
                    else if (fieldInfo->value(type).typeId() == QVariant::Double)
                        fieldInfo->setValue(type, configElement.attribute("field_value").toDouble());
                    else if (fieldInfo->value(type).typeId() == QVariant::Int)
                        fieldInfo->setValue(type, configElement.attribute("field_value").toInt());
                }
            }

            // polynomial order
            QDomNodeList polynomialOrdersElements = fieldElement.elementsByTagName("polynomial_orders").at(0).childNodes();
            for (int j = 0; j < polynomialOrdersElements.size(); j++)
            {
                QDomElement polynomialOrderElement = polynomialOrdersElements.at(j).toElement();

                SceneLabel* label = m_scene->labels->items().at(polynomialOrderElement.attribute("polynomial_order_id").toInt());

                fieldInfo->setLabelPolynomialOrder(label, polynomialOrderElement.attribute("polynomial_order_number").toInt());
            }

            // boundary conditions
            QDomNodeList boundariesElements = fieldElement.elementsByTagName("boundaries").at(0).childNodes();
            for (int j = 0; j < boundariesElements.size(); j++)
            {
                QDomElement boundaryElement = boundariesElements.at(j).toElement();

                // read marker
                SceneBoundary* bound = new SceneBoundary(m_scene,
                                                         fieldInfo,
                                                         boundaryElement.attribute("name"),
                                                         boundaryElement.attribute("type"));

                // default values
                Module::BoundaryType boundaryType = fieldInfo->boundaryType(boundaryElement.attribute("type"));
                foreach(Module::BoundaryTypeVariable variable, boundaryType.variables())
                    bound->setValue(variable.id(), Value(this));

                QDomNodeList boundaryTypesElements = boundaryElement.elementsByTagName("boundary_types").at(0).childNodes();
                for (int k = 0; k < boundaryTypesElements.size(); k++)
                {
                    QDomElement typeElement = boundaryTypesElements.at(k).toElement();

                    Value m = Value(this, typeElement.attribute("value"));
                    if (!m.isEvaluated()) throw AgrosException(m.error());

                    bound->setValue(typeElement.attribute("key"), m);
                }

                m_scene->addBoundary(bound);

                // add boundary to the edge marker
                QDomNodeList materialEdgesElements = boundaryElement.elementsByTagName("boundary_edges").at(0).childNodes();
                for (int k = 0; k < materialEdgesElements.size(); k++)
                {
                    int face = materialEdgesElements.at(k).toElement().attribute("id").toInt();

                    m_scene->faces->at(face)->addMarker(bound);
                }
            }

            // materials
            QDomNodeList materialsElements = fieldElement.elementsByTagName("materials").at(0).childNodes();
            for (int j = 0; j < materialsElements.size(); j++)
            {
                QDomElement materialElement = materialsElements.at(j).toElement();

                // read marker
                SceneMaterial* mat = new SceneMaterial(m_scene,
                                                       fieldInfo,
                                                       materialElement.attribute("name"));

                // default values
                foreach(Module::MaterialTypeVariable variable, fieldInfo->materialTypeVariables())
                    mat->setValue(variable.id(), Value(this));

                QDomNodeList materialTypesElements = materialElement.elementsByTagName("material_types").at(0).childNodes();
                for (int k = 0; k < materialTypesElements.size(); k++)
                {
                    QDomElement typeElement = materialTypesElements.at(k).toElement();

                    Value m = Value(this, typeElement.attribute("value"));
                    if (!m.isEvaluated()) throw AgrosException(m.error());

                    mat->setValue(typeElement.attribute("key"), m);
                }

                m_scene->addMaterial(mat);

                // add label to the label marker
                QDomNodeList materialLabelsElements = materialElement.elementsByTagName("material_labels").at(0).childNodes();
                for (int k = 0; k < materialLabelsElements.size(); k++)
                {
                    int label = materialLabelsElements.at(k).toElement().attribute("id").toInt();

                    m_scene->labels->at(label)->addMarker(mat);
                }
            }

            // add missing none markers
            m_scene->faces->addMissingFieldMarkers(fieldInfo);
            m_scene->labels->addMissingFieldMarkers(fieldInfo);

            // add field
            addField(fieldInfo);
        }

        // couplings
        synchronizeCouplings();

        QDomNodeList couplingsElements = rootElement.elementsByTagName("couplings").at(0).toElement().elementsByTagName("coupling");
        for (int i = 0; i < couplingsElements.count(); ++i)
        {
            QDomElement couplingElement = couplingsElements.at(i).toElement();

            // couplings
            FieldInfo* fieldInfo = new FieldInfo(couplingElement.attribute("target_fieldid"));
            QStringList couplings = fieldInfo->plugin()->couplings();

            if (couplings.contains(couplingElement.attribute("source_fieldid")))
            {
                CouplingInfo* cpl = couplingInfo(couplingElement.attribute("source_fieldid"),
                                                 couplingElement.attribute("target_fieldid"));
                cpl->setCouplingType(couplingTypeFromStringKey("weak"));
            }
        }

        // invalidate scene (parameter update)
        m_scene->invalidate();
    }
    catch (AgrosException& e)
    {
        Agros::log()->printError(QObject::tr("Import a2d"), e.what());
    }
    catch (std::exception& e)
    {
        Agros::log()->printError(QObject::tr("Import a2d"), e.what());
    }
}

void ProblemBase::writeProblemToJson(const QString& fileName)
{
    // QTime time;
    // time.start();

    QFile file(fileName.isEmpty() ? problemFileName() : fileName);

    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << QString("Couldn't write json problem file '%1'.").arg(fileName);
        return;
    }

    // root object
    QJsonObject rootJson;
    rootJson[VERSION] = 1;

    writeProblemToJsonInternal(rootJson);

    // save to file
    QJsonDocument doc(rootJson);
    file.write(doc.toJson(QJsonDocument::Indented));
    // file.write(doc.toJson(QJsonDocument::Compact));

    // qDebug() << "writeProblemToJson" << time.elapsed();
}

void ProblemBase::readProblemFromJsonInternal(QJsonObject& rootJson)
{
    // config
    QJsonObject configJson = rootJson[CONFIG].toObject();
    m_config->load(configJson);

    // geometry
    QJsonObject geometryJson = rootJson[GEOMETRY].toObject();

    // nodes
    QJsonArray nodesJson = geometryJson[NODES].toArray();
    for (int i = 0; i < nodesJson.size(); i++)
    {
        QJsonObject nodeJson = nodesJson[i].toObject();

        Value x = Value(this, nodeJson[X].toString());
        if (!x.isEvaluated()) throw AgrosException(x.error());

        Value y = Value(this, nodeJson[Y].toString());
        if (!y.isEvaluated()) throw AgrosException(y.error());

        m_scene->addNode(new SceneNode(m_scene, PointValue(x, y)));
        if (i != m_scene->nodes->count() - 1)
            throw AgrosException(
                tr("Node with coordinates (%1, %2) is too close to an existing node.").arg(nodeJson[X].toString()).arg(
                    nodeJson[Y].toString()));
    }

    // qWarning() << "nodes" << m_scene->nodes->count();

    // edges
    QJsonArray facesJson = geometryJson[FACES].toArray();
    for (int i = 0; i < facesJson.size(); i++)
    {
        QJsonObject faceJson = facesJson[i].toObject();

        QJsonArray listJson = faceJson[LIST].toArray();
        SceneNode* nodeFrom = m_scene->nodes->at(listJson[0].toInt());
        SceneNode* nodeTo = m_scene->nodes->at(listJson[1].toInt());

        int segments = faceJson[SEGMENTS].toInt();

        Value angle = Value(this, faceJson[ANGLE].toString());
        if (!angle.isEvaluated()) throw AgrosException(angle.error());

        if (angle.number() < 0.0) angle.setNumber(0.0);
        if (angle.number() > 90.0) angle.setNumber(90.0);

        m_scene->addFace(new SceneFace(m_scene, nodeFrom, nodeTo, angle, segments));
    }

    // qWarning() << "faces" << m_scene->faces->count();

    // labels
    QJsonArray labelsJson = geometryJson[LABELS].toArray();
    for (int i = 0; i < labelsJson.size(); i++)
    {
        QJsonObject labelJson = labelsJson[i].toObject();

        Value x = Value(this, labelJson[X].toString());
        if (!x.isEvaluated()) throw AgrosException(x.error());

        Value y = Value(this, labelJson[Y].toString());
        if (!y.isEvaluated()) throw AgrosException(y.error());

        double area = labelJson[AREA].toDouble();

        m_scene->addLabel(new SceneLabel(m_scene, PointValue(x, y), area));
    }

    // qWarning() << "labels" << m_scene->labels->count();

    // fields
    QJsonArray fieldsJson = rootJson[FIELDS].toArray();
    for (int i = 0; i < fieldsJson.size(); i++)
    {
        QJsonObject fieldJson = fieldsJson[i].toObject();

        FieldInfo* fieldInfo = new FieldInfo(fieldJson[FIELDID].toString());

        // settings
        QJsonObject settingsJson = fieldJson[SETTINGS].toObject();
        fieldInfo->load(settingsJson);

        // polynomial order
        QJsonArray labelOrderJson = fieldJson[REFINEMENT_ORDER].toArray();
        for (int i = 0; i < labelOrderJson.size(); i++)
        {
            QJsonObject order = labelOrderJson[i].toObject();

            fieldInfo->setLabelPolynomialOrder(m_scene->labels->items().at(order[ID].toInt()), order[VALUE].toInt());
        }

        // boundary conditions
        QJsonArray boundariesJson = fieldJson[BOUNDARIES].toArray();
        for (int j = 0; j < boundariesJson.size(); j++)
        {
            QJsonObject boundaryJson = boundariesJson[j].toObject();

            // read marker
            SceneBoundary* bound = new SceneBoundary(m_scene,
                                                     fieldInfo,
                                                     boundaryJson[NAME].toString(),
                                                     boundaryJson[TYPE].toString());

            // default values
            Module::BoundaryType boundaryType = fieldInfo->boundaryType(boundaryJson[TYPE].toString());
            foreach(Module::BoundaryTypeVariable variable, boundaryType.variables())
                bound->setValue(variable.id(), Value(this));

            QJsonArray boundaryTypesJson = boundaryJson[BOUNDARY_TYPES].toArray();
            for (int k = 0; k < boundaryTypesJson.size(); k++)
            {
                QJsonObject type = boundaryTypesJson[k].toObject();

                Value m = Value(this, type[VALUE].toString());
                if (!m.isEvaluated()) throw AgrosException(m.error());

                bound->setValue(type[ID].toString(), m);
            }

            m_scene->addBoundary(bound);

            // add boundary to the edge marker
            QJsonArray boundaryFacesJson = boundaryJson[BOUNDARY_FACES].toArray();
            for (unsigned int k = 0; k < boundaryFacesJson.size(); k++)
            {
                int face = boundaryFacesJson[k].toInt();

                m_scene->faces->at(face)->addMarker(bound);
            }
        }

        // materials
        QJsonArray materialsJson = fieldJson[MATERIALS].toArray();
        for (int j = 0; j < materialsJson.size(); j++)
        {
            QJsonObject materialJson = materialsJson[j].toObject();

            // read marker
            SceneMaterial* mat = new SceneMaterial(m_scene,
                                                   fieldInfo,
                                                   materialJson[NAME].toString());

            // default values
            foreach(Module::MaterialTypeVariable variable, fieldInfo->materialTypeVariables())
                mat->setValue(variable.id(), Value(this));

            QJsonArray materialTypesJson = materialJson[MATERIAL_TYPES].toArray();
            for (int k = 0; k < materialTypesJson.size(); k++)
            {
                QJsonObject type = materialTypesJson[k].toObject();

                Value m = Value(this, type[VALUE].toString());
                if (!m.isEvaluated()) throw AgrosException(m.error());

                mat->setValue(type[ID].toString(), m);
            }

            m_scene->addMaterial(mat);

            // add label to the label marker
            QJsonArray materialLabelsJson = materialJson[MATERIAL_LABELS].toArray();
            for (unsigned int k = 0; k < materialLabelsJson.size(); k++)
            {
                int label = materialLabelsJson[k].toInt();

                m_scene->labels->at(label)->addMarker(mat);
            }
        }

        // add missing none markers
        m_scene->faces->addMissingFieldMarkers(fieldInfo);
        m_scene->labels->addMissingFieldMarkers(fieldInfo);

        // add field
        addField(fieldInfo);
    }

    // couplings
    synchronizeCouplings();

    QJsonArray couplingsJson = rootJson[COUPLINGS].toArray();
    for (int i = 0; i < couplingsJson.size(); i++)
    {
        QJsonObject couplingJson = couplingsJson[i].toObject();

        // couplings
        FieldInfo* fieldInfo = new FieldInfo(couplingJson[TARGET_FIELDID].toString());
        QStringList couplings = fieldInfo->plugin()->couplings();

        if (couplings.contains(couplingJson[SOURCE_FIELDID].toString()))
        {
            CouplingInfo* cpl = couplingInfo(couplingJson[SOURCE_FIELDID].toString(),
                                             couplingJson[TARGET_FIELDID].toString());
            cpl->setCouplingType(couplingTypeFromStringKey(couplingJson[TYPE].toString()));
        }
    }

    // invalidate scene (parameter update)
    m_scene->invalidate();
}

void ProblemBase::writeProblemToJsonInternal(QJsonObject& rootJson)
{
    // config
    QJsonObject configJson;
    m_config->save(configJson);
    rootJson[CONFIG] = configJson;

    // geometry
    QJsonObject geometryJson;

    // nodes
    QJsonArray nodesJson;
    int inode = 0;
    foreach(SceneNode *node, m_scene->nodes->items())
    {
        QJsonObject nodeJson;

        nodeJson[ID] = inode;
        nodeJson[X] = node->pointValue().x().toString();
        nodeJson[Y] = node->pointValue().y().toString();

        nodesJson.append(nodeJson);

        inode++;
    }
    geometryJson[NODES] = nodesJson;

    // edges
    QJsonArray edgesJson;
    int iedge = 0;
    foreach(SceneFace *edge, m_scene->faces->items())
    {
        QJsonObject edgeJson;

        edgeJson[ID] = iedge;
        edgeJson[ANGLE] = edge->angleValue().toString();
        edgeJson[SEGMENTS] = edge->segments();

        QJsonArray listJson;
        listJson.append(m_scene->nodes->items().indexOf(edge->nodeStart()));
        listJson.append(m_scene->nodes->items().indexOf(edge->nodeEnd()));
        edgeJson[LIST] = listJson;

        edgesJson.append(edgeJson);

        iedge++;
    }
    geometryJson[FACES] = edgesJson;

    // labels
    QJsonArray labelsJson;
    int ilabel = 0;
    foreach(SceneLabel *label, m_scene->labels->items())
    {
        QJsonObject labelJson;

        labelJson[ID] = inode;
        labelJson[X] = label->pointValue().x().toString();
        labelJson[Y] = label->pointValue().y().toString();
        labelJson[AREA] = label->area();

        labelsJson.append(labelJson);

        ilabel++;
    }
    geometryJson[LABELS] = labelsJson;

    // geometry
    rootJson[GEOMETRY] = geometryJson;

    // fields
    QJsonArray fieldsJson;
    foreach(FieldInfo *fieldInfo, fieldInfos())
    {
        QJsonObject fieldJson;
        fieldJson[FIELDID] = fieldInfo->fieldId();

        // settings
        QJsonObject settingsJson;
        fieldInfo->save(settingsJson);
        fieldJson[SETTINGS] = settingsJson;

        // polynomial order
        QJsonArray labelOrderJson;
        QMapIterator<SceneLabel*, int> labelOrderIterator(fieldInfo->labelsPolynomialOrder());
        while (labelOrderIterator.hasNext())
        {
            labelOrderIterator.next();

            QJsonObject order;
            order[ID] = m_scene->labels->items().indexOf(labelOrderIterator.key());
            order[VALUE] = labelOrderIterator.value();

            labelOrderJson.append(order);
        }
        fieldJson[REFINEMENT_ORDER] = labelOrderJson;

        // boundaries
        QJsonArray boundariesJson;
        int iboundary = 1;
        foreach(SceneBoundary *bound, m_scene->boundaries->filter(fieldInfo).items())
        {
            QJsonObject boundaryJson;

            QJsonArray boundaryFacesJson;
            foreach(SceneFace *edge, m_scene->faces->items())
            {
                if (edge->hasMarker(bound))
                    boundaryFacesJson.append(m_scene->faces->items().indexOf(edge));
            }
            boundaryJson[BOUNDARY_FACES] = boundaryFacesJson;

            QJsonArray boundaryTypesJson;
            const QMap<size_t, QSharedPointer<Value>> values = bound->values();
            for (QMap<size_t, QSharedPointer<Value>>::const_iterator it = values.begin(); it != values.end(); ++it)
            {
                QJsonObject type;
                type[ID] = bound->valueName(it.key());
                type[VALUE] = it.value()->toString();

                boundaryTypesJson.append(type);
            }
            boundaryJson[BOUNDARY_TYPES] = boundaryTypesJson;

            boundaryJson[ID] = iboundary;
            boundaryJson[NAME] = bound->name();
            boundaryJson[TYPE] = (bound->type() == "") ? "none" : bound->type();

            boundariesJson.append(boundaryJson);

            iboundary++;
        }
        fieldJson[BOUNDARIES] = boundariesJson;

        // materials
        QJsonArray materialsJson;
        int imaterial = 1;
        foreach(SceneMaterial *mat, m_scene->materials->filter(fieldInfo).items())
        {
            QJsonObject materialJson;

            QJsonArray materialLabelsJson;
            foreach(SceneLabel *label, m_scene->labels->items())
            {
                if (label->hasMarker(mat))
                    materialLabelsJson.append(m_scene->labels->items().indexOf(label));
            }
            materialJson[MATERIAL_LABELS] = materialLabelsJson;

            QJsonArray materialTypesJson;
            const QMap<size_t, QSharedPointer<Value>> values = mat->values();
            for (QMap<size_t, QSharedPointer<Value>>::const_iterator it = values.begin(); it != values.end(); ++it)
            {
                QJsonObject type;
                type[ID] = mat->valueName(it.key());
                type[VALUE] = it.value()->toString();

                materialTypesJson.append(type);
            }
            materialJson[MATERIAL_TYPES] = materialTypesJson;

            materialJson[ID] = imaterial;
            materialJson[NAME] = mat->name();

            materialsJson.append(materialJson);

            imaterial++;
        }
        fieldJson[MATERIALS] = materialsJson;

        fieldsJson.append(fieldJson);
    }
    rootJson[FIELDS] = fieldsJson;

    // couplings
    QJsonArray couplingsJson;
    foreach(CouplingInfo *couplingInfo, couplingInfos())
    {
        QJsonObject couplingJson;

        couplingJson[ID] = couplingInfo->couplingId();
        couplingJson[TYPE] = couplingTypeToStringKey(couplingInfo->couplingType());
        couplingJson[SOURCE_FIELDID] = couplingInfo->sourceFieldId();
        couplingJson[TARGET_FIELDID] = couplingInfo->targetFieldId();

        couplingsJson.append(couplingJson);
    }
    rootJson[COUPLINGS] = couplingsJson;
}

void ProblemBase::generateHash()
{
    ScriptGenerator scriptGenerator;
    m_hash = qHash(scriptGenerator.createPython());

    // qInfo() << "generateHash: " << m_hash;
}

bool ProblemBase::hasChanged() const
{
    ScriptGenerator scriptGenerator;
    const size_t hash = qHash(scriptGenerator.createPython());

    // qInfo() << "hasChanged: " << m_hash << hash;

    return (m_hash != hash);
}

// computation

Computation::Computation(const QString& problemDir) : ProblemBase(),
                                                      m_lastTimeElapsed(QTime()),
                                                      m_isSolving(false),
                                                      m_abort(false),
                                                      m_isPostprocessingRunning(false),
                                                      m_setting(new PostprocessorSetting(this)),
                                                      m_problemSolver(new ProblemSolver(this)),
                                                      m_solutionStore(new SolutionStore(this)),
                                                      m_results(new ComputationResults()),
                                                      m_postDeal(new PostDeal(this))
{
    if (problemDir.isEmpty())
    {
        // new dir
        QDateTime time(QDateTime::currentDateTime());
        m_problemDir = QString("%1").arg(time.toString("yyyy-MM-dd-hh-mm-ss-zzz"));
    }
    else
    {
        // existing dir
        m_problemDir = problemDir;
    }

    // create dir
    QDir(cacheProblemDir()).mkdir(m_problemDir);
}

Computation::~Computation()
{
    clearSolution();
    removeDirectory(QString("%1/%2").arg(cacheProblemDir(), m_problemDir));

    delete m_setting;
    delete m_problemSolver;
    delete m_solutionStore;
    delete m_results;
    delete m_postDeal;
}

void Computation::readFromProblem()
{
    clearFields();

    // copy config from problem
    m_config->copy(Agros::problem()->config());

    // copy scene from problem
    m_scene->clear();
    m_scene->copy(Agros::problem()->scene());

    foreach(QString fieldId, Agros::problem()->fieldInfos().keys())
    {
        FieldInfo* originFieldInfo = Agros::problem()->fieldInfo(fieldId);
        FieldInfo* fieldInfo = new FieldInfo(fieldId);
        fieldInfo->copy(originFieldInfo);

        // polynomial order
        foreach(SceneLabel *originLabel, originFieldInfo->labelsPolynomialOrder().keys())
        {
            SceneLabel* label = m_scene->labels->items().at(
                Agros::problem()->scene()->labels->items().indexOf(originLabel));

            fieldInfo->setLabelPolynomialOrder(label, originFieldInfo->labelsPolynomialOrder()[originLabel]);
        }

        // boundaries
        foreach(SceneBoundary *originBoundary, Agros::problem()->scene()->boundaries->items())
        {
            if (originBoundary->fieldInfo() != originFieldInfo)
                continue;

            SceneBoundary* bound = new SceneBoundary(m_scene,
                                                     fieldInfo,
                                                     originBoundary->name(),
                                                     originBoundary->type());

            // default values
            Module::BoundaryType boundaryType = fieldInfo->boundaryType(bound->type());
            foreach(Module::BoundaryTypeVariable variable, boundaryType.variables())
                bound->setValue(variable.id(), Value(this));

            // boundaries
            foreach(size_t originValueIndex, originBoundary->values().keys())
            {
                QSharedPointer<Value> originValue = originBoundary->value(originValueIndex);

                bound->setValue(originBoundary->valueName(originValueIndex),
                                Value(this, originValue->text(), originValue->table()));
            }

            // add boundary to the edge marker
            foreach(SceneFace *originFace, Agros::problem()->scene()->faces->items())
            {
                SceneFace* face = m_scene->faces->items().at(
                    Agros::problem()->scene()->faces->items().indexOf(originFace));

                if (originFace->hasMarker(originBoundary))
                    face->addMarker(bound);
            }

            m_scene->addBoundary(bound);
        }

        // materials
        foreach(SceneMaterial *originMaterial, Agros::problem()->scene()->materials->items())
        {
            if (originMaterial->fieldInfo() != originFieldInfo)
                continue;

            SceneMaterial* mat = new SceneMaterial(m_scene,
                                                   fieldInfo,
                                                   originMaterial->name());

            // default values
            foreach(Module::MaterialTypeVariable variable, fieldInfo->materialTypeVariables())
                mat->setValue(variable.id(), Value(this));

            // materials
            foreach(size_t originValueIndex, originMaterial->values().keys())
            {
                QSharedPointer<Value> originValue = originMaterial->value(originValueIndex);

                mat->setValue(originMaterial->valueName(originValueIndex),
                              Value(this, originValue->text(), originValue->table()));
            }

            // add label to the label marker
            foreach(SceneLabel *originLabel, Agros::problem()->scene()->labels->items())
            {
                SceneLabel* label = m_scene->labels->items().at(
                    Agros::problem()->scene()->labels->items().indexOf(originLabel));

                if (originLabel->hasMarker(originMaterial))
                    label->addMarker(mat);
            }

            m_scene->addMaterial(mat);
        }

        // add missing none markers
        m_scene->faces->addMissingFieldMarkers(fieldInfo);
        m_scene->labels->addMissingFieldMarkers(fieldInfo);

        addField(fieldInfo);
    }

    // couplings
    foreach(CouplingInfo *couplingInfo, Agros::problem()->couplingInfos())
    {
        m_couplingInfos[qMakePair(couplingInfo->sourceFieldId(), couplingInfo->targetFieldId())]->setCouplingType(
            couplingInfo->couplingType());
    }

    // default values
    m_scene->invalidate();
}

QString Computation::problemFileName() const
{
    return QString("%1/%2/problem.json").arg(cacheProblemDir()).arg(m_problemDir);
}

QList<double> Computation::timeStepTimes() const
{
    QList<double> times;

    double time = 0.0;
    for (int ts = 0; ts < m_timeStepLengths.size(); ts++)
    {
        time += m_timeStepLengths[ts];
        times.append(time);
    }

    return times;
}

double Computation::timeStepToTotalTime(int timeStepIndex) const
{
    double time = 0.0;
    for (int ts = 0; ts <= timeStepIndex; ts++)
        time += m_timeStepLengths[ts];

    return time;
}

void Computation::setActualTimeStepLength(double timeStep)
{
    m_timeStepLengths.append(timeStep);
}

void Computation::removeLastTimeStepLength()
{
    m_timeStepLengths.removeLast();
}

void Computation::solveInit()
{
    if (fieldInfos().isEmpty())
    {
        Agros::log()->printError(QObject::tr("Solver"), QObject::tr("No fields defined"));
        throw AgrosSolverException(tr("No field defined"));
    }

    // check problem settings
    if (isTransient())
    {
        if (!(m_config->value(ProblemConfig::TimeTotal).toDouble() > 0.0))
            throw AgrosSolverException(tr("Total time is zero"));
        if (!(m_config->value(ProblemConfig::TimeMethodTolerance).toDouble() > 0.0))
            throw AgrosSolverException(tr("Time method tolerance is zero"));
        if (m_config->value(ProblemConfig::TimeInitialStepSize).toDouble() < 0.0)
            throw AgrosSolverException(tr("Initial step size is negative"));
    }

    // invalidate scene (parameter update)
    m_scene->invalidate();

    // control geometry
    m_scene->checkGeometryResult();
    m_scene->checkGeometryAssignement();

    // nonlinearity
    m_isNonlinear = determineIsNonlinear();

    // mesh computation
    if (mesh())
    {
        // save initial mesh
        QString fnMesh = QString("%1/%2/mesh_initial.msh").arg(cacheProblemDir()).arg(m_problemDir);
        std::ofstream ofsMesh(fnMesh.toStdString());
        boost::archive::text_oarchive sbMesh(ofsMesh);
        m_initialMesh.save(sbMesh, 0);

        // set calculation mesh
        setCalculationMesh(m_initialMesh);

        // refine mesh
        int maxNumOfRefinements = 0;
        foreach(FieldInfo *fieldInfo, m_fieldInfos)
            maxNumOfRefinements = std::max(maxNumOfRefinements,
                                           fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt());
        m_calculationMesh.refine_global(maxNumOfRefinements);
    }
    else
    {
        throw AgrosSolverException(tr("Could not create mesh"));
    }

    m_problemSolver->init();
}

void Computation::abortSolving()
{
    m_abort = true;
    Agros::log()->printError(QObject::tr("Solver"), QObject::tr("Aborting calculation..."));
}

void Computation::solve()
{
    if (!isPreparedForAction())
        return;

    if ((m_fieldInfos.size() > 1) && isTransient() && (numAdaptiveFields() >= 1))
    {
        Agros::log()->printError(tr("Solver"),
                                 tr("Space adaptivity for transient coupled problems not possible at the moment."));
        return;
    }

    if (m_fieldInfos.isEmpty())
    {
        Agros::log()->printError(tr("Solver"), tr("No fields defined"));
        return;
    }

    if (Agros::configComputer()->value(Config::Config_LinearSystemSave).toBool())
    {
        Agros::log()->printWarning(tr("Solver"),
                                   tr(
                                       "Matrix and RHS will be saved on the disk and this will slow down the calculation. You may disable it in application settings."));
    }

    try
    {
        m_isSolving = true;

        QElapsedTimer time;
        time.start();

        // clear solution
        clearSolution();

        solveInit();
        assert(isMeshed());

        m_problemSolver->solveProblem();

        m_lastTimeElapsed = milisecondsToTime(time.elapsed());

        // elapsed time
        Agros::log()->printMessage(QObject::tr("Solver"),
                                   QObject::tr("Elapsed time: %1 s").arg(m_lastTimeElapsed.toString("mm:ss.zzz")));

        m_abort = false;
        m_isSolving = false;

        // evaluate results recipes
        Agros::problem()->recipes()->evaluate(this);
    }
    catch (AgrosGeometryException& e)
    {
        Agros::log()->printError(QObject::tr("Geometry"), e.what());
        m_isSolving = false;
        return;
    }
    catch (AgrosSolverException& e)
    {
        qDebug() << e.what();
        Agros::log()->printError(QObject::tr("Solver"), e.what());
        m_isSolving = false;
        return;
    }
    catch (AgrosException& e)
    {
        Agros::log()->printError(QObject::tr("Solver"), e.what());
        m_isSolving = false;
        return;
    }
    catch (std::exception& e)
    {
        Agros::log()->printError(QObject::tr("Solver"), e.what());
        m_isSolving = false;
        return;
    }
    catch (...)
    {
        // TODO: dangerous
        // catching all other exceptions. This is not save at all
        m_isSolving = false;
        Agros::log()->printError(tr("Solver"), tr("An unknown exception occurred in solver and has been ignored"));
        qDebug() << "Solver: An unknown exception occurred and has been ignored";
        return;
    }
}

void Computation::propagateBoundaryMarkers()
{
    dealii::Triangulation<2>::cell_iterator cell_unrefined = m_initialUnrefinedMesh.begin();
    dealii::Triangulation<2>::cell_iterator end_cell_unrefined = m_initialUnrefinedMesh.end();
    dealii::Triangulation<2>::cell_iterator cell_initial = m_initialMesh.begin();
    dealii::Triangulation<2>::cell_iterator cell_calculation = m_calculationMesh.begin();

    for (unsigned int idx = 0; cell_unrefined != end_cell_unrefined; ++cell_initial, ++cell_calculation, ++cell_unrefined, ++idx)
    // loop over all cells, not just active ones
    {
        for (unsigned int f = 0; f < dealii::GeometryInfo<2>::faces_per_cell; f++)
        {
            if (cell_unrefined->face(f)->user_index() != 0)
            {
                cell_initial->face(f)->recursively_set_user_index(cell_unrefined->face(f)->user_index());
                cell_calculation->face(f)->recursively_set_user_index(cell_unrefined->face(f)->user_index());
            }
        }
    }
}

bool Computation::isSolved() const
{
    // return (!Agros::solutionStore()->isEmpty() && !m_isSolving && !m_isMeshing);
    return (!m_solutionStore->isEmpty());
}

void Computation::clearFields()
{
    m_config->clear();

    clearSolution();
    clearResults();

    ProblemBase::clearFields();
}

void Computation::clearSolution()
{
    m_abort = false;

    // m_timeStep = 0;
    m_lastTimeElapsed = QTime();
    m_timeStepLengths.clear();
    m_timeStepLengths.append(0.0);

    QString fnMesh = QString("%1/%2/mesh_initial.msh").arg(cacheProblemDir()).arg(problemDir());
    if (QFile::exists(fnMesh))
        QFile::remove(fnMesh);

    m_calculationMesh.clear();
    m_solutionStore->clear();
}

void Computation::clearResults()
{
    m_results->clear();
}

void Computation::clearFieldsAndConfig()
{
    m_setting->clear();

    clearFields();

    ProblemBase::clearFieldsAndConfig();

    QString fn = QString("%1/%2/problem.json").arg(cacheProblemDir()).arg(m_problemDir);
    if (QFile::exists(fn))
        QFile::remove(fn);
}

void Computation::readProblemFromJsonInternal(QJsonObject& rootJson)
{
    ProblemBase::readProblemFromJsonInternal(rootJson);

    // settings
    QJsonObject settingsJson = rootJson[SETTINGS].toObject();
    m_setting->load(settingsJson);

    // results
    m_results->load(rootJson);
}

void Computation::writeProblemToJsonInternal(QJsonObject& rootJson)
{
    ProblemBase::writeProblemToJsonInternal(rootJson);

    QJsonObject settingsJson;
    m_setting->save(settingsJson);
    rootJson[SETTINGS] = settingsJson;

    // results
    m_results->save(rootJson);
}

// *****************************************************************************************************************************

Problem::Problem() : ProblemBase(),
                     m_studies(new Studies()),
                     m_recipes(new ResultRecipes())
{
}

Problem::~Problem()
{
    delete m_studies;
    delete m_recipes;
}

void Problem::removeField(FieldInfo* field)
{
    // remove recipes
    m_recipes->removeAll(field);

    ProblemBase::removeField(field);
}

QString Problem::problemFileName() const
{
    return QString("%1/problem.json").arg(cacheProblemDir());
}

void Problem::readProblemFromJsonInternal(QJsonObject& rootJson)
{
    ProblemBase::readProblemFromJsonInternal(rootJson);

    // studies
    m_studies->clear();
    QJsonArray studiesJson = rootJson[STUDIES].toArray();
    for (int i = 0; i < studiesJson.size(); i++)
    {
        QJsonObject studyJson = studiesJson[i].toObject();
        StudyType type = studyTypeFromStringKey(studyJson[TYPE].toString());

        Study* study = Study::factory(type);
        study->load(studyJson);

        m_studies->addStudy(study);
    }

    // recipes
    m_recipes->clear();
    QJsonArray recipesJson = rootJson[RECIPES].toArray();
    for (int i = 0; i < recipesJson.size(); i++)
    {
        QJsonObject recipeJson = recipesJson[i].toObject();
        ResultRecipeType type = resultRecipeTypeFromStringKey(recipeJson[TYPE].toString());

        ResultRecipe* recipe = ResultRecipe::factory(type);
        recipe->load(recipeJson);

        m_recipes->addRecipe(recipe);
    }
}

void Problem::writeProblemToJsonInternal(QJsonObject& rootJson)
{
    ProblemBase::writeProblemToJsonInternal(rootJson);

    // studies
    QJsonArray studiesJson;
    foreach(Study *study, m_studies->items())
    {
        QJsonObject studyJson;
        studyJson[TYPE] = studyTypeToStringKey(study->type());
        study->save(studyJson);
        studiesJson.append(studyJson);
    }
    rootJson[STUDIES] = studiesJson;

    // recipes
    QJsonArray recipesJson;
    foreach(ResultRecipe *recipe, m_recipes->items())
    {
        QJsonObject recipeJson;
        recipe->save(recipeJson);
        recipesJson.append(recipeJson);
    }
    rootJson[RECIPES] = recipesJson;
}

QSharedPointer<Computation> Problem::createComputation(bool newComputation)
{
    if (newComputation || m_currentComputation.isNull() || Agros::computations().isEmpty())
    {
        m_currentComputation = QSharedPointer<Computation>(new Computation());
        Agros::addComputation(m_currentComputation->problemDir(), m_currentComputation);
    }
    else
    {
        m_currentComputation->clearFields();
    }

    // read data from problem
    m_currentComputation->readFromProblem();
    m_currentComputation->writeProblemToJson();

    return m_currentComputation;
}

void Problem::setCurrentComputation(QSharedPointer<Computation> computation)
{
    m_currentComputation = computation;
}

void Problem::clearFieldsAndConfig()
{
    // clear all computations
    m_currentComputation.clear();
    Agros::clearComputations();

    ProblemBase::clearFieldsAndConfig();
    clearFields();

    m_recipes->clear();
    m_studies->clear();

    QFile::remove(QString("%1/problem.a2d").arg(cacheProblemDir()));
    QFile::remove(QString("%1/problem.json").arg(cacheProblemDir()));

    m_fileName = "";

    // set checksum
    generateHash();
}

void Problem::readProblemFromAgs(const QString& fileName)
{
    clearFieldsAndConfig();

    QSettings settings;
    QFileInfo fileInfo(fileName);
    if (fileInfo.absoluteDir() != tempProblemDir() && !fileName.contains("resources/examples"))
        settings.setValue("General/LastProblemDir", fileInfo.absolutePath());

    JlCompress::extractDir(fileName, cacheProblemDir());

    // QTime time;
    // time.start();
    // read solutions
    QDirIterator it(cacheProblemDir(), QDir::Dirs, QDirIterator::NoIteratorFlags);
    while (it.hasNext())
    {
        QString dir = it.next();

        // skip . and ..
        QFileInfo fileInfo(dir);
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        QString problemDir = fileInfo.fileName();

        if (QFile::exists(problemFileName()))
        {
            QSharedPointer<Computation> computation = QSharedPointer<Computation>(new Computation(problemDir));
            Agros::addComputation(computation->problemDir(), computation);

            Agros::log()->printMessage(tr("Problem"), tr("Loading solution from disk: %1").arg(problemDir));

            computation->readProblemFromJson();

            // read results
            if (computation->solutionStore()->loadRunTimeDetails())
            {
                // read mesh file
                try
                {
                    computation->readInitialMeshFromFile(computation->problemDir());
                    computation->setCalculationMesh(computation->initialMesh());
                }
                catch (AgrosException& e)
                {
                    computation->clearSolution();
                    Agros::log()->printError(tr("Mesh"), tr("Initial mesh is corrupted (%1)").arg(e.what()));
                }
            }
        }
    }
    // qDebug() << "read solutions" << time.elapsed();

    // read problem
    readProblemFromJson();

    // // remove a2d
    // if (QFile::exists(problemA2D))
    //     QFile::remove(problemA2D);

    m_fileName = QFileInfo(fileName).absoluteFilePath();

    // set last computation
    if (!Agros::computations().isEmpty())
    {
        if (m_currentComputation.isNull() && Agros::computations().last()->isSolved())
            m_currentComputation = Agros::computations().last();
    }

    // invalidate scene (parameter update)
    m_scene->invalidate();
}

void Problem::writeProblemToFile(const QString& fileName, bool onlyProblemFile)
{
    QSettings settings;
    QFileInfo fileInfo(fileName);
    if (fileInfo.absoluteDir() != tempProblemDir())
    {
        settings.setValue("General/LastProblemDir", fileInfo.absoluteFilePath());
        m_fileName = fileName;
    }

    writeProblemToJson();

    if (onlyProblemFile)
    {
        // only problem file
        JlCompress::compressFiles(fileName, QStringList() << problemFileName());
    }
    else
    {
        // whole directory
        JlCompress::compressDir(fileName, cacheProblemDir());
    }

    // set checksum
    generateHash();
}

void Problem::readProblemFromFile(const QString& fileName)
{
    QFileInfo fileInfo(fileName);

    try
    {
        // load problem
        if (fileInfo.suffix() == "ags")
        {
            readProblemFromAgs(fileName);
        }
        // deprecated
        else if (fileInfo.suffix() == "a2d")
        {
            // copy file to cache
            QFile::remove(QString("%1/problem.a2d").arg(cacheProblemDir()));
            QFile::copy(fileName, QString("%1/problem.a2d").arg(cacheProblemDir()));

            // open file
            importProblemFromA2D(QString("%1/problem.a2d").arg(cacheProblemDir()));

            // set filename
            m_fileName = QString("%1/%2.ags").arg(fileInfo.absolutePath()).arg(fileInfo.baseName());
        }
    }
    catch (AgrosModuleException& e)
    {
        clearFieldsAndConfig();
        Agros::log()->printError(tr("Problem"), e.toString());
    }
    catch (AgrosPluginException& e)
    {
        clearFieldsAndConfig();
        Agros::log()->printError(tr("Problem"), e.toString());
    }
    catch (AgrosException& e)
    {
        clearFieldsAndConfig();
        Agros::log()->printError(tr("Problem"), e.toString());
    }

    // set checksum
    generateHash();
}
