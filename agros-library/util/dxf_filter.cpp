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

#include "dxf_filter.h"

#include "util/util.h"

#include "util/global.h"

#include "scene.h"
#include "solver/problem.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

DxfInterfaceDXFRW::DxfInterfaceDXFRW(Scene *scene, const QString &fileName) : m_isBlock(false)
{
    this->m_scene = scene;
    m_dxf = new dxfRW(fileName.toStdString().c_str());
}

void DxfInterfaceDXFRW::read()
{
    m_isBlock = false;

    m_dxf->read(this, true);
}

void DxfInterfaceDXFRW::write()
{
    m_dxf->write(this, DRW::AC1015, false);
}

void DxfInterfaceDXFRW::writeAppId()
{
    DRW_AppId ai;
    ai.name ="Agros";

    m_dxf->writeAppId(&ai);
}

void DxfInterfaceDXFRW::addLine(const DRW_Line &l)
{
    if (!m_isBlock)
    {
        // start node
        SceneNode *nodeStart = m_scene->addNode(new SceneNode(m_scene,
                                                              Point(l.basePoint.x, l.basePoint.y)));
        // end node
        SceneNode *nodeEnd = m_scene->addNode(new SceneNode(m_scene,
                                                            Point(l.secPoint.x, l.secPoint.y)));

        // edge
        m_scene->addFace(new SceneFace(m_scene,
                                       nodeStart, nodeEnd, 0));
    }
    else
    {
        m_activeInsert.lines[m_activeInsert.blockName].append(l);
    }
}

void DxfInterfaceDXFRW::addArc(const DRW_Arc& a)
{
    if (!m_isBlock)
    {
        double angle1 = a.staangle / M_PI * 180.0;
        double angle2 = a.endangle / M_PI * 180.0;

        while (angle1 < 0.0) angle1 += 360.0;
        while (angle1 >= 360.0) angle1 -= 360.0;
        while (angle2 < 0.0) angle2 += 360.0;
        while (angle2 >= 360.0) angle2 -= 360.0;

        // start node
        SceneNode *nodeStart = m_scene->addNode(new SceneNode(m_scene,
                                                              Point(a.basePoint.x + a.radious*cos(angle1/180.0*M_PI),
                                                                    a.basePoint.y + a.radious*sin(angle1/180.0*M_PI))));
        // end node
        SceneNode *nodeEnd = m_scene->addNode(new SceneNode(m_scene,
                                                            Point(a.basePoint.x + a.radious*cos(angle2/180.0*M_PI),
                                                                  a.basePoint.y + a.radious*sin(angle2/180.0*M_PI))));
        // edge
        m_scene->addFace(new SceneFace(m_scene,
                                       nodeStart, nodeEnd, Value(m_scene->parentProblem(), angle1 < angle2 ? angle2-angle1 : angle2+360.0-angle1)));
    }
    else
    {
        m_activeInsert.arcs[m_activeInsert.blockName].append(a);
    }
}

void DxfInterfaceDXFRW::addCircle(const DRW_Circle &c)
{
    if (!m_isBlock)
    {
        // nodes
        SceneNode *node1 = m_scene->addNode(new SceneNode(m_scene, Point(c.basePoint.x + c.radious, c.basePoint.y)));
        SceneNode *node2 = m_scene->addNode(new SceneNode(m_scene, Point(c.basePoint.x, c.basePoint.y + c.radious)));
        SceneNode *node3 = m_scene->addNode(new SceneNode(m_scene, Point(c.basePoint.x - c.radious, c.basePoint.y)));
        SceneNode *node4 = m_scene->addNode(new SceneNode(m_scene, Point(c.basePoint.x, c.basePoint.y - c.radious)));

        // edges
        m_scene->addFace(new SceneFace(m_scene, node1, node2, Value(m_scene->parentProblem(), 90)));
        m_scene->addFace(new SceneFace(m_scene, node2, node3, Value(m_scene->parentProblem(), 90)));
        m_scene->addFace(new SceneFace(m_scene, node3, node4, Value(m_scene->parentProblem(), 90)));
        m_scene->addFace(new SceneFace(m_scene, node4, node1, Value(m_scene->parentProblem(), 90)));
    }
}

void DxfInterfaceDXFRW::addPolyline(const DRW_Polyline& data)
{
    for (size_t i = 0; i < data.vertlist.size() - 1; i++)
    {
        const std::shared_ptr<DRW_Vertex> vertStart = data.vertlist.at(i);
        const std::shared_ptr<DRW_Vertex> vertEnd = data.vertlist.at(i+1);

        SceneNode *nodeStart = m_scene->addNode(new SceneNode(m_scene, Point(vertStart->basePoint.x, vertStart->basePoint.y)));
        SceneNode *nodeEnd = m_scene->addNode(new SceneNode(m_scene, Point(vertEnd->basePoint.x, vertEnd->basePoint.y)));

        m_scene->addFace(new SceneFace(m_scene, nodeStart, nodeEnd, Value(m_scene->parentProblem())));
    }
}

