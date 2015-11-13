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
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "problem.h"
#include "problem_config.h"
#include "problem_result.h"

#include "util/global.h"
#include "util/constants.h"
#include "util/xml.h"

#include "field.h"
#include "solutionstore.h"

#include "scene.h"
#include "scenemarker.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "weak_form.h"
#include "coupling.h"
#include "solver.h"
#include "logview.h"

#include "pythonlab/pythonengine.h"
#include "pythonlab/pythonengine_agros.h"

#include "mesh/meshgenerator_triangle.h"
#include "mesh/meshgenerator_cubit.h"
#include "mesh/meshgenerator_gmsh.h"
// #include "mesh/meshgenerator_netgen.h"

#include "../3rdparty/quazip/JlCompress.h"
#include "../resources_source/classes/problem_a2d_31_xml.h"

CalculationThread::CalculationThread(ProblemComputation *parentProblem) : QThread(), m_computation(parentProblem)
{
}

void CalculationThread::startCalculation(CalculationType type)
{
    m_calculationType = type;
    start(QThread::TimeCriticalPriority);
}

void CalculationThread::run()
{
    Agros2D::log()->printHeading(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));

    dealii::deal_II_exceptions::disable_abort_on_exception();

    switch (m_calculationType)
    {
    case CalculationType_Mesh:
        m_computation->mesh(true);
        break;
    case CalculationType_Solve:
        m_computation->solve();
        break;
    case CalculationType_SolveTimeStep:
        assert(0);
        break;
    default:
        assert(0);
    }
}

Problem::Problem()
{
    m_config = new ProblemConfig();
    m_setting = new ProblemSetting();

    m_scene = new Scene(this);

    m_isNonlinear = false;

    m_timeStepLengths.append(0.0);
}

Problem::~Problem()
{
    clearFieldsAndConfig();

    delete m_config;
    delete m_setting;

    delete m_scene;
}

int Problem::numAdaptiveFields() const
{
    int num = 0;
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->adaptivityType() != AdaptivityMethod_None)
            num++;
    return num;
}

int Problem::numTransientFields() const
{
    int num = 0;
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->analysisType() == AnalysisType_Transient)
            num++;
    return num;
}

bool Problem::isTransient() const
{
    return numTransientFields() > 0;
}

bool Problem::isHarmonic() const
{
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->analysisType() == AnalysisType_Harmonic)
            return true;

    return false;
}

bool Problem::determineIsNonlinear() const
{
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->linearityType() != LinearityType_Linear)
            return true;

    return false;
}

bool Problem::checkAndApplyParameters(ParametersType parameters, bool apply)
{
    // store original parameters
    ParametersType parametersOriginal = m_config->value(ProblemConfig::Parameters).value<ParametersType>();

    // set new parameters
    m_config->setValue(ProblemConfig::Parameters, parameters);

    // apply new parameters
    bool successfulRun = applyParametersInternal();

    // restore original parameters
    if (!successfulRun || !apply)
    {
        m_config->setValue(ProblemConfig::Parameters, parametersOriginal);

        // apply original parameters
        applyParametersInternal();
    }

    return successfulRun;
}

// apply parameters from m_config
bool Problem::applyParametersInternal()
{
    bool successfulRun = true;

    // check and apply parameters
    currentPythonEngineAgros()->blockSignals(true);

    // check geometry
    // nodes
    foreach (SceneNode *node, m_scene->nodes->items())
    {
        if (node->pointValue().x().isNumber() && node->pointValue().y().isNumber())
        {
            continue;
        }
        else
        {
            if (!node->pointValue().x().evaluateAtTime(0.0))
            {
                ErrorResult result = currentPythonEngineAgros()->parseError();
                Agros2D::log()->printError(QObject::tr("Parameters"), QObject::tr("Node %1%2: %3").
                                           arg(m_scene->nodes->items().indexOf(node)).
                                           arg(m_config->labelX()).
                                           arg(result.error()));

                successfulRun = false;
            }
            if (!node->pointValue().y().evaluateAtTime(0.0))
            {
                ErrorResult result = currentPythonEngineAgros()->parseError();
                Agros2D::log()->printError(QObject::tr("Parameters"), QObject::tr("Node %1%2: %3").
                                           arg(m_scene->nodes->items().indexOf(node)).
                                           arg(m_config->labelY()).
                                           arg(result.error()));

                successfulRun = false;
            }
        }
    }

    // edges
    foreach (SceneEdge *edge, m_scene->edges->items())
    {
        if (edge->angleValue().isNumber())
        {
            continue;
        }
        else
        {
            if (!edge->angleValue().evaluateAtTime(0.0))
            {
                ErrorResult result = currentPythonEngineAgros()->parseError();
                Agros2D::log()->printError(QObject::tr("Parameters"), QObject::tr("Edge %1: %2").
                                           arg(m_scene->edges->items().indexOf(edge)).
                                           arg(result.error()));

                successfulRun = false;
            }
        }
    }

    // labels
    foreach (SceneLabel *label, m_scene->labels->items())
    {
        if (label->pointValue().x().isNumber() && label->pointValue().y().isNumber())
        {
            continue;
        }
        else
        {
            if (!label->pointValue().x().evaluateAtTime(0.0))
            {
                ErrorResult result = currentPythonEngineAgros()->parseError();
                Agros2D::log()->printError(QObject::tr("Parameters"), QObject::tr("Label %1%2: %3").
                                           arg(m_scene->labels->items().indexOf(label)).
                                           arg(m_config->labelX()).
                                           arg(result.error()));

                successfulRun = false;
            }
            if (!label->pointValue().y().evaluateAtTime(0.0))
            {
                ErrorResult result = currentPythonEngineAgros()->parseError();
                Agros2D::log()->printError(QObject::tr("Parameters"), QObject::tr("Label %1%2: %3").
                                           arg(m_scene->labels->items().indexOf(label)).
                                           arg(m_config->labelY()).
                                           arg(result.error()));

                successfulRun = false;
            }
        }
    }

    // check materials
    foreach (SceneMaterial* material, m_scene->materials->items())
    {
        foreach (uint key, material->values().keys())
        {
            if (!material->evaluate(key, 0.0))
            {
                Agros2D::log()->printError(QObject::tr("Marker"), QObject::tr("Material %1: %2").
                                           arg(key).arg(material->value(key).data()->toString()));
                successfulRun = false;
            }
        }
    }

    // check boundaries
    foreach (SceneBoundary* boundary, m_scene->boundaries->items())
    {
        foreach (uint key, boundary->values().keys())
        {
            if (!boundary->evaluate(key, 0.0))
            {
                Agros2D::log()->printError(QObject::tr("Marker"), QObject::tr("Boundary %1: %2").
                                           arg(key).arg(boundary->value(key).data()->toString()));
                successfulRun = false;
            }
        }
    }

    // check frequency
    if (!m_config->value(ProblemConfig::Frequency).value<Value>().evaluateAtTime(0.0))
    {
        Agros2D::log()->printError(QObject::tr("Frequency"), QObject::tr("Value: %1").
                                   arg(m_config->value(ProblemConfig::Frequency).value<Value>().toString()));
        successfulRun = false;
    }

    // restore parameters
    currentPythonEngineAgros()->blockSignals(false);

    return successfulRun;
}

