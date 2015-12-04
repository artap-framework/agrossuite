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

#include "pygeometry.h"
#include "pythonengine_agros.h"
#include "solver/problem_config.h"
#include "sceneview_geometry.h"
#include "scenemarker.h"

int PyGeometry::nodesCount() const
{
    return Agros2D::problem()->scene()->nodes->count();
}

int PyGeometry::edgesCount() const
{
    return Agros2D::problem()->scene()->faces->count();
}

int PyGeometry::labelsCount() const
{
    return Agros2D::problem()->scene()->labels->count();
}

int PyGeometry::addNode(std::string x, std::string y)
{
    PointValue point = PointValue(Value(Agros2D::problem(), QString::fromStdString(x)),
                                  Value(Agros2D::problem(), QString::fromStdString(y)));

    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Axisymmetric && point.numberX() < 0.0)
        throw out_of_range(QObject::tr("Radial component must be greater then or equal to zero.").toStdString());

    foreach (SceneNode *node, Agros2D::problem()->scene()->nodes->items())
    {
        if (node->point() == point.point())
            throw logic_error(QObject::tr("Node already exist.").toStdString());
    }

    SceneNode *node = Agros2D::problem()->scene()->addNode(new SceneNode(Agros2D::problem()->scene(), point));
    return Agros2D::problem()->scene()->nodes->items().indexOf(node);
}

int PyGeometry::addEdge(std::string x1, std::string y1, std::string x2, std::string y2, std::string angle, int segments, int curvilinear,
                        const map<std::string, int> &refinements, const map<std::string, std::string> &boundaries)
{
    PointValue pointStart = PointValue(Value(Agros2D::problem(), QString::fromStdString(x1)),
                                       Value(Agros2D::problem(), QString::fromStdString(y1)));
    PointValue pointEnd = PointValue(Value(Agros2D::problem(), QString::fromStdString(x2)),
                                     Value(Agros2D::problem(), QString::fromStdString(y2)));
    Value valueAngle = Value(Agros2D::problem(), QString::fromStdString(angle));

    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Axisymmetric &&
            (pointStart.numberX() < 0.0 || pointEnd.numberX() < 0.0))
        throw out_of_range(QObject::tr("Radial component must be greater then or equal to zero.").toStdString());

    testAngle(valueAngle.number());
    testSegments(segments);

    foreach (SceneFace *edge, Agros2D::problem()->scene()->faces->items())
    {
        if (almostEqualRelAndAbs(edge->nodeStart()->point().x, pointStart.numberX(), POINT_ABS_ZERO, POINT_REL_ZERO) &&
                almostEqualRelAndAbs(edge->nodeEnd()->point().x, pointEnd.numberX(), POINT_ABS_ZERO, POINT_REL_ZERO) &&
                almostEqualRelAndAbs(edge->nodeStart()->point().y, pointStart.numberY(), POINT_ABS_ZERO, POINT_REL_ZERO) &&
                almostEqualRelAndAbs(edge->nodeEnd()->point().y, pointEnd.numberY(), POINT_ABS_ZERO, POINT_REL_ZERO))
            throw logic_error(QObject::tr("Edge already exist.").toStdString());
    }

    SceneNode *nodeStart = new SceneNode(Agros2D::problem()->scene(), pointStart);
    nodeStart = Agros2D::problem()->scene()->addNode(nodeStart);
    SceneNode *nodeEnd = new SceneNode(Agros2D::problem()->scene(), pointEnd);
    nodeEnd = Agros2D::problem()->scene()->addNode(nodeEnd);
    SceneFace *edge = new SceneFace(Agros2D::problem()->scene(), nodeStart, nodeEnd, valueAngle, segments, curvilinear);

    try
    {
        setBoundaries(edge, boundaries);       
    }
    catch (std::exception& e)
    {
        delete edge;
        throw;
    }

    Agros2D::problem()->scene()->addFace(edge);

    return Agros2D::problem()->scene()->faces->items().indexOf(edge);
}