void DxfInterfaceDXFRW::addLWPolyline(const DRW_LWPolyline& data)
{
    for (size_t i = 0; i < data.vertlist.size() - 1; i++)
    {
        const std::shared_ptr<DRW_Vertex2D> vertStart = data.vertlist.at(i);
        const std::shared_ptr<DRW_Vertex2D> vertEnd = data.vertlist.at(i+1);

        SceneNode *nodeStart = m_scene->addNode(new SceneNode(m_scene, Point(vertStart->x, vertStart->y)));
        SceneNode *nodeEnd = m_scene->addNode(new SceneNode(m_scene, Point(vertEnd->x, vertEnd->y)));

        m_scene->addFace(new SceneFace(m_scene, nodeStart, nodeEnd, Value(m_scene->parentProblem())));
    }
}

void DxfInterfaceDXFRW::addSpline(const DRW_Spline *data)
{
    // 1st order
    if (data->degree == 1)
    {
        // first and last point
        const std::shared_ptr<DRW_Coord> vertStart = data->controllist.at(0);
        const std::shared_ptr<DRW_Coord> vertEnd = data->controllist.at(data->controllist.size() - 1);

        SceneNode *nodeStart = m_scene->addNode(new SceneNode(m_scene, Point(vertStart->x, vertStart->y)));
        SceneNode *nodeEnd = m_scene->addNode(new SceneNode(m_scene, Point(vertEnd->x, vertEnd->y)));

        m_scene->addFace(new SceneFace(m_scene, nodeStart, nodeEnd, Value(m_scene->parentProblem())));
    }
    else if (data->degree > 1)
    {
        qDebug() << "spline > 1nd order - not implemented";

        // first and last point
        const std::shared_ptr<DRW_Coord> vertStart = data->controllist.at(0);
        const std::shared_ptr<DRW_Coord> vertEnd = data->controllist.at(data->controllist.size() - 1);

        SceneNode *nodeStart = m_scene->addNode(new SceneNode(m_scene, Point(vertStart->x, vertStart->y)));
        SceneNode *nodeEnd = m_scene->addNode(new SceneNode(m_scene, Point(vertEnd->x, vertEnd->y)));

        m_scene->addFace(new SceneFace(m_scene, nodeStart, nodeEnd, Value(m_scene->parentProblem())));
    }
}

void DxfInterfaceDXFRW::addBlock(const DRW_Block& data)
{
    m_activeInsert.blockName = QString::fromStdString(data.name);
    // m_activeInsert.lines.clear();
    m_isBlock = true;

    // qDebug() << "addBlock" << QString::fromStdString(data.name);
}

void DxfInterfaceDXFRW::setBlock(const int handle)
{
    // qDebug() << "setBlock" << handle;
}

void DxfInterfaceDXFRW::endBlock()
{
    // qDebug() << "endBlock";

    m_isBlock = false;
}

void DxfInterfaceDXFRW::addInsert(const DRW_Insert& data)
{
    // line
    foreach (DRW_Line line, m_activeInsert.lines[QString::fromStdString(data.name)])
    {
        DRW_Line newLine;

        newLine.basePoint.x = (data.basePoint.x + data.xscale * sqrt(line.basePoint.x*line.basePoint.x + line.basePoint.y*line.basePoint.y) * cos(atan2(line.basePoint.y, line.basePoint.x) + data.angle / 180.0 * M_PI));
        newLine.basePoint.y = (data.basePoint.y + data.yscale * sqrt(line.basePoint.x*line.basePoint.x + line.basePoint.y*line.basePoint.y) * sin(atan2(line.basePoint.y, line.basePoint.x) + data.angle / 180.0 * M_PI));
        newLine.secPoint.x = (data.basePoint.x + data.xscale * sqrt(line.secPoint.x*line.secPoint.x + line.secPoint.y*line.secPoint.y) * cos(atan2(line.secPoint.y, line.secPoint.x) + data.angle / 180.0 * M_PI));
        newLine.secPoint.y = (data.basePoint.y + data.yscale * sqrt(line.secPoint.x*line.secPoint.x + line.secPoint.y*line.secPoint.y) * sin(atan2(line.secPoint.y, line.secPoint.x) + data.angle / 180.0 * M_PI));

        addLine(newLine);
    }
    // arcs
    foreach (DRW_Arc arc, m_activeInsert.arcs[QString::fromStdString(data.name)])
    {
        DRW_Arc newArc;

        newArc.basePoint.x = (data.basePoint.x + data.xscale * sqrt(arc.basePoint.x*arc.basePoint.x + arc.basePoint.y*arc.basePoint.y) * cos(atan2(arc.basePoint.y, arc.basePoint.x) + data.angle / 180.0 * M_PI));
        newArc.basePoint.y = (data.basePoint.y + data.yscale * sqrt(arc.basePoint.x*arc.basePoint.x + arc.basePoint.y*arc.basePoint.y) * sin(atan2(arc.basePoint.y, arc.basePoint.x) + data.angle / 180.0 * M_PI));
        newArc.radious = arc.radious;
        newArc.staangle = arc.staangle + data.angle / 180.0 * M_PI;
        newArc.endangle = arc.endangle + data.angle / 180.0 * M_PI;

        // qDebug() << "base: " << newArc.basePoint.x << newArc.basePoint.y <<
        //             "data angle: " << data.angle
        //          << "arc : " << arc.staangle / M_PI * 180.0 << arc.endangle / M_PI * 180.0
        //          << atan2(arc.basePoint.y, arc.basePoint.x) / M_PI * 180.0 << data.xscale << data.yscale;

        addArc(newArc);
    }
}