void Problem::clearFieldsAndConfig()
{
    // clear couplings
    foreach (CouplingInfo* couplingInfo, m_couplingInfos)
        delete couplingInfo;
    m_couplingInfos.clear();

    QMapIterator<QString, FieldInfo *> i(m_fieldInfos);
    while (i.hasNext())
    {
        i.next();
        removeField(i.value());
        delete i.value();
    }
    m_fieldInfos.clear();

    // clear config
    m_config->clear();
    m_setting->clear();

    // clear scene
    m_scene->clear();
}

void Problem::addField(FieldInfo *field)
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

    emit fieldsChanged();
}

void Problem::removeField(FieldInfo *field)
{
    // first remove references to markers of this field from all edges and labels
    m_scene->edges->removeFieldMarkers(field);
    m_scene->labels->removeFieldMarkers(field);

    // then remove them from lists of markers - here they are really deleted
    m_scene->boundaries->removeFieldMarkers(field);
    m_scene->materials->removeFieldMarkers(field);

    // remove from the collection
    m_fieldInfos.remove(field->fieldId());

    synchronizeCouplings();

    currentPythonEngine()->runExpression(QString("agros2d.__remove_field__(\"%1\")").arg(field->fieldId()));

    emit fieldsChanged();
}

void Problem::synchronizeCouplings()
{
    bool changed = false;

    // add missing
    foreach (FieldInfo* sourceField, m_fieldInfos)
    {
        foreach (FieldInfo* targetField, m_fieldInfos)
        {
            if (sourceField == targetField)
                continue;

            if (couplingList()->isCouplingAvailable(sourceField, targetField))
            {
                QPair<FieldInfo*, FieldInfo*> fieldInfosPair(sourceField, targetField);

                if (!m_couplingInfos.keys().contains(fieldInfosPair))
                {
                    m_couplingInfos[fieldInfosPair] = new CouplingInfo(sourceField, targetField);
                    changed = true;
                }
            }
        }
    }

    // remove extra
    foreach (CouplingInfo *couplingInfo, m_couplingInfos)
    {
        if (!(m_fieldInfos.contains(couplingInfo->sourceField()->fieldId()) &&
              m_fieldInfos.contains(couplingInfo->targetField()->fieldId()) &&
              couplingList()->isCouplingAvailable(couplingInfo->sourceField(), couplingInfo->targetField())))
        {
            QPair<FieldInfo *, FieldInfo *> key = QPair<FieldInfo *, FieldInfo *>(couplingInfo->sourceField(), couplingInfo->targetField());
            m_couplingInfos.remove(key);

            changed = true;
        }
    }

    if (changed)
        emit couplingsChanged();
}

void Problem::readProblemFromA2D(const QString &fileName)
{
    Agros2D::log()->printMessage(tr("Problem"), tr("Loading problem from disk: %1").arg(QFileInfo(fileName).baseName()));

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        throw AgrosException(tr("File '%1' cannot be opened (%2).").arg(fileName).arg(file.errorString()));

    QDomDocument doc;
    if (!doc.setContent(&file))
    {
        file.close();
        throw AgrosException(tr("File '%1' is not valid Agros2D data file.").arg(fileName));
    }
    file.close();

    double version = doc.documentElement().attribute("version").toDouble();

    try
    {
        if (version == 3.1)
        {
            readProblemFromA2D31(fileName);
        }
        else
        {
            QString tempFileName = tempProblemDir() + QString("/%1.a2d").arg(QFileInfo(fileName).baseName());
            transformProblem(fileName, tempFileName, version);
            readProblemFromA2D31(tempFileName);
        }
    }
    catch (AgrosException &e)
    {
        Agros2D::log()->printError(tr("Problem"), e.toString());
    }
}

