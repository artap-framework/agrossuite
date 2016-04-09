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

#include "optilab/study.h"

#include "../3rdparty/quazip/JlCompress.h"
#include "../resources_source/classes/problem_a2d_31_xml.h"

const QString VERSION = "version";

const QString LIST = "list";
const QString ANGLE = "angle";
const QString SEGMENTS = "segments";
const QString ISCURVILINEAR = "iscurvilinear";
const QString X = "x";
const QString Y = "y";
const QString Z = "Z";
const QString AREA = "area";
const QString VALUE = "value";

const QString ID = "id";
const QString NAME = "name";
const QString TYPE = "type";
const QString GEOMETRY = "geometry";
const QString NODES = "nodes";
const QString FACES = "faces";
const QString LABELS = "labels";
const QString SETTINGS = "settings";
const QString CONFIG = "config";
const QString FIELDS = "fields";
const QString FIELDID = "fieldid";
const QString BOUNDARIES = "boundaries";
const QString BOUNDARY_FACES = "boundary_faces";
const QString BOUNDARY_TYPES = "boundary_types";
const QString MATERIALS = "materials";
const QString MATERIAL_LABELS = "material_labels";
const QString MATERIAL_TYPES = "material_types";
const QString REFINEMENT_NUMBER = "refinement_number";
const QString REFINEMENT_ORDER = "refinement_order";

const QString COUPLINGS = "couplings";
const QString SOURCE_FIELDID = "source_field";
const QString TARGET_FIELDID = "target_field";

const QString STUDIES = "studies";
const QString RECIPES = "recipes";

void MeshThread::run()
{
    Agros2D::log()->printHeading(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));

    dealii::deal_II_exceptions::disable_abort_on_exception();

    m_problem->mesh();
}

void MeshThread::finished()
{
    emit m_problem->meshedWithGUI();
    deleteLater();
}

void SolveThread::run()
{
    Agros2D::log()->printHeading(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));

    dealii::deal_II_exceptions::disable_abort_on_exception();

    m_computation->solve();
}

void SolveThread::finished()
{
    emit m_computation->solvedWithGUI();
    deleteLater();
}

// *******************************************************************

ProblemBase::ProblemBase() :
    m_isMeshing(false),
    m_config(new ProblemConfig(this)),
    m_scene(new Scene(this)),
    m_isNonlinear(false)
{
    m_timeStepLengths.append(0.0);
}

ProblemBase::~ProblemBase()
{
    m_config->clear();

    clearFieldsAndConfig();

    delete m_scene;
    delete m_config;
}

int ProblemBase::numAdaptiveFields() const
{
    int num = 0;
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->adaptivityType() != AdaptivityMethod_None)
            num++;
    return num;
}

int ProblemBase::numTransientFields() const
{
    int num = 0;
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->analysisType() == AnalysisType_Transient)
            num++;
    return num;
}

bool ProblemBase::isTransient() const
{
    return numTransientFields() > 0;
}

bool ProblemBase::isHarmonic() const
{
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->analysisType() == AnalysisType_Harmonic)
            return true;

    return false;
}

bool ProblemBase::determineIsNonlinear() const
{
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->linearityType() != LinearityType_Linear)
            return true;

    return false;
}

bool ProblemBase::checkAndApplyParameters(StringToDoubleMap parameters, bool apply)
{
    // store original parameters
    StringToDoubleMap parametersOriginal = m_config->value(ProblemConfig::Parameters).value<StringToDoubleMap>();

    // set new parameters
    m_config->setParameters(parameters);

    // apply new parameters
    bool successfulRun = applyParametersInternal();

    // restore original parameters
    if (!successfulRun || !apply)
    {
        m_config->setParameters(parametersOriginal);

        // apply original parameters
        applyParametersInternal();
    }
    else
    {
        m_scene->cacheGeometryConstraints();
        // control geometry
        m_scene->invalidate();
    }

    return successfulRun;
}