int PyGeometry::addEdgeByNodes(int nodeStartIndex, int nodeEndIndex, std::string angle, int segments, int curvilinear,
                               const map<std::string, int> &refinements, const map<std::string, std::string> &boundaries)
{
    Value valueAngle = Value(Agros2D::problem(), QString::fromStdString(angle));

    if (Agros2D::problem()->scene()->nodes->isEmpty())
        throw out_of_range(QObject::tr("Geometry does not contain nodes.").toStdString());

    if (nodeStartIndex == nodeEndIndex)
        throw logic_error(QObject::tr("Start node index is the same as index of end node.").toStdString());

    if (nodeStartIndex > (Agros2D::problem()->scene()->nodes->length() - 1) || nodeStartIndex < 0)
        throw out_of_range(QObject::tr("Node with index '%1' does not exist.").arg(nodeStartIndex).toStdString());
    if (nodeEndIndex > (Agros2D::problem()->scene()->nodes->length() - 1) || nodeEndIndex < 0)
        throw out_of_range(QObject::tr("Node with index '%1' does not exist.").arg(nodeEndIndex).toStdString());

    testAngle(valueAngle.number());
    testSegments(segments);

    foreach (SceneFace *edge, Agros2D::problem()->scene()->faces->items())
    {
        if (Agros2D::problem()->scene()->nodes->items().indexOf(edge->nodeStart()) == nodeStartIndex &&
                Agros2D::problem()->scene()->nodes->items().indexOf(edge->nodeEnd()) == nodeEndIndex)
            throw logic_error(QObject::tr("Edge already exist.").toStdString());
    }

    SceneFace *edge = new SceneFace(Agros2D::problem()->scene(),
                                    Agros2D::problem()->scene()->nodes->at(nodeStartIndex),
                                    Agros2D::problem()->scene()->nodes->at(nodeEndIndex),
                                    valueAngle, segments, curvilinear);

    try
    {
        setBoundaries(edge, boundaries);        
    }
    catch (std::exception& e)
    {
        delete edge;
        throw;
    }

    Agros2D::problem()->scene()->addFace(edge);

    return Agros2D::problem()->scene()->faces->items().indexOf(edge);
}

void PyGeometry::modifyEdge(int index, std::string angle, int segments, int isCurvilinear, const map<std::string, int> &refinements, const map<std::string, std::string> &boundaries)
{
    Value valueAngle = Value(Agros2D::problem(), QString::fromStdString(angle));

    if (Agros2D::problem()->scene()->faces->isEmpty())
        throw out_of_range(QObject::tr("No edges are defined.").toStdString());

    if (index < 0 || index >= Agros2D::problem()->scene()->faces->length())
        throw out_of_range(QObject::tr("Edge index must be between 0 and '%1'.").arg(Agros2D::problem()->scene()->faces->length()-1).toStdString());

    testAngle(valueAngle.number());
    testSegments(segments);

    SceneFace *edge = Agros2D::problem()->scene()->faces->items().at(index);

    edge->setAngleValue(valueAngle);
    edge->setSegments(segments);
    edge->setCurvilinear(isCurvilinear);

    setBoundaries(edge, boundaries);

    Agros2D::problem()->scene()->invalidate();
}

void PyGeometry::testAngle(double angle) const
{
    if (angle < 0.0 || angle > 90.0)
        throw out_of_range(QObject::tr("Angle '%1' is out of range.").arg(angle).toStdString());
}

void PyGeometry::testSegments(int segments) const
{
    if (segments < 3 || segments > 20)
        throw out_of_range(QObject::tr("Segments '%1' is out of range.").arg(segments).toStdString());
}

void PyGeometry::setBoundaries(SceneFace *edge, const map<std::string, std::string> &boundaries)
{
    for (map<std::string, std::string>::const_iterator i = boundaries.begin(); i != boundaries.end(); ++i)
    {
        QString field = QString::fromStdString((*i).first);
        QString marker = QString::fromStdString((*i).second);

        if (!Agros2D::problem()->hasField(field))
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(field).toStdString());

        bool assigned = false;
        foreach (SceneBoundary *boundary, Agros2D::problem()->scene()->boundaries->filter(Agros2D::problem()->fieldInfo(field)).items())
        {
            if (boundary->name() == marker)
            {
                assigned = true;
                edge->addMarker(boundary);
                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Boundary condition '%1' doesn't exists.").arg(marker).toStdString());
    }
}

int PyGeometry::addLabel(std::string x, std::string y, double area, const map<std::string, int> &refinements,
                         const map<std::string, int> &orders, const map<std::string, std::string> &materials)
{
    PointValue point = PointValue(Value(Agros2D::problem(), QString::fromStdString(x)),
                                  Value(Agros2D::problem(), QString::fromStdString(y)));

    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Axisymmetric && point.numberX() < 0.0)
        throw out_of_range(QObject::tr("Radial component must be greater then or equal to zero.").toStdString());

    if (area < 0.0)
        throw out_of_range(QObject::tr("Area must be positive.").toStdString());

    foreach (SceneLabel *label, Agros2D::problem()->scene()->labels->items())
    {
        if (label->point() == point.point())
            throw logic_error(QObject::tr("Label already exist.").toStdString());
    }

    SceneLabel *label = new SceneLabel(Agros2D::problem()->scene(), point, area);

    try
    {
        setMaterials(label, materials);
        setRefinements(label, refinements);
        setPolynomialOrders(label, orders);
    }
    catch (std::exception& e)
    {
        delete label;
        throw;
    }

    Agros2D::problem()->scene()->addLabel(label);

    return Agros2D::problem()->scene()->labels->items().indexOf(label);
}