void Problem::readProblemFromA2D31(const QString &fileName)
{
    try
    {
        std::unique_ptr<XMLProblem::document> document_xsd = XMLProblem::document_(compatibleFilename(fileName).toStdString(), xml_schema::flags::dont_validate);
        XMLProblem::document *doc = document_xsd.get();

        // clear scene
        m_scene->clear();

        m_scene->blockSignals(true);
        m_scene->stopInvalidating(true);

        // coordinate type
        m_config->setCoordinateType(coordinateTypeFromStringKey(QString::fromStdString(doc->problem().coordinate_type())));
        // mesh type
        m_config->setMeshType(meshTypeFromStringKey(QString::fromStdString(doc->problem().mesh_type())));

        // problem config
        m_config->load(&doc->problem().problem_config());
        // general config
        m_setting->load(&doc->config());

        // nodes
        for (unsigned int i = 0; i < doc->geometry().nodes().node().size(); i++)
        {
            XMLProblem::node node = doc->geometry().nodes().node().at(i);

            if (node.valuex().present() && node.valuey().present())
            {
                Value x = Value(QString::fromStdString(node.valuex().get()), this);
                if (!x.isEvaluated())
                {
                    ErrorResult result = currentPythonEngineAgros()->parseError();
                    throw AgrosException(result.error());
                }

                Value y = Value(QString::fromStdString(node.valuey().get()), this);
                if (!y.isEvaluated())
                {
                    ErrorResult result = currentPythonEngineAgros()->parseError();
                    throw AgrosException(result.error());
                }

                m_scene->addNode(new SceneNode(m_scene, PointValue(x, y)));
            }
            else
            {
                Point point = Point(node.x(),
                                    node.y());

                m_scene->addNode(new SceneNode(m_scene, point));
            }
        }

        // edges
        for (unsigned int i = 0; i < doc->geometry().edges().edge().size(); i++)
        {
            XMLProblem::edge edge = doc->geometry().edges().edge().at(i);

            SceneNode *nodeFrom = m_scene->nodes->at(edge.start());
            SceneNode *nodeTo = m_scene->nodes->at(edge.end());

            int segments = 3;
            int isCurvilinear = 1;
            if (edge.segments().present())
                segments = edge.segments().get();
            if (edge.is_curvilinear().present())
                isCurvilinear = edge.is_curvilinear().get();

            if (edge.valueangle().present())
            {
                Value angle = Value(QString::fromStdString(edge.valueangle().get()), this);
                if (!angle.isEvaluated())
                {
                    ErrorResult result = currentPythonEngineAgros()->parseError();
                    throw AgrosException(result.error());
                }
                if (angle.number() < 0.0) angle.setNumber(0.0);
                if (angle.number() > 90.0) angle.setNumber(90.0);

                m_scene->addEdge(new SceneEdge(m_scene, nodeFrom, nodeTo, angle, segments, isCurvilinear));
            }
            else
            {
                m_scene->addEdge(new SceneEdge(m_scene, nodeFrom, nodeTo, edge.angle(), segments, isCurvilinear));
            }
        }

        // labels
        for (unsigned int i = 0; i < doc->geometry().labels().label().size(); i++)
        {
            XMLProblem::label label = doc->geometry().labels().label().at(i);

            if (label.valuex().present() && label.valuey().present())
            {
                Value x = Value(QString::fromStdString(label.valuex().get()), this);
                if (!x.isEvaluated())
                {
                    ErrorResult result = currentPythonEngineAgros()->parseError();
                    throw AgrosException(result.error());
                }

                Value y = Value(QString::fromStdString(label.valuey().get()), this);
                if (!y.isEvaluated())
                {
                    ErrorResult result = currentPythonEngineAgros()->parseError();
                    throw AgrosException(result.error());
                }

                m_scene->addLabel(new SceneLabel(m_scene,
                                                 PointValue(x, y),
                                                 label.area()));
            }
            else
            {
                Point point = Point(label.x(),
                                    label.y());

                m_scene->addLabel(new SceneLabel(m_scene,
                                                 point,
                                                 label.area()));
            }
        }

        for (unsigned int i = 0; i < doc->problem().fields().field().size(); i++)
        {
            XMLProblem::field field = doc->problem().fields().field().at(i);

            FieldInfo *fieldInfo = new FieldInfo(QString::fromStdString(field.field_id()));

            // analysis type
            fieldInfo->setAnalysisType(analysisTypeFromStringKey(QString::fromStdString(field.analysis_type())));
            // adaptivity
            fieldInfo->setAdaptivityType(adaptivityTypeFromStringKey(QString::fromStdString(field.adaptivity_type())));
            // linearity
            fieldInfo->setLinearityType(linearityTypeFromStringKey(QString::fromStdString(field.linearity_type())));
            // matrix solver
            if (field.matrix_solver().present())
                fieldInfo->setMatrixSolver(matrixSolverTypeFromStringKey(QString::fromStdString(field.matrix_solver().get())));

            // field config
            fieldInfo->load(&field.field_config());

            // edge refinement
            for (unsigned int j = 0; j < field.refinement_edges().refinement_edge().size(); j++)
            {
                XMLProblem::refinement_edge edge = field.refinement_edges().refinement_edge().at(j);

                if (edge.refinement_edge_id() != -1)
                    fieldInfo->setEdgeRefinement(m_scene->edges->items().at(edge.refinement_edge_id()), edge.refinement_edge_number());
            }

            // label refinement
            for (unsigned int j = 0; j < field.refinement_labels().refinement_label().size(); j++)
            {
                XMLProblem::refinement_label label = field.refinement_labels().refinement_label().at(j);

                if (label.refinement_label_id() != -1)
                    fieldInfo->setLabelRefinement(m_scene->labels->items().at(label.refinement_label_id()), label.refinement_label_number());
            }

            // polynomial order
            for (unsigned int j = 0; j < field.polynomial_orders().polynomial_order().size(); j++)
            {
                XMLProblem::polynomial_order order = field.polynomial_orders().polynomial_order().at(j);

                fieldInfo->setLabelPolynomialOrder(m_scene->labels->items().at(order.polynomial_order_id()), order.polynomial_order_number());
            }

            // boundary conditions
            for (unsigned int j = 0; j < field.boundaries().boundary().size(); j++)
            {
                XMLProblem::boundary boundary = field.boundaries().boundary().at(j);

                // read marker
                SceneBoundary *bound = new SceneBoundary(m_scene,
                                                         fieldInfo,
                                                         QString::fromStdString(boundary.name()),
                                                         QString::fromStdString(boundary.type()));

                // default values
                Module::BoundaryType boundaryType = fieldInfo->boundaryType(QString::fromStdString(boundary.type()));
                foreach (Module::BoundaryTypeVariable variable, boundaryType.variables())
                    bound->setValue(variable.id(), Value());

                for (unsigned int k = 0; k < boundary.boundary_types().boundary_type().size(); k++)
                {
                    XMLProblem::boundary_type type = boundary.boundary_types().boundary_type().at(k);

                    Value b = Value(QString::fromStdString(type.value()), this);
                    if (!b.isEvaluated())
                    {
                        ErrorResult result = currentPythonEngineAgros()->parseError();
                        throw AgrosException(result.error());
                    }

                    bound->setValue(QString::fromStdString(type.key()), b);
                }

                m_scene->addBoundary(bound);

                // add boundary to the edge marker
                for (unsigned int k = 0; k < boundary.boundary_edges().boundary_edge().size(); k++)
                {
                    XMLProblem::boundary_edge edge = boundary.boundary_edges().boundary_edge().at(k);

                    m_scene->edges->at(edge.id())->addMarker(bound);
                }
            }

            // materials
            for (unsigned int j = 0; j < field.materials().material().size(); j++)
            {
                XMLProblem::material material = field.materials().material().at(j);

                // read marker
                SceneMaterial *mat = new SceneMaterial(m_scene,
                                                       fieldInfo,
                                                       QString::fromStdString(material.name()));

                // default values
                foreach (Module::MaterialTypeVariable variable, fieldInfo->materialTypeVariables())
                {
                    mat->setValue(variable.id(), Value());
                }

                for (unsigned int k = 0; k < material.material_types().material_type().size(); k++)
                {
                    XMLProblem::material_type type = material.material_types().material_type().at(k);

                    Value m = Value(QString::fromStdString(type.value()), this);
                    if (!m.isEvaluated())
                    {
                        ErrorResult result = currentPythonEngineAgros()->parseError();
                        throw AgrosException(result.error());
                    }

                    mat->setValue(QString::fromStdString(type.key()), m);
                }

                m_scene->addMaterial(mat);

                // add boundary to the edge marker
                for (unsigned int k = 0; k < material.material_labels().material_label().size(); k++)
                {
                    XMLProblem::material_label label = material.material_labels().material_label().at(k);

                    m_scene->labels->at(label.id())->addMarker(mat);
                }
            }

            // add missing none markers
            m_scene->edges->addMissingFieldMarkers(fieldInfo);
            m_scene->labels->addMissingFieldMarkers(fieldInfo);

            // add field
            addField(fieldInfo);
        }

        // couplings
        synchronizeCouplings();

        for (unsigned int i = 0; i < doc->problem().couplings().coupling().size(); i++)
        {
            XMLProblem::coupling coupling = doc->problem().couplings().coupling().at(i);

            if (hasCoupling(QString::fromStdString(coupling.source_fieldid()),
                            QString::fromStdString(coupling.target_fieldid())))
            {
                CouplingInfo *cpl = couplingInfo(QString::fromStdString(coupling.source_fieldid()),
                                                 QString::fromStdString(coupling.target_fieldid()));
                cpl->setCouplingType(couplingTypeFromStringKey(QString::fromStdString(coupling.type())));
            }
        }

        m_scene->stopInvalidating(false);
        m_scene->blockSignals(false);

        // default values
        emit m_scene->invalidated();
        emit m_scene->defaultValues();
    }
    catch (const xml_schema::expected_element& e)
    {
        m_scene->blockSignals(false);
        throw AgrosException(QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.name())));
    }
    catch (const xml_schema::expected_attribute& e)
    {
        m_scene->blockSignals(false);
        throw AgrosException(QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.name())));
    }
    catch (const xml_schema::exception& e)
    {
        m_scene->blockSignals(false);
        throw AgrosException(QString::fromStdString(e.what()));
    }
    catch (AgrosException e)
    {
        m_scene->blockSignals(false);
        throw e;
    }
}