// apply parameters from m_config
bool ProblemBase::applyParametersInternal()
{
    bool successfulRun = true;

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
            if (!node->pointValue().x().isEvaluated())
            {
                Agros2D::log()->printError(QObject::tr("Parameters"), QObject::tr("Node %1%2: %3").
                                           arg(m_scene->nodes->items().indexOf(node)).
                                           arg(m_config->labelX()).
                                           arg(node->pointValue().x().error()));

                successfulRun = false;
            }
            if (!node->pointValue().y().isEvaluated())
            {
                Agros2D::log()->printError(QObject::tr("Parameters"), QObject::tr("Node %1%2: %3").
                                           arg(m_scene->nodes->items().indexOf(node)).
                                           arg(m_config->labelY()).
                                           arg(node->pointValue().y().error()));

                successfulRun = false;
            }
        }
    }

    // edges
    foreach (SceneFace *edge, m_scene->faces->items())
    {
        if (edge->angleValue().isNumber())
        {
            continue;
        }
        else
        {
            if (!edge->angleValue().isEvaluated())
            {
                Agros2D::log()->printError(QObject::tr("Parameters"), QObject::tr("Edge %1: %2").
                                           arg(m_scene->faces->items().indexOf(edge)).
                                           arg(edge->angleValue().error()));

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
            if (!label->pointValue().x().isEvaluated())
            {
                Agros2D::log()->printError(QObject::tr("Parameters"), QObject::tr("Label %1%2: %3").
                                           arg(m_scene->labels->items().indexOf(label)).
                                           arg(m_config->labelX()).
                                           arg(label->pointValue().x().error()));

                successfulRun = false;
            }
            if (!label->pointValue().y().isEvaluated())
            {
                Agros2D::log()->printError(QObject::tr("Parameters"), QObject::tr("Label %1%2: %3").
                                           arg(m_scene->labels->items().indexOf(label)).
                                           arg(m_config->labelY()).
                                           arg(label->pointValue().y().error()));

                successfulRun = false;
            }
        }
    }

    // check materials
    foreach (SceneMaterial* material, m_scene->materials->items())
    {
        foreach (uint key, material->values().keys())
        {
            if (!material->value(key)->isEvaluated())
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
            if (!boundary->value(key)->isEvaluated())
            {
                Agros2D::log()->printError(QObject::tr("Marker"), QObject::tr("Boundary %1: %2").
                                           arg(key).arg(boundary->value(key).data()->toString()));
                successfulRun = false;
            }
        }
    }

    // check frequency
    if (!m_config->value(ProblemConfig::Frequency).value<Value>().isEvaluated())
    {
        Agros2D::log()->printError(QObject::tr("Frequency"), QObject::tr("Value: %1").
                                   arg(m_config->value(ProblemConfig::Frequency).value<Value>().error()));
        successfulRun = false;
    }

    return successfulRun;
}

void ProblemBase::clearFields()
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

    // clear scene
    m_scene->clear();

    // initial mesh
    m_initialMesh.clear();
    m_initialUnrefinedMesh.clear();
}

void ProblemBase::clearFieldsAndConfig()
{
    // clear config
    m_config->clear();
}

void ProblemBase::addField(FieldInfo *field)
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

void ProblemBase::removeField(FieldInfo *field)
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

    currentPythonEngine()->runExpression(QString("agros2d.__problem__.__remove_field__(\"%1\")").arg(field->fieldId()));

    emit fieldsChanged();
}

void ProblemBase::synchronizeCouplings()
{
    // zero or one field
    if (m_fieldInfos.count() <= 1)
        return;

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
        Agros2D::log()->printError(tr("Mesh"), tr("No fields defined"));
        return false;
    }

    m_isMeshing = true;

    try
    {
        m_initialMesh.clear();
        m_initialUnrefinedMesh.clear();

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

        if (meshGenerator->mesh())
        {
            // load mesh
            m_initialMesh.copy_triangulation(meshGenerator->triangulation());
            // this is just a workaround for the problem in deal user data are not preserved on faces after refinement
            m_initialUnrefinedMesh.copy_triangulation(m_initialMesh);

            Agros2D::log()->printDebug(tr("Mesh Generator"), tr("Reading initial mesh from memory"));

            m_isMeshing = false;
            return true;
        }
    }
    catch (AgrosGeometryException& e)
    {
        // this assumes that all the code in Hermes and Agros is exception-safe
        // todo:  this is almost certainly not the case, at least for Agros. It should be further investigated
        Agros2D::log()->printError(tr("Geometry"), QString("%1").arg(e.what()));
    }
    catch (AgrosMeshException& e)
    {
        // this assumes that all the code in Hermes and Agros is exception-safe
        // todo:  this is almost certainly not the case, at least for Agros. It should be further investigated
        Agros2D::log()->printError(tr("Mesh"), QString("%1").arg(e.what()));
    }
    catch (AgrosException& e)
    {
        // todo: dangerous
        // catching all other exceptions. This is not safe at all
        Agros2D::log()->printWarning(tr("Mesh"), e.what());
    }
    catch (dealii::ExceptionBase &e)
    {
        qDebug() << e.what();
        Agros2D::log()->printWarning(tr("Mesh (deal.II)"), e.what());
    }
    catch (...)
    {
        // todo: dangerous
        // catching all other exceptions. This is not safe at all
        Agros2D::log()->printWarning(tr("Mesh"), tr("An unknown exception occurred and has been ignored"));
        qDebug() << "Mesh: An unknown exception occurred and has been ignored";
    }

    m_isMeshing = false;
    return false;
}