void DxfInterfaceDXFRW::writeHeader(DRW_Header& data)
{
    // bounding box
    RectPoint box = m_scene->boundingBox();

    data.addCoord("$EXTMIN", DRW_Coord(box.start.x, box.start.y, 0.0), 0);
    data.addCoord("$EXTMAX", DRW_Coord(box.end.x, box.end.y, 0.0), 0);
}

void DxfInterfaceDXFRW::writeEntities()
{
    // edges
    foreach (SceneFace *edge, m_scene->faces->items())
    {
        if (fabs(edge->angle()) < EPS_ZERO)
        {
            // line
            double x1 = edge->nodeStart()->point().x;
            double y1 = edge->nodeStart()->point().y;
            double x2 = edge->nodeEnd()->point().x;
            double y2 = edge->nodeEnd()->point().y;

            DRW_Line line;
            line.basePoint.x = x1;
            line.basePoint.y = y1;
            line.secPoint.x = x2;
            line.secPoint.y = y2;
            line.layer = "AGROS2D";
            line.color = 256;
            line.color24 = -1;
            line.lWeight = DRW_LW_Conv::widthDefault;
            line.lineType = "BYLAYER";

            m_dxf->writeLine(&line);
        }
        else
        {
            // arc
            double cx = edge->center().x;
            double cy = edge->center().y;
            double radius = edge->radius();
            double angle1 = atan2(cy - edge->nodeStart()->point().y, cx - edge->nodeStart()->point().x)/M_PI*180.0 + 180.0;
            double angle2 = atan2(cy - edge->nodeEnd()->point().y, cx - edge->nodeEnd()->point().x)/M_PI*180.0 + 180.0;

            while (angle1 < 0.0) angle1 += 360.0;
            while (angle1 >= 360.0) angle1 -= 360.0;
            while (angle2 < 0.0) angle2 += 360.0;
            while (angle2 >= 360.0) angle2 -= 360.0;

            DRW_Arc arc;
            arc.basePoint.x = cx;
            arc.basePoint.y = cy;
            arc.radious = radius;
            arc.staangle = angle1 / 180.0 * M_PI;
            arc.endangle = angle2 / 180.0 * M_PI;
            arc.layer = "AGROS2D";
            arc.color = 256;
            arc.color24 = -1;
            arc.lWeight = DRW_LW_Conv::widthDefault;
            arc.lineType = "BYLAYER";

            m_dxf->writeArc(&arc);
        }
    }
}


// *******************************************************************************

void readFromDXF(Scene *scene, const QString &fileName)
{
    // save current locale
    // char *plocale = setlocale (LC_NUMERIC, "");
    // setlocale (LC_NUMERIC, "C");

    DxfInterfaceDXFRW filter(scene, fileName);
    filter.read();

    scene->invalidate();

    // set system locale
    // setlocale(LC_NUMERIC, plocale);
}

void writeToDXF(Scene *scene, const QString &fileName)
{
    // save current locale
    char *plocale = setlocale (LC_NUMERIC, "");
    setlocale (LC_NUMERIC, "C");

    scene->blockSignals(true);

    DxfInterfaceDXFRW filter(scene, fileName);
    filter.write();

    scene->blockSignals(false);
    scene->invalidate();

    // set system locale
    setlocale(LC_NUMERIC, plocale);
}