void Problem::transformProblem(const QString &fileName, const QString &tempFileName, double version)
{
    QString out;

    if (version == 3.1)
    {
        Agros2D::log()->printMessage(tr("Problem"), tr("Data file was transformed to new version and saved to temp dictionary."));
        return;
    }
    else if (version == 3.0)
    {
        out = transformXML(fileName, datadir() + "/resources/xslt/problem_a2d_31_xml.xsl");
        version = 3.1;
    }
    else if (version == 2.1)
    {
        out = transformXML(fileName, datadir() + "/resources/xslt/problem_a2d_30_xml.xsl");
        version = 3.0;
    }
    else if (version == 2.0 || version == 0.0)
    {
        out = transformXML(fileName, datadir() + "/resources/xslt/problem_a2d_21_xml.xsl");
        version = 2.1;
    }
    else
        throw AgrosException(tr("It is impossible to transform data file."));

    //qDebug() << out;

    QFile tempFile(tempFileName);
    if (!tempFile.open(QIODevice::WriteOnly))
        throw AgrosException(tr("File cannot be saved (%2).").arg(tempFile.errorString()));

    QTextStream(&tempFile) << out;
    tempFile.close();

    transformProblem(tempFileName, tempFileName, version);
}

void Problem::writeProblemToA2D(const QString &fileName)
{
    double version = 3.1;

    try
    {
        XMLProblem::fields fields;
        // fields
        foreach (FieldInfo *fieldInfo, fieldInfos())
        {
            XMLProblem::refinement_edges refinement_edges;
            QMapIterator<SceneEdge *, int> edgeIterator(fieldInfo->edgesRefinement());
            while (edgeIterator.hasNext()) {
                edgeIterator.next();

                refinement_edges.refinement_edge().push_back(XMLProblem::refinement_edge(m_scene->edges->items().indexOf(edgeIterator.key()),
                                                                                         edgeIterator.value()));
            }

            XMLProblem::refinement_labels refinement_labels;
            QMapIterator<SceneLabel *, int> labelIterator(fieldInfo->labelsRefinement());
            while (labelIterator.hasNext()) {
                labelIterator.next();

                refinement_labels.refinement_label().push_back(XMLProblem::refinement_label(m_scene->labels->items().indexOf(labelIterator.key()),
                                                                                            labelIterator.value()));
            }

            XMLProblem::polynomial_orders polynomial_orders;
            QMapIterator<SceneLabel *, int> labelOrderIterator(fieldInfo->labelsPolynomialOrder());
            while (labelOrderIterator.hasNext()) {
                labelOrderIterator.next();

                polynomial_orders.polynomial_order().push_back(XMLProblem::polynomial_order(m_scene->labels->items().indexOf(labelOrderIterator.key()),
                                                                                            labelOrderIterator.value()));
            }

            XMLProblem::boundaries boundaries;
            int iboundary = 1;
            foreach (SceneBoundary *bound, m_scene->boundaries->filter(fieldInfo).items())
            {
                // add edges
                XMLProblem::boundary_edges boundary_edges;
                foreach (SceneEdge *edge, m_scene->edges->items())
                    if (edge->hasMarker(bound))
                        boundary_edges.boundary_edge().push_back(XMLProblem::boundary_edge(m_scene->edges->items().indexOf(edge)));

                XMLProblem::boundary_types boundary_types;
                const QMap<uint, QSharedPointer<Value> > values = bound->values();
                for (QMap<uint, QSharedPointer<Value> >::const_iterator it = values.begin(); it != values.end(); ++it)
                    boundary_types.boundary_type().push_back(XMLProblem::boundary_type(bound->valueName(it.key()).toStdString(), it.value()->toString().toStdString()));

                XMLProblem::boundary boundary(boundary_edges,
                                              boundary_types,
                                              (bound->type() == "") ? "none" : bound->type().toStdString(),
                                              iboundary,
                                              bound->name().toStdString());


                boundaries.boundary().push_back(boundary);

                iboundary++;
            }

            // materials
            XMLProblem::materials materials;
            int imaterial = 1;
            foreach (SceneMaterial *mat, m_scene->materials->filter(fieldInfo).items())
            {
                // add labels
                XMLProblem::material_labels material_labels;
                foreach (SceneLabel *label, m_scene->labels->items())
                    if (label->hasMarker(mat))
                        material_labels.material_label().push_back(XMLProblem::material_label(m_scene->labels->items().indexOf(label)));

                XMLProblem::material_types material_types;
                const QMap<uint, QSharedPointer<Value> > values = mat->values();
                for (QMap<uint, QSharedPointer<Value> >::const_iterator it = values.begin(); it != values.end(); ++it)
                    material_types.material_type().push_back(XMLProblem::material_type(mat->valueName(it.key()).toStdString(), it.value()->toString().toStdString()));

                XMLProblem::material material(material_labels,
                                              material_types,
                                              imaterial,
                                              mat->name().toStdString());

                materials.material().push_back(material);

                imaterial++;
            }

            XMLProblem::field_config field_config;
            fieldInfo->save(&field_config);

            XMLProblem::field field(refinement_edges,
                                    refinement_labels,
                                    polynomial_orders,
                                    boundaries,
                                    materials,
                                    field_config,
                                    fieldInfo->fieldId().toStdString(),
                                    analysisTypeToStringKey(fieldInfo->analysisType()).toStdString(),
                                    adaptivityTypeToStringKey(fieldInfo->adaptivityType()).toStdString(),
                                    linearityTypeToStringKey(fieldInfo->linearityType()).toStdString());
            field.matrix_solver().set(matrixSolverTypeToStringKey(fieldInfo->matrixSolver()).toStdString());

            fields.field().push_back(field);
        }

        XMLProblem::couplings couplings;
        foreach (CouplingInfo *couplingInfo, couplingInfos())
        {
            couplings.coupling().push_back(XMLProblem::coupling(couplingInfo->couplingId().toStdString(),
                                                                couplingTypeToStringKey(couplingInfo->couplingType()).toStdString(),
                                                                couplingInfo->sourceField()->fieldId().toStdString(),
                                                                couplingInfo->targetField()->fieldId().toStdString()));
        }

        XMLProblem::config config;
        m_setting->save(&config);

        XMLProblem::problem_config problem_config;
        m_config->save(&problem_config);

        XMLProblem::problem problem(fields,
                                    couplings,
                                    problem_config,
                                    coordinateTypeToStringKey(m_config->coordinateType()).toStdString(),
                                    meshTypeToStringKey(m_config->meshType()).toStdString());

        // nodes
        XMLProblem::nodes nodes;
        int inode = 0;
        foreach (SceneNode *node, m_scene->nodes->items())
        {
            XMLProblem::node nodexml(inode,
                                     node->point().x,
                                     node->point().y);

            nodexml.valuex().set(node->pointValue().x().toString().toStdString());
            nodexml.valuey().set(node->pointValue().y().toString().toStdString());

            nodes.node().push_back(nodexml);
            inode++;
        }

        // edges
        XMLProblem::edges edges;
        int iedge = 0;
        foreach (SceneEdge *edge, m_scene->edges->items())
        {
            XMLProblem::edge edgexml = XMLProblem::edge(iedge,
                                                        m_scene->nodes->items().indexOf(edge->nodeStart()),
                                                        m_scene->nodes->items().indexOf(edge->nodeEnd()),
                                                        edge->angle());

            edgexml.segments().set(edge->segments());
            edgexml.is_curvilinear().set(edge->isCurvilinear());
            edgexml.valueangle().set(edge->angleValue().toString().toStdString());

            edges.edge().push_back(edgexml);
            iedge++;
        }

        // labels
        XMLProblem::labels labels;
        int ilabel = 0;
        foreach (SceneLabel *label, m_scene->labels->items())
        {
            XMLProblem::label labelxml(ilabel,
                                       label->point().x,
                                       label->point().y,
                                       label->area());

            labelxml.valuex().set(label->pointValue().x().toString().toStdString());
            labelxml.valuey().set(label->pointValue().y().toString().toStdString());

            labels.label().push_back(labelxml);
            ilabel++;
        }

        // geometry
        XMLProblem::geometry geometry(nodes, edges, labels);

        XMLProblem::document doc(geometry, problem, config, version);

        std::string problem_schema_location("");

        problem_schema_location.append(QString("%1/problem_a2d_31_xml.xsd").arg(QFileInfo(datadir() + XSDROOT).absoluteFilePath()).toStdString());
        ::xml_schema::namespace_info namespace_info_problem("XMLProblem", problem_schema_location);

        ::xml_schema::namespace_infomap namespace_info_map;
        namespace_info_map.insert(std::pair<std::basic_string<char>, xml_schema::namespace_info>("problem", namespace_info_problem));

        std::ofstream out(compatibleFilename(fileName).toStdString().c_str());
        XMLProblem::document_(out, doc, namespace_info_map);
    }
    catch (const xml_schema::exception& e)
    {
        throw AgrosException(QString::fromStdString(e.what()));
    }
}