void ProblemBase::meshWithGUI()
{
    // LogDialog *logDialog = new LogDialog(this, tr("Mesh"));
    // logDialog->show();

    MeshThread *meshThread = new MeshThread(this);
    meshThread->startCalculation();
}

void ProblemBase::readInitialMeshFromFile(const QString &problemDir, bool emitMeshed)
{
    // load initial mesh file
    QString fnMesh = QString("%1/%2/mesh_initial.msh").arg(cacheProblemDir()).arg(problemDir);
    std::ifstream ifsMesh(fnMesh.toStdString());
    boost::archive::binary_iarchive sbiMesh(ifsMesh);
    m_initialMesh.load(sbiMesh, 0);

    // this is just a workaround for the problem in deal user data are not preserved on faces after refinement
    m_initialUnrefinedMesh.copy_triangulation(m_initialMesh);

    Agros2D::log()->printDebug(tr("Mesh Generator"), tr("Reading initial mesh from disk"));

    if (emitMeshed)
        emit meshed();
}

void ProblemBase::readProblemFromJson(const QString &fileName)
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
        qWarning("Couldn't open problem file.");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject rootJson = doc.object();

    readProblemFromJsonInternal(rootJson);
    // qDebug() << "readProblemFromJson" << time.elapsed();
}