void PyGeometry::modifyLabel(int index, double area, const map<std::string, int> &refinements,
                             const map<std::string, int> &orders, const map<std::string, std::string> &materials)
{
    if (Agros2D::problem()->scene()->labels->isEmpty())
        throw out_of_range(QObject::tr("No labels are defined.").toStdString());

    if (index < 0 || index >= Agros2D::problem()->scene()->labels->length())
        throw out_of_range(QObject::tr("Label index must be between 0 and '%1'.").arg(Agros2D::problem()->scene()->labels->length()-1).toStdString());

    if (area < 0.0)
        throw out_of_range(QObject::tr("Area must be positive.").toStdString());

    SceneLabel *label = Agros2D::problem()->scene()->labels->at(index);

    label->setArea(area);
    setMaterials(label, materials);
    setRefinements(label, refinements);
    setPolynomialOrders(label, orders);

    Agros2D::problem()->scene()->invalidate();
}

void PyGeometry::setMaterials(SceneLabel *label, const map<std::string, std::string> &materials)
{
    for( map<std::string, std::string>::const_iterator i = materials.begin(); i != materials.end(); ++i)
    {
        QString field = QString::fromStdString((*i).first);
        QString marker = QString::fromStdString((*i).second);

        if (!Agros2D::problem()->hasField(field))
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(field).toStdString());

        if (marker != "none")
        {

            bool assigned = false;
            foreach (SceneMaterial *material, Agros2D::problem()->scene()->materials->filter(Agros2D::problem()->fieldInfo(field)).items())
            {
                if ((material->fieldId() == field) && (material->name() == marker))
                {
                    assigned = true;
                    label->addMarker(material);
                    break;
                }
            }

            if (!assigned)
                throw invalid_argument(QObject::tr("Material '%1' doesn't exists.").arg(marker).toStdString());
        }
    }
}

void PyGeometry::setRefinements(SceneLabel *label, const map<std::string, int> &refinements)
{
    for (map<std::string, int>::const_iterator i = refinements.begin(); i != refinements.end(); ++i)
    {
        QString field = QString::fromStdString((*i).first);
        int refinement = (*i).second;

        if (!Agros2D::problem()->hasField(field))
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(field).toStdString());

        if ((refinement < 0) || (refinement > 10))
            throw out_of_range(QObject::tr("Number of refinements '%1' is out of range (0 - 10).").arg(refinement).toStdString());

        Agros2D::problem()->fieldInfo(field)->setLabelRefinement(label, refinement);
    }
}

void PyGeometry::setPolynomialOrders(SceneLabel *label, const map<std::string, int> &orders)
{
    for (map<std::string, int>::const_iterator i = orders.begin(); i != orders.end(); ++i)
    {
        QString field = QString::fromStdString((*i).first);
        int order = (*i).second;

        if (!Agros2D::problem()->hasField(field))
            throw invalid_argument(QObject::tr("Invalid field id '%1'.").arg(field).toStdString());

        if ((order < 1) || (order > 10))
            throw out_of_range(QObject::tr("Polynomial order '%1' is out of range (1 - 10).").arg(order).toStdString());

        Agros2D::problem()->fieldInfo(field)->setLabelPolynomialOrder(label, order);
    }
}

void PyGeometry::removeNodes(const vector<int> &nodes)
{
    Agros2D::problem()->scene()->selectNone();

    if (!nodes.empty())
    {
        for (vector<int>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros2D::problem()->scene()->nodes->length()))
                Agros2D::problem()->scene()->nodes->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Node index must be between 0 and '%1'.").arg(Agros2D::problem()->scene()->nodes->length()-1).toStdString());
        }
    }
    else
        Agros2D::problem()->scene()->selectAll(SceneGeometryMode_OperateOnNodes);

    Agros2D::problem()->scene()->deleteSelected();
}

void PyGeometry::removeEdges(const vector<int> &edges)
{
    Agros2D::problem()->scene()->selectNone();

    if (!edges.empty())
    {
        for (vector<int>::const_iterator it = edges.begin(); it != edges.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros2D::problem()->scene()->faces->length()))
                Agros2D::problem()->scene()->faces->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Edge index must be between 0 and '%1'.").arg(Agros2D::problem()->scene()->faces->length()-1).toStdString());
        }
    }
    else
        Agros2D::problem()->scene()->selectAll(SceneGeometryMode_OperateOnEdges);

    Agros2D::problem()->scene()->deleteSelected();
}

void PyGeometry::removeLabels(const vector<int> &labels)
{
    Agros2D::problem()->scene()->selectNone();

    if (!labels.empty())
    {
        for (vector<int>::const_iterator it = labels.begin(); it != labels.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros2D::problem()->scene()->labels->length()))
                Agros2D::problem()->scene()->labels->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Label index must be between 0 and '%1'.").arg(Agros2D::problem()->scene()->labels->length()-1).toStdString());
        }
    }
    else
        Agros2D::problem()->scene()->selectAll(SceneGeometryMode_OperateOnLabels);

    Agros2D::problem()->scene()->deleteSelected();
}