// computation

ProblemComputation::ProblemComputation(const QString &problemDir) : Problem()
{
    // m_timeStep = 0;
    m_lastTimeElapsed = QTime();
    m_isSolving = false;
    m_isMeshing = false;
    m_abort = false;
    m_isPostprocessingRunning = false;

    connect(this, SIGNAL(fieldsChanged()), m_scene, SLOT(doFieldsChanged()));

    m_calculationThread = new CalculationThread(this);

    m_problemSolver = new ProblemSolver(this);
    m_postDeal = new PostDeal(this);
    m_solutionStore = new SolutionStore(this);
    m_result = new ProblemResult();

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

ProblemComputation::~ProblemComputation()
{
    clearSolution();

    delete m_calculationThread;
    delete m_problemSolver;
    delete m_postDeal;
    delete m_solutionStore;
    delete m_result;
}

QList<double> ProblemComputation::timeStepTimes() const
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

double ProblemComputation::timeStepToTotalTime(int timeStepIndex) const
{
    double time = 0.0;
    for (int ts = 0; ts <= timeStepIndex; ts++)
        time += m_timeStepLengths[ts];

    return time;
}

void ProblemComputation::setActualTimeStepLength(double timeStep)
{
    m_timeStepLengths.append(timeStep);
}

void ProblemComputation::removeLastTimeStepLength()
{
    m_timeStepLengths.removeLast();
}

void ProblemComputation::solveInit()
{
    if (fieldInfos().isEmpty())
    {
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("No fields defined"));
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

    // open indicator progress
    Indicator::openProgress();

    // control geometry
    m_scene->checkGeometryResult();
    m_scene->checkGeometryAssignement();

    // nonlinearity
    m_isNonlinear = determineIsNonlinear();

    // todo: we should not mesh always, but we would need to refine signals to determine when is it neccesary
    // (whether, e.g., parameters of the mesh have been changed)
    if (!mesh(false))
        throw AgrosSolverException(tr("Could not create mesh"));

    // dealii - new concept without Blocks (not necessary)
    m_problemSolver->init();
}

void ProblemComputation::doAbortSolve()
{
    m_abort = true;
    Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Aborting calculation..."));
}

void ProblemComputation::meshWithGUI()
{
    if (!isPreparedForAction())
        return;

    LogDialog *logDialog = new LogDialog(this, tr("Mesh"));
    logDialog->show();

    // create mesh
    meshThread();
}

void ProblemComputation::solveWithGUI()
{
    if (!isPreparedForAction())
        return;

    LogDialog *logDialog = new LogDialog(this, tr("Solver"));
    logDialog->show();

    // solve problem
    solveThread();
}

void ProblemComputation::meshThread()
{
    if (!isPreparedForAction())
        return;

    m_calculationThread->startCalculation(CalculationThread::CalculationType_Mesh);
}

void ProblemComputation::solveThread()
{
    if (!isPreparedForAction())
        return;

    m_calculationThread->startCalculation(CalculationThread::CalculationType_Solve);
}

void ProblemComputation::solve()
{
    if (!isPreparedForAction())
        return;

    // clear solution
    clearSolution();

    if ((m_fieldInfos.size() > 1) && isTransient() && (numAdaptiveFields() >= 1))
    {
        Agros2D::log()->printError(tr("Solver"), tr("Space adaptivity for transient coupled problems not possible at the moment."));
        return;
    }

    if (m_fieldInfos.isEmpty())
    {
        Agros2D::log()->printError(tr("Solver"), tr("No fields defined"));
        return;
    }

    if (Agros2D::configComputer()->value(Config::Config_LinearSystemSave).toBool())
        Agros2D::log()->printWarning(tr("Solver"), tr("Matrix and RHS will be saved on the disk and this will slow down the calculation. You may disable it in application settings."));

    try
    {
        m_isSolving = true;

        QTime time;
        time.start();

        solveAction();

        m_lastTimeElapsed = milisecondsToTime(time.elapsed());

        // elapsed time
        Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("Elapsed time: %1 s").arg(m_lastTimeElapsed.toString("mm:ss.zzz")));

        m_abort = false;
        m_isSolving = false;

        // refresh post deal
        m_postDeal->problemSolved();

        emit solved();

        // close indicator progress
        Indicator::closeProgress();
    }
    /*
    catch (Exceptions::NonlinearException &e)
    {
        Solvers::NonlinearConvergenceState convergenceState = e.get_exception_state();
        switch(convergenceState)
        {
        case Solvers::NotConverged:
        {
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("Newton solver did not converge."));
            break;
        }
        case Solvers::BelowMinDampingCoeff:
        {
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("Damping coefficient below minimum."));
            break;
        }
        case Solvers::AboveMaxAllowedResidualNorm:
        {
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("Residual norm exceeded limit."));
            break;
        }
        case Solvers::AboveMaxIterations:
        {
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("Number of iterations exceeded limit."));
            break;
        }
        case Solvers::Error:
        {
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("An error occurred in Newton solver."));
            break;
        }
        default:
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("Newton solver failed from unknown reason."));
        }

        m_isSolving = false;
        return;
    }
    */
    catch (AgrosGeometryException& e)
    {
        Agros2D::log()->printError(QObject::tr("Geometry"), e.what());
        m_isSolving = false;
        return;
    }
    catch (AgrosSolverException& e)
    {
        Agros2D::log()->printError(QObject::tr("Solver"), e.what());
        m_isSolving = false;
        return;
    }
    catch (AgrosException& e)
    {
        Agros2D::log()->printError(QObject::tr("Solver"), e.what());
        m_isSolving = false;
        return;
    }
    catch (std::exception& e)
    {
        Agros2D::log()->printError(QObject::tr("Solver"), e.what());
        m_isSolving = false;
        return;
    }
    catch (...)
    {
        // todo: dangerous
        // catching all other exceptions. This is not save at all
        m_isSolving = false;
        Agros2D::log()->printError(tr("Solver"), tr("An unknown exception occurred in solver and has been ignored"));
        qDebug() << "Solver: An unknown exception occurred and has been ignored";
        return;
    }
}