void ProblemBase::importProblemFromA2D(const QString &fileName)
{
    try
    {
        std::unique_ptr<XMLProblem::document> document_xsd = XMLProblem::document_(compatibleFilename(fileName).toStdString(), xml_schema::flags::dont_validate);
        XMLProblem::document *doc = document_xsd.get();

        // clear scene
        m_scene->clear();

        m_scene->blockSignals(true);
        m_scene->stopInvalidating(true);

        // problem config
        m_config->load(&doc->problem().problem_config());

        // coordinate type
        m_config->setCoordinateType(coordinateTypeFromStringKey(QString::fromStdString(doc->problem().coordinate_type())));
        // mesh type
        m_config->setMeshType(meshTypeFromStringKey(QString::fromStdString(doc->problem().mesh_type())));

        // nodes
        for (unsigned int i = 0; i < doc->geometry().nodes().node().size(); i++)
        {
            XMLProblem::node node = doc->geometry().nodes().node().at(i);

            if (node.valuex().present() && node.valuey().present())
            {
                Value x = Value(this, QString::fromStdString(node.valuex().get()));
                if (!x.isEvaluated()) throw AgrosException(x.error());

                Value y = Value(this, QString::fromStdString(node.valuey().get()));
                if (!y.isEvaluated()) throw AgrosException(y.error());

                m_scene->addNode(new SceneNode(m_scene, PointValue(x, y)));
            }
            else
            {
                Point point = Point(node.x(), node.y());

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
                Value angle = Value(this, QString::fromStdString(edge.valueangle().get()));
                if (!angle.isEvaluated()) throw AgrosException(angle.error());

                if (angle.number() < 0.0) angle.setNumber(0.0);
                if (angle.number() > 90.0) angle.setNumber(90.0);

                m_scene->addFace(new SceneFace(m_scene, nodeFrom, nodeTo, angle, segments, isCurvilinear));
            }
            else
            {
                m_scene->addFace(new SceneFace(m_scene, nodeFrom, nodeTo, Value(this, edge.angle()), segments, isCurvilinear));
            }
        }

        // labels
        for (unsigned int i = 0; i < doc->geometry().labels().label().size(); i++)
        {
            XMLProblem::label label = doc->geometry().labels().label().at(i);

            if (label.valuex().present() && label.valuey().present())
            {
                Value x = Value(this, QString::fromStdString(label.valuex().get()));
                if (!x.isEvaluated()) throw AgrosException(x.error());

                Value y = Value(this, QString::fromStdString(label.valuey().get()));
                if (!y.isEvaluated()) throw AgrosException(y.error());

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

            // field config
            fieldInfo->load(&field.field_config());

            // analysis type
            fieldInfo->setAnalysisType(analysisTypeFromStringKey(QString::fromStdString(field.analysis_type())));
            // adaptivity
            fieldInfo->setAdaptivityType(adaptivityTypeFromStringKey(QString::fromStdString(field.adaptivity_type())));
            // linearity
            fieldInfo->setLinearityType(linearityTypeFromStringKey(QString::fromStdString(field.linearity_type())));
            // matrix solver
            if (field.matrix_solver().present())
                fieldInfo->setMatrixSolver(matrixSolverTypeFromStringKey(QString::fromStdString(field.matrix_solver().get())));

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
                    bound->setValue(variable.id(), Value(this));

                for (unsigned int k = 0; k < boundary.boundary_types().boundary_type().size(); k++)
                {
                    XMLProblem::boundary_type type = boundary.boundary_types().boundary_type().at(k);

                    Value b = Value(this, QString::fromStdString(type.value()));
                    if (!b.isEvaluated()) throw AgrosException(b.error());

                    bound->setValue(QString::fromStdString(type.key()), b);
                }

                m_scene->addBoundary(bound);

                // add boundary to the edge marker
                for (unsigned int k = 0; k < boundary.boundary_edges().boundary_edge().size(); k++)
                {
                    XMLProblem::boundary_edge edge = boundary.boundary_edges().boundary_edge().at(k);

                    m_scene->faces->at(edge.id())->addMarker(bound);
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
                    mat->setValue(variable.id(), Value(this));

                for (unsigned int k = 0; k < material.material_types().material_type().size(); k++)
                {
                    XMLProblem::material_type type = material.material_types().material_type().at(k);

                    Value m = Value(this, QString::fromStdString(type.value()));
                    if (!m.isEvaluated()) throw AgrosException(m.error());

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
            m_scene->faces->addMissingFieldMarkers(fieldInfo);
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

        // qDebug() << "readProblemFromA2D" << time.elapsed();

        // default values
        // emit m_scene->invalidated();
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

void ProblemBase::exportProblemToA2D(const QString &fileName)
{
    double version = 3.1;

    // QTime time;
    // time.start();

    try
    {
        XMLProblem::fields fields;
        // fields
        foreach (FieldInfo *fieldInfo, fieldInfos())
        {
            // deprecated
            XMLProblem::refinement_edges refinement_edges;

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
                foreach (SceneFace *edge, m_scene->faces->items())
                    if (edge->hasMarker(bound))
                        boundary_edges.boundary_edge().push_back(XMLProblem::boundary_edge(m_scene->faces->items().indexOf(edge)));

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
        foreach (SceneFace *edge, m_scene->faces->items())
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

        // qDebug() << "writeProblemToA2D" << time.elapsed();
    }
    catch (const xml_schema::exception& e)
    {
        throw AgrosException(QString::fromStdString(e.what()));
    }
}

void ProblemBase::writeProblemToJson(const QString &fileName)
{
    // QTime time;
    // time.start();

    QFile file(fileName.isEmpty() ? problemFileName() : fileName);

    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning("Couldn't open problem file.");
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

void ProblemBase::readProblemFromJsonInternal(QJsonObject &rootJson)
{
    // block signal
    bool blocked = m_scene->signalsBlocked();
    if (!blocked)
        m_scene->blockSignals(true);

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
    }

    // edges
    QJsonArray facesJson = geometryJson[FACES].toArray();
    for (int i = 0; i < facesJson.size(); i++)
    {
        QJsonObject faceJson = facesJson[i].toObject();

        QJsonArray listJson = faceJson[LIST].toArray();
        SceneNode *nodeFrom = m_scene->nodes->at(listJson[0].toInt());
        SceneNode *nodeTo = m_scene->nodes->at(listJson[1].toInt());

        int segments = faceJson[SEGMENTS].toInt();

        Value angle = Value(this, faceJson[ANGLE].toString());
        if (!angle.isEvaluated()) throw AgrosException(angle.error());

        if (angle.number() < 0.0) angle.setNumber(0.0);
        if (angle.number() > 90.0) angle.setNumber(90.0);

        bool isCurvilinear = faceJson[ISCURVILINEAR].toBool(); // (angle.number() > 0);

        m_scene->addFace(new SceneFace(m_scene, nodeFrom, nodeTo, angle, segments, isCurvilinear));
    }

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

    // fields
    QJsonArray fieldsJson = rootJson[FIELDS].toArray();
    for (int i = 0; i < fieldsJson.size(); i++)
    {
        QJsonObject fieldJson = fieldsJson[i].toObject();

        FieldInfo *fieldInfo = new FieldInfo(fieldJson[FIELDID].toString());

        // settings
        QJsonObject settingsJson = fieldJson[SETTINGS].toObject();
        fieldInfo->load(settingsJson);

        // label refinement
        QJsonArray labelRefinementJson = fieldJson[REFINEMENT_NUMBER].toArray();
        for (int i = 0; i < labelRefinementJson.size(); i++)
        {
            QJsonObject refinement = labelRefinementJson[i].toObject();

            // if (label.refinement_label_id() != -1)
            fieldInfo->setLabelRefinement(m_scene->labels->items().at(refinement[ID].toInt()), refinement[VALUE].toInt());
        }

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
            SceneBoundary *bound = new SceneBoundary(m_scene,
                                                     fieldInfo,
                                                     boundaryJson[NAME].toString(),
                                                     boundaryJson[TYPE].toString());

            // default values
            Module::BoundaryType boundaryType = fieldInfo->boundaryType(boundaryJson[TYPE].toString());
            foreach (Module::BoundaryTypeVariable variable, boundaryType.variables())
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
            SceneMaterial *mat = new SceneMaterial(m_scene,
                                                   fieldInfo,
                                                   materialJson[NAME].toString());

            // default values
            foreach (Module::MaterialTypeVariable variable, fieldInfo->materialTypeVariables())
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

        if (hasCoupling(couplingJson[SOURCE_FIELDID].toString(),
                        couplingJson[TARGET_FIELDID].toString()))
        {
            CouplingInfo *cpl = couplingInfo(couplingJson[SOURCE_FIELDID].toString(),
                                             couplingJson[TARGET_FIELDID].toString());
            cpl->setCouplingType(couplingTypeFromStringKey(couplingJson[TYPE].toString()));
        }
    }

    // restore signal
    m_scene->blockSignals(blocked);
    // invalidate scene (parameter update)
    m_scene->invalidate();
}

void ProblemBase::writeProblemToJsonInternal(QJsonObject &rootJson)
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
    foreach (SceneNode *node, m_scene->nodes->items())
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
    foreach (SceneFace *edge, m_scene->faces->items())
    {
        QJsonObject edgeJson;

        edgeJson[ID] = iedge;
        edgeJson[ANGLE] = edge->angleValue().toString();
        edgeJson[SEGMENTS] = edge->segments();
        edgeJson[ISCURVILINEAR] = edge->isCurvilinear();

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
    foreach (SceneLabel *label, m_scene->labels->items())
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
    foreach (FieldInfo *fieldInfo, fieldInfos())
    {
        QJsonObject fieldJson;
        fieldJson[FIELDID] = fieldInfo->fieldId();

        // settings
        QJsonObject settingsJson;
        fieldInfo->save(settingsJson);
        fieldJson[SETTINGS] = settingsJson;

        // label refinement
        QJsonArray labelRefinementJson;
        QMapIterator<SceneLabel *, int> labelIterator(fieldInfo->labelsRefinement());
        while (labelIterator.hasNext())
        {
            labelIterator.next();

            QJsonObject refinement;
            refinement[ID] = QString::number(m_scene->labels->items().indexOf(labelIterator.key()));
            refinement[VALUE] = labelIterator.value();

            labelRefinementJson.append(refinement);
        }
        fieldJson[REFINEMENT_NUMBER] = labelRefinementJson;

        // polynomial order
        QJsonArray labelOrderJson;
        QMapIterator<SceneLabel *, int> labelOrderIterator(fieldInfo->labelsPolynomialOrder());
        while (labelOrderIterator.hasNext())
        {
            labelOrderIterator.next();

            QJsonObject order;
            order[ID] = QString::number(m_scene->labels->items().indexOf(labelOrderIterator.key()));
            order[VALUE] = labelOrderIterator.value();

            labelOrderJson.append(order);
        }
        fieldJson[REFINEMENT_ORDER] = labelOrderJson;

        // boundaries
        QJsonArray boundariesJson;
        int iboundary = 1;
        foreach (SceneBoundary *bound, m_scene->boundaries->filter(fieldInfo).items())
        {
            QJsonObject boundaryJson;

            QJsonArray boundaryFacesJson;
            XMLProblem::boundary_edges boundary_edges;
            foreach (SceneFace *edge, m_scene->faces->items())
            {
                if (edge->hasMarker(bound))
                    boundaryFacesJson.append(m_scene->faces->items().indexOf(edge));
            }
            boundaryJson[BOUNDARY_FACES] = boundaryFacesJson;

            QJsonArray boundaryTypesJson;
            const QMap<uint, QSharedPointer<Value> > values = bound->values();
            for (QMap<uint, QSharedPointer<Value> >::const_iterator it = values.begin(); it != values.end(); ++it)
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
        QJsonArray  materialsJson;
        int imaterial = 1;
        foreach (SceneMaterial *mat, m_scene->materials->filter(fieldInfo).items())
        {
            QJsonObject materialJson;

            QJsonArray materialLabelsJson;
            foreach (SceneLabel *label, m_scene->labels->items())
            {
                if (label->hasMarker(mat))
                    materialLabelsJson.append(m_scene->labels->items().indexOf(label));
            }
            materialJson[MATERIAL_LABELS] = materialLabelsJson;

            QJsonArray materialTypesJson;
            const QMap<uint, QSharedPointer<Value> > values = mat->values();
            for (QMap<uint, QSharedPointer<Value> >::const_iterator it = values.begin(); it != values.end(); ++it)
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
    foreach (CouplingInfo *couplingInfo, couplingInfos())
    {
        QJsonObject couplingJson;

        couplingJson[ID] = couplingInfo->couplingId();
        couplingJson[TYPE] = couplingTypeToStringKey(couplingInfo->couplingType());
        couplingJson[SOURCE_FIELDID] = couplingInfo->sourceField()->fieldId();
        couplingJson[TARGET_FIELDID] = couplingInfo->targetField()->fieldId();

        couplingsJson.append(couplingJson);
    }
    rootJson[COUPLINGS] = couplingsJson;
}

// computation

Computation::Computation(const QString &problemDir) : ProblemBase(),
    m_lastTimeElapsed(QTime()),
    m_isSolving(false),
    m_abort(false),
    m_isPostprocessingRunning(false),
    m_setting(new PostprocessorSetting(this)),
    m_problemSolver(new ProblemSolver(this)),
    m_postDeal(new PostDeal(this)),
    m_solutionStore(new SolutionStore(this)),
    m_results(new ComputationResults())
{
    connect(this, SIGNAL(fieldsChanged()), m_scene, SLOT(doFieldsChanged()));

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
    delete m_postDeal;
    delete m_solutionStore;
    delete m_results;
}

void Computation::readFromProblem()
{    
    clearFields();

    // copy config from problem
    m_config->copy(Agros2D::problem()->config());

    // copy scene from problem
    m_scene->clear();
    m_scene->copy(Agros2D::problem()->scene());

    foreach (QString fieldId, Agros2D::problem()->fieldInfos().keys())
    {
        FieldInfo *originFieldInfo = Agros2D::problem()->fieldInfo(fieldId);
        FieldInfo *fieldInfo = new FieldInfo(fieldId);
        fieldInfo->copy(originFieldInfo);

        // label refinement
        foreach (SceneLabel *originLabel, originFieldInfo->labelsRefinement().keys())
        {
            SceneLabel *label = m_scene->labels->items().at(Agros2D::problem()->scene()->labels->items().indexOf(originLabel));

            fieldInfo->setLabelRefinement(label, originFieldInfo->labelsRefinement()[originLabel]);
        }

        // polynomial order
        foreach (SceneLabel *originLabel, originFieldInfo->labelsPolynomialOrder().keys())
        {
            SceneLabel *label = m_scene->labels->items().at(Agros2D::problem()->scene()->labels->items().indexOf(originLabel));

            fieldInfo->setLabelPolynomialOrder(label, originFieldInfo->labelsPolynomialOrder()[originLabel]);
        }

        // boundaries
        foreach (SceneBoundary *originBoundary, Agros2D::problem()->scene()->boundaries->items())
        {
            if (originBoundary->fieldInfo() != originFieldInfo)
                continue;

            SceneBoundary *bound = new SceneBoundary(m_scene,
                                                     fieldInfo,
                                                     originBoundary->name(),
                                                     originBoundary->type());

            // default values
            Module::BoundaryType boundaryType = fieldInfo->boundaryType(bound->type());
            foreach (Module::BoundaryTypeVariable variable, boundaryType.variables())
                bound->setValue(variable.id(), Value(this));

            // boundaries
            foreach (uint originValueIndex, originBoundary->values().keys())
            {
                QSharedPointer<Value> originValue = originBoundary->value(originValueIndex);

                bound->setValue(originBoundary->valueName(originValueIndex),
                                Value(this, originValue->text(), originValue->table()));
            }

            // add boundary to the edge marker
            foreach (SceneFace *originFace, Agros2D::problem()->scene()->faces->items())
            {
                SceneFace *face = m_scene->faces->items().at(Agros2D::problem()->scene()->faces->items().indexOf(originFace));

                if (originFace->hasMarker(originBoundary))
                    face->addMarker(bound);
            }

            m_scene->addBoundary(bound);
        }

        // materials
        foreach (SceneMaterial *originMaterial, Agros2D::problem()->scene()->materials->items())
        {
            if (originMaterial->fieldInfo() != originFieldInfo)
                continue;

            SceneMaterial *mat = new SceneMaterial(m_scene,
                                                   fieldInfo,
                                                   originMaterial->name());

            // default values
            foreach (Module::MaterialTypeVariable variable, fieldInfo->materialTypeVariables())
                mat->setValue(variable.id(), Value(this));

            // materials
            foreach (uint originValueIndex, originMaterial->values().keys())
            {
                QSharedPointer<Value> originValue = originMaterial->value(originValueIndex);

                mat->setValue(originMaterial->valueName(originValueIndex),
                              Value(this, originValue->text(), originValue->table()));
            }

            // add label to the label marker
            foreach (SceneLabel *originLabel, Agros2D::problem()->scene()->labels->items())
            {
                SceneLabel *label = m_scene->labels->items().at(Agros2D::problem()->scene()->labels->items().indexOf(originLabel));

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
    foreach (CouplingInfo *couplingInfo, Agros2D::problem()->couplingInfos())
    {
        m_couplingInfos[QPair<FieldInfo *, FieldInfo *>(fieldInfo(couplingInfo->sourceField()->fieldId()),
                                                        fieldInfo(couplingInfo->targetField()->fieldId()))]->setCouplingType(couplingInfo->couplingType());
    }

    // default values
    emit m_scene->invalidated();
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

    // invalidate scene (parameter update)
    m_scene->cacheGeometryConstraints();
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
        boost::archive::binary_oarchive sbMesh(ofsMesh);
        m_initialMesh.save(sbMesh, 0);

        // set calculation mesh
        setCalculationMesh(m_initialMesh);

        // refine mesh
        int maxNumOfRefinements = 0;
        foreach (FieldInfo *fieldInfo, m_fieldInfos)
            maxNumOfRefinements = std::max(maxNumOfRefinements, fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt());
        m_calculationMesh.refine_global(maxNumOfRefinements);
    }
    else
    {
        throw AgrosSolverException(tr("Could not create mesh"));
    }

    // dealii - new concept without Blocks (not necessary)
    m_problemSolver->init();
}

void Computation::doAbortSolve()
{
    m_abort = true;
    Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Aborting calculation..."));
}

void Computation::solveWithGUI()
{
    if (!isPreparedForAction())
        return;

    LogDialog *logDialog = new LogDialog(this, tr("Solver"));
    logDialog->show();

    SolveThread *solveThread = new SolveThread(this);
    solveThread->startCalculation();
}

void Computation::solve()
{
    if (!isPreparedForAction())
        return;

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

        // clear solution
        clearSolution();

        solveInit();
        assert(isMeshed());

        m_problemSolver->solveProblem();

        m_lastTimeElapsed = milisecondsToTime(time.elapsed());

        // elapsed time
        Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("Elapsed time: %1 s").arg(m_lastTimeElapsed.toString("mm:ss.zzz")));

        m_abort = false;
        m_isSolving = false;

        emit solved();

        // evaluate results recipes
        Agros2D::problem()->recipes()->evaluate(this);
    }
    catch (AgrosGeometryException& e)
    {
        Agros2D::log()->printError(QObject::tr("Geometry"), e.what());
        m_isSolving = false;
        return;
    }
    catch (AgrosSolverException& e)
    {
        qDebug() << e.what();
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
        // TODO: dangerous
        // catching all other exceptions. This is not save at all
        m_isSolving = false;
        Agros2D::log()->printError(tr("Solver"), tr("An unknown exception occurred in solver and has been ignored"));
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

bool Computation::isSolved() const
{
    // return (!Agros2D::solutionStore()->isEmpty() && !m_isSolving && !m_isMeshing);
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

    emit cleared();
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

void Computation::readProblemFromJsonInternal(QJsonObject &rootJson)
{
    ProblemBase::readProblemFromJsonInternal(rootJson);

    // settings
    QJsonObject settingsJson = rootJson[SETTINGS].toObject();
    m_setting->load(settingsJson);

    // results
    m_results->load(rootJson);
}

void Computation::writeProblemToJsonInternal(QJsonObject &rootJson)
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

void Problem::removeField(FieldInfo *field)
{
    // remove recipes
    m_recipes->removeAll(field);

    ProblemBase::removeField(field);
}

QString Problem::problemFileName() const
{
    return QString("%1/problem.json").arg(cacheProblemDir());
}

void Problem::readProblemFromJsonInternal(QJsonObject &rootJson)
{
    ProblemBase::readProblemFromJsonInternal(rootJson);

    // studies
    m_studies->clear();
    QJsonArray studiesJson = rootJson[STUDIES].toArray();
    for (int i = 0; i < studiesJson.size(); i++)
    {
        QJsonObject studyJson = studiesJson[i].toObject();
        StudyType type = studyTypeFromStringKey(studyJson[TYPE].toString());

        Study *study = Study::factory(type);
        study->load(studyJson);

        m_studies->blockSignals(true);
        m_studies->addStudy(study);
        m_studies->blockSignals(false);
    }
    m_studies->invalidated();

    // recipes
    m_recipes->clear();
    QJsonArray recipesJson = rootJson[RECIPES].toArray();
    for (int i = 0; i < recipesJson.size(); i++)
    {
        QJsonObject recipeJson = recipesJson[i].toObject();
        ResultRecipeType type = resultRecipeTypeFromStringKey(recipeJson[TYPE].toString());

        ResultRecipe *recipe = ResultRecipe::factory(type);
        recipe->load(recipeJson);

        m_recipes->addRecipe(recipe);
    }
}

void Problem::writeProblemToJsonInternal(QJsonObject &rootJson)
{
    ProblemBase::writeProblemToJsonInternal(rootJson);

    // studies
    QJsonArray studiesJson;
    foreach (Study *study, m_studies->items())
    {
        QJsonObject studyJson;
        studyJson[TYPE] = studyTypeToStringKey(study->type());
        study->save(studyJson);
        studiesJson.append(studyJson);
    }
    rootJson[STUDIES] = studiesJson;

    // recipes
    QJsonArray recipesJson;
    foreach (ResultRecipe *recipe, m_recipes->items())
    {
        QJsonObject recipeJson;
        recipe->save(recipeJson);
        recipesJson.append(recipeJson);
    }
    rootJson[RECIPES] = recipesJson;
}

QSharedPointer<Computation> Problem::createComputation(bool newComputation)
{
    if (newComputation || m_currentComputation.isNull() || Agros2D::computations().isEmpty())
    {
        m_currentComputation = QSharedPointer<Computation>(new Computation());
        Agros2D::addComputation(m_currentComputation->problemDir(), m_currentComputation);
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

void Problem::clearFieldsAndConfig()
{
    // clear all computations
    m_currentComputation.clear();
    Agros2D::clearComputations();

    ProblemBase::clearFieldsAndConfig();
    clearFields();

    m_recipes->clear();
    m_studies->clear();

    QFile::remove(QString("%1/problem.a2d").arg(cacheProblemDir()));
    QFile::remove(QString("%1/problem.json").arg(cacheProblemDir()));

    m_fileName = "";

    emit fileNameChanged(tr("unnamed"));
}

void Problem::readProblemFromArchive(const QString &fileName)
{
    clearFieldsAndConfig();

    QSettings settings;
    QFileInfo fileInfo(fileName);
    if (fileInfo.absoluteDir() != tempProblemDir() && !fileName.contains("resources/examples"))
        settings.setValue("General/LastProblemDir", fileInfo.absolutePath());

    JlCompress::extractDir(fileName, cacheProblemDir());

    QTime time;
    time.start();
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
            Agros2D::addComputation(computation->problemDir(), computation);

            Agros2D::log()->printMessage(tr("Problem"), tr("Loading solution from disk: %1").arg(problemDir));

            computation->readProblemFromJson();

            // read results
            if (computation->solutionStore()->loadRunTimeDetails())
            {
                // read mesh file
                try
                {
                    computation->readInitialMeshFromFile(computation->problemDir(), true);
                    computation->setCalculationMesh(computation->initialMesh());
                }
                catch (AgrosException& e)
                {
                    computation->clearSolution();
                    Agros2D::log()->printError(tr("Mesh"), tr("Initial mesh is corrupted (%1)").arg(e.what()));
                }
            }
        }
    }
    qDebug() << "read solutions" << time.elapsed();

    // convert a2d
    QString problemA2D = QString("%1/problem.a2d").arg(cacheProblemDir());
    if (QFile::exists(problemA2D))
    {
        importProblemFromA2D(problemA2D);
        writeProblemToJson();
    }

    // read problem
    readProblemFromJson();

    // remove a2d
    if (QFile::exists(problemA2D))
        QFile::remove(problemA2D);

    m_fileName = QFileInfo(fileName).absoluteFilePath();
    emit fileNameChanged(m_fileName);

    // set last computation
    if (!Agros2D::computations().isEmpty())
    {
        if (m_currentComputation.isNull() && Agros2D::computations().last()->isSolved())
            m_currentComputation = Agros2D::computations().last();
    }

    // invalidate scene (parameter update)
    m_scene->invalidate();
}

void Problem::writeProblemToArchive(const QString &fileName, bool onlyProblemFile)
{
    QSettings settings;
    QFileInfo fileInfo(fileName);
    if (fileInfo.absoluteDir() != tempProblemDir())
    {
        settings.setValue("General/LastProblemDir", fileInfo.absoluteFilePath());
        m_fileName = fileName;
    }

    // temporary - speed check
    // QFileInfo fileInfoProblem(QString("%1/problem").arg(cacheProblemDir()));
    // exportProblemToA2D(fileInfoProblem.absoluteFilePath());
    // QFile::remove(fileInfoProblem.absoluteFilePath() + ".a2d");

    writeProblemToJson();

    if (onlyProblemFile)
    {
        // only problem file
        if (JlCompress::compressFiles(fileName, QStringList() << problemFileName()))
            emit fileNameChanged(QFileInfo(fileName).absoluteFilePath());
    }
    else
    {
        // whole directory
        JlCompress::compressDir(fileName, cacheProblemDir());
    }
}

void Problem::readProblemFromFile(const QString &fileName)
{
    QFileInfo fileInfo(fileName);

    // set filename
    QString fn = fileName;

    try
    {
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
            importProblemFromA2D(QString("%1/problem.a2d").arg(cacheProblemDir()));

            // set filename
            m_fileName = QString("%1/%2.ags").arg(fileInfo.absolutePath()).arg(fileInfo.baseName());
            emit fileNameChanged(m_fileName);
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