void PyGeometry::selectNodes(const vector<int> &nodes)
{
    Agros2D::problem()->scene()->selectNone();

    if (!nodes.empty())
    {
        for (vector<int>::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros2D::problem()->scene()->nodes->length()))
                Agros2D::problem()->scene()->nodes->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Node index must be between 0 and '%1'.").arg(Agros2D::problem()->scene()->nodes->length()-1).toStdString());
        }
    }
    else
    {
        Agros2D::problem()->scene()->selectAll(SceneGeometryMode_OperateOnNodes);
    }
}

void PyGeometry::selectEdges(const vector<int> &edges)
{
    Agros2D::problem()->scene()->selectNone();

    if (!edges.empty())
    {
        for (vector<int>::const_iterator it = edges.begin(); it != edges.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros2D::problem()->scene()->faces->length()))
                Agros2D::problem()->scene()->faces->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Edge index must be between 0 and '%1'.").arg(Agros2D::problem()->scene()->faces->length()-1).toStdString());
        }
    }
    else
    {
        Agros2D::problem()->scene()->selectAll(SceneGeometryMode_OperateOnEdges);
    }
}

void PyGeometry::selectLabels(const vector<int> &labels)
{
    Agros2D::problem()->scene()->selectNone();

    if (!labels.empty())
    {
        for (vector<int>::const_iterator it = labels.begin(); it != labels.end(); ++it)
        {
            if ((*it >= 0) && (*it < Agros2D::problem()->scene()->labels->length()))
                Agros2D::problem()->scene()->labels->at(*it)->setSelected(true);
            else
                throw out_of_range(QObject::tr("Label index must be between 0 and '%1'.").arg(Agros2D::problem()->scene()->labels->length()-1).toStdString());
        }
    }
    else
    {
        Agros2D::problem()->scene()->selectAll(SceneGeometryMode_OperateOnLabels);
    }
}

void PyGeometry::selectNodeByPoint(double x, double y)
{
    Agros2D::problem()->scene()->selectNone();

    SceneNode *node = SceneNode::findClosestNode(Agros2D::problem()->scene(), Point(x, y));
    if (node)
        node->setSelected(true);
    else
        throw logic_error(QObject::tr("There are no nodes around the point [%1, %2].").arg(x).arg(y).toStdString());
}

void PyGeometry::selectEdgeByPoint(double x, double y)
{
    Agros2D::problem()->scene()->selectNone();

    SceneFace *edge = SceneFace::findClosestFace(Agros2D::problem()->scene(), Point(x, y));
    if (edge)
        edge->setSelected(true);
    else
        throw logic_error(QObject::tr("There are no edges around the point [%1, %2].").arg(x).arg(y).toStdString());
}

void PyGeometry::selectLabelByPoint(double x, double y)
{
    Agros2D::problem()->scene()->selectNone();

    SceneLabel *label = SceneLabel::findClosestLabel(Agros2D::problem()->scene(), Point(x, y));
    if (label)
        label->setSelected(true);
    else
        throw logic_error(QObject::tr("There are no labels around the point [%1, %2].").arg(x).arg(y).toStdString());
}

void PyGeometry::selectNone()
{
    Agros2D::problem()->scene()->selectNone();
}

void PyGeometry::moveSelection(double dx, double dy, bool copy, bool withMarkers)
{
    Agros2D::problem()->scene()->transformTranslate(Point(dx, dy), copy, withMarkers);
}

void PyGeometry::rotateSelection(double x, double y, double angle, bool copy, bool withMarkers)
{
    Agros2D::problem()->scene()->transformRotate(Point(x, y), angle, copy, withMarkers);
}

void PyGeometry::scaleSelection(double x, double y, double scale, bool copy, bool withMarkers)
{
    Agros2D::problem()->scene()->transformScale(Point(x, y), scale, copy, withMarkers);
}

void PyGeometry::removeSelection()
{
    Agros2D::problem()->scene()->deleteSelected();
}

void PyGeometry::exportVTK(const std::string &fileName) const
{
    Agros2D::problem()->scene()->exportVTKGeometry(QString::fromStdString(fileName));
}

void PyGeometry::exportSVG(const std::string &fileName) const
{
    QString geometry = generateSvgGeometry(Agros2D::problem()->scene()->faces->items());
    writeStringContent(QString::fromStdString(fileName), geometry);
}

std::string PyGeometry::exportSVG() const
{
    return generateSvgGeometry(Agros2D::problem()->scene()->faces->items()).toStdString();
}