void ProblemComputation::solveAction()
{
    // clear solution
    clearSolution();

    solveInit();
    assert(isMeshed());

    m_problemSolver->solveProblem();
}

bool ProblemComputation::mesh(bool emitMeshed)
{
    bool result = false;

    // TODO: make global check geometry before mesh() and solve()
    if (m_fieldInfos.isEmpty())
    {
        Agros2D::log()->printError(tr("Mesh"), tr("No fields defined"));
        return false;
    }

    m_isMeshing = true;

    try
    {
        result = meshAction(emitMeshed);
    }
    catch (AgrosGeometryException& e)
    {
        // this assumes that all the code in Hermes and Agros is exception-safe
        // todo:  this is almost certainly not the case, at least for Agros. It should be further investigated
        m_isMeshing = false;
        Agros2D::log()->printError(tr("Geometry"), QString("%1").arg(e.what()));
        return false;
    }
    catch (AgrosMeshException& e)
    {
        // this assumes that all the code in Hermes and Agros is exception-safe
        // todo:  this is almost certainly not the case, at least for Agros. It should be further investigated
        m_isMeshing = false;
        Agros2D::log()->printError(tr("Mesh"), QString("%1").arg(e.what()));
        return false;
    }
    catch (AgrosException& e)
    {
        // todo: dangerous
        // catching all other exceptions. This is not safe at all
        m_isMeshing = false;
        Agros2D::log()->printWarning(tr("Mesh"), e.what());
        return false;
    }
    catch (dealii::ExceptionBase &e)
    {
        m_isMeshing = false;
        Agros2D::log()->printWarning(tr("Mesh (deal.II)"), e.what());
        return false;
    }
    catch (...)
    {
        // todo: dangerous
        // catching all other exceptions. This is not safe at all
        m_isMeshing = false;
        Agros2D::log()->printWarning(tr("Mesh"), tr("An unknown exception occurred and has been ignored"));
        qDebug() << "Mesh: An unknown exception occurred and has been ignored";
        return false;
    }

    m_isMeshing = false;

    // refreh post
    m_postDeal->problemMeshed();

    return result;
}

bool ProblemComputation::meshAction(bool emitMeshed)
{
    clearSolution();

    Agros2D::log()->printMessage(QObject::tr("Mesh Generator"), QObject::tr("Initial mesh generation"));

    // check geometry
    m_scene->checkGeometryResult();
    m_scene->checkGeometryAssignement();

    QSharedPointer<MeshGenerator> meshGenerator;
    switch (config()->meshType())
    {
    case MeshType_Triangle:
        // case MeshType_Triangle_QuadFineDivision:
        // case MeshType_Triangle_QuadRoughDivision:
        // case MeshType_Triangle_QuadJoin:
        meshGenerator = QSharedPointer<MeshGenerator>(new MeshGeneratorTriangle(this));
        break;
        // case MeshType_GMSH_Triangle:
    case MeshType_GMSH_Quad:
    case MeshType_GMSH_QuadDelaunay_Experimental:
        meshGenerator = QSharedPointer<MeshGenerator>(new MeshGeneratorGMSH(this));
        break;
        // case MeshType_NETGEN_Triangle:
        // case MeshType_NETGEN_QuadDominated:
        //     meshGenerator = QSharedPointer<MeshGenerator>(new MeshGeneratorNetgen());
        //     break;
    case MeshType_CUBIT:
        meshGenerator = QSharedPointer<MeshGenerator>(new MeshGeneratorCubitExternal(this));
        break;
    default:
        QMessageBox::critical(QApplication::activeWindow(), "Mesh generator error", QString("Mesh generator '%1' is not supported.").arg(meshTypeString(config()->meshType())));
        break;
    }

    // add icon to progress
    Agros2D::log()->addIcon(icon("scene-meshgen"),
                            tr("Mesh generator\n%1").arg(meshTypeString(config()->meshType())));

    if (meshGenerator && meshGenerator->mesh())
    {
        // load mesh
        try
        {
            readInitialMeshFromFile(emitMeshed, meshGenerator);
            return true;
        }
        catch (AgrosException& e)
        {
            throw AgrosMeshException(e.what());
        }
    }

    return false;
}

void ProblemComputation::readInitialMeshFromFile(bool emitMeshed, QSharedPointer<MeshGenerator> meshGenerator)
{
    if (!meshGenerator)
    {
        // load initial mesh file
        QString fnMesh = QString("%1/%2/mesh_initial.msh").arg(cacheProblemDir()).arg(problemDir());
        std::ifstream ifsMesh(fnMesh.toStdString());
        boost::archive::binary_iarchive sbiMesh(ifsMesh);
        m_initialMesh.load(sbiMesh, 0);

        Agros2D::log()->printDebug(tr("Mesh Generator"), tr("Reading initial mesh from disk"));
    }
    else
    {
        m_initialMesh.copy_triangulation(meshGenerator->triangulation());
        Agros2D::log()->printDebug(tr("Mesh Generator"), tr("Reading initial mesh from memory"));
    }

    int max_num_refinements = 0;
    foreach (FieldInfo *fieldInfo, m_fieldInfos)
    {
        // refine mesh
        // TODO: at the present moment, not possible to refine independently
        max_num_refinements = std::max(max_num_refinements, fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt());
    }

    // this is just a workaround for the problem in deal user data are not preserved on faces after refinement
    m_initialUnrefinedMesh.copy_triangulation(m_initialMesh);

    m_calculationMesh.copy_triangulation(m_initialMesh);
    m_calculationMesh.refine_global(max_num_refinements);
    //propagateBoundaryMarkers();

    // nonlinearity
    m_isNonlinear = determineIsNonlinear();

    if (emitMeshed)
        emit meshed();
}

void ProblemComputation::propagateBoundaryMarkers()
{
    dealii::Triangulation<2>::cell_iterator cell_unrefined = m_initialUnrefinedMesh.begin();
    dealii::Triangulation<2>::cell_iterator end_cell_unrefined = m_initialUnrefinedMesh.end();
    dealii::Triangulation<2>::cell_iterator cell_initial = m_initialMesh.begin();
    dealii::Triangulation<2>::cell_iterator cell_calculation = m_calculationMesh.begin();

    for (int idx = 0; cell_unrefined != end_cell_unrefined; ++cell_initial, ++cell_calculation, ++cell_unrefined, ++idx)   // loop over all cells, not just active ones
    {
        for (int f=0; f < dealii::GeometryInfo<2>::faces_per_cell; f++)
        {
            if (cell_unrefined->face(f)->user_index() != 0)
            {
                cell_initial->face(f)->recursively_set_user_index(cell_unrefined->face(f)->user_index());
                cell_calculation->face(f)->recursively_set_user_index(cell_unrefined->face(f)->user_index());
            }
        }
    }
}

bool ProblemComputation::isMeshed() const
{
    if (m_initialMesh.n_active_cells() == 0)
        return false;

    return (m_fieldInfos.size() > 0);
}

bool ProblemComputation::isSolved() const
{
    // return (!Agros2D::solutionStore()->isEmpty() && !m_isSolving && !m_isMeshing);
    return (!m_solutionStore->isEmpty());
}

void ProblemComputation::clearSolution()
{
    m_abort = false;

    // m_timeStep = 0;
    m_lastTimeElapsed = QTime();
    m_timeStepLengths.clear();
    m_timeStepLengths.append(0.0);

    m_initialMesh.clear();
    m_initialUnrefinedMesh.clear();
    m_calculationMesh.clear();
    m_solutionStore->clear();
}

void ProblemComputation::clearResults()
{
    m_result->clear();
}

void ProblemComputation::clearFieldsAndConfig()
{
    m_solutionStore->clear();
    m_postDeal->clear();

    Problem::clearFieldsAndConfig();
}

bool ProblemComputation::loadResults()
{
    QString fnResults = QString("%1/%2/results.json").arg(cacheProblemDir()).arg(m_problemDir);
    return m_result->load(fnResults);
}

bool ProblemComputation::saveResults()
{
    QString fnResults = QString("%1/%2/results.json").arg(cacheProblemDir()).arg(m_problemDir);
    return m_result->save(fnResults);
}

// preprocessor

ProblemPreprocessor::ProblemPreprocessor() : Problem()
{
    connect(this, SIGNAL(fileNameChanged(QString)), this, SLOT(doFileNameChanged(QString)));
}

QSharedPointer<ProblemComputation> ProblemPreprocessor::createComputation(bool newComputation, bool setCurrentComputation)
{
    QSharedPointer<ProblemComputation> computation;
    if (newComputation || Agros2D::computations().isEmpty())
    {
        computation = QSharedPointer<ProblemComputation>(new ProblemComputation());
        Agros2D::addComputation(computation->problemDir(), computation);

        if (setCurrentComputation)
            Agros2D::setCurrentComputation(computation->problemDir());
    }
    else
    {
        computation = Agros2D::computation();
        computation->clearFieldsAndConfig();
    }

    QString fn = QString("%1/%2/problem.a2d").
            arg(cacheProblemDir()).
            arg(computation->problemDir());

    writeProblemToA2D(fn);    
    computation->readProblemFromA2D31(fn);

    return computation;
}

void ProblemPreprocessor::clearFieldsAndConfig()
{
    Problem::clearFieldsAndConfig();
    QFile::remove(QString("%1/problem.a2d").arg(cacheProblemDir()));

    emit fileNameChanged(tr("unnamed"));
}

void ProblemPreprocessor::readProblemFromArchive(const QString &fileName)
{
    QSettings settings;
    QFileInfo fileInfo(fileName);
    if (fileInfo.absoluteDir() != tempProblemDir() && !fileName.contains("resources/examples"))
        settings.setValue("General/LastProblemDir", fileInfo.absolutePath());

    JlCompress::extractDir(fileName, cacheProblemDir());

    QString problem = QString("%1/problem.a2d").arg(cacheProblemDir());
    if (QFile::exists(problem))
    {
        readProblemFromA2D(problem);

        QString fn = QFileInfo(fileName).absoluteFilePath();
        m_config->setFileName(fn);
        emit fileNameChanged(fn);
    }

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

        QString fnProblem = QString("%1/%2/problem.a2d").
                arg(cacheProblemDir()).
                arg(problemDir);

        if (QFile::exists(fnProblem))
        {
            QSharedPointer<ProblemComputation> computation = QSharedPointer<ProblemComputation>(new ProblemComputation(problemDir));
            Agros2D::addComputation(computation->problemDir(), computation);

            Agros2D::log()->printMessage(tr("Problem"), tr("Loading solution from disk: %1").arg(problemDir));

            computation->readProblemFromA2D31(fnProblem);

            // read results
            computation->solutionStore()->loadRunTimeDetails();

            // read mesh file
            try
            {
                computation->readInitialMeshFromFile(true);
            }
            catch (AgrosException& e)
            {
                computation->clearSolution();
                Agros2D::log()->printError(tr("Mesh"), tr("Initial mesh is corrupted (%1)").arg(e.what()));
            }

            // results
            computation->loadResults();
        }
    }

    // set last computation
    if (Agros2D::computations().count() > 0)
    {
        QSharedPointer<ProblemComputation> post = Agros2D::computation();
        if (post.isNull())
            post = Agros2D::computations().last();

        Agros2D::setCurrentComputation(post->problemDir());
        emit post->solved();
    }
}

void ProblemPreprocessor::writeProblemToArchive(const QString &fileName, bool saveWithSolution)
{
    QSettings settings;
    QFileInfo fileInfo(fileName);
    if (fileInfo.absoluteDir() != tempProblemDir())
    {
        settings.setValue("General/LastProblemDir", fileInfo.absoluteFilePath());
        m_config->setFileName(fileName);
    }

    QFileInfo fileInfoProblem(QString("%1/problem.a2d").arg(cacheProblemDir()));
    writeProblemToA2D(fileInfoProblem.absoluteFilePath());

    if (saveWithSolution)
    {
        // whole directory
        JlCompress::compressDir(fileName, cacheProblemDir());
    }
    else
    {
        // only problem file
        QStringList lst;
        lst << fileInfoProblem.absoluteFilePath();
        if (JlCompress::compressFiles(fileName, lst))
            emit fileNameChanged(QFileInfo(fileName).absoluteFilePath());
    }
}

void ProblemPreprocessor::readProblemFromFile(const QString &fileName)
{
    QFileInfo fileInfo(fileName);

    // set filename
    QString fn = fileName;

    try
    {
        // clear all computations
        Agros2D::clearComputations();

        // load problem
        if (fileInfo.suffix() == "ags")
        {
            readProblemFromArchive(fileName);
        }
        // deprecated
        else if (fileInfo.suffix() == "a2d")
        {
            Agros2D::log()->printWarning(tr("Problem"), tr("A2D file is deprecated."));

            // copy file to cache
            QFile::remove(QString("%1/problem.a2d").arg(cacheProblemDir()));
            QFile::copy(fileName, QString("%1/problem.a2d").arg(cacheProblemDir()));

            // open file
            readProblemFromA2D(QString("%1/problem.a2d").arg(cacheProblemDir()));

            // set filename
            fn = QString("%1/%2.ags").arg(fileInfo.absolutePath()).arg(fileInfo.baseName());

            m_config->setFileName(fn);
            emit fileNameChanged(fn);
            // writeProblemToArchive(fn); // only for automatic convert
        }
    }
    catch (AgrosModuleException& e)
    {
        clearFieldsAndConfig();
        Agros2D::log()->printError(tr("Problem"), e.toString());
    }
    catch (AgrosPluginException& e)
    {
        clearFieldsAndConfig();
        Agros2D::log()->printError(tr("Problem"), e.toString());
    }
    catch (AgrosException &e)
    {
        clearFieldsAndConfig();
        Agros2D::log()->printError(tr("Problem"), e.toString());
    }
}
