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

#include "loops.h"

#include "util/global.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "solver/problem.h"
#include "solver/problem_config.h"
#include "logview.h"

#ifdef DEAL_II_WITH_TBB
#include "tbb/tbb.h"
tbb::mutex processMutex;
#endif

MeshGeneratorTriangleFast::MeshGeneratorTriangleFast(ProblemBase *problem)
    : MeshGenerator(problem), m_loopsInfo(nullptr)
{
}


bool MeshGeneratorTriangleFast::mesh()
{
    Agros::log()->printMessage(tr("Fast Mesh Generator"), tr("Triangle"));

    clear();

    // process polygon and loops (find laying labels)
    m_loopsInfo->processPolygons();

    // create triangle files
    if (writeToTriangle())
    {
        // convert triangle mesh
        if (readTriangleMeshFormat())
             return true;
    }

    return false;
}

void MeshGeneratorTriangleFast::clear()
{
    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    m_polygonTriangles.clear();

    if (m_loopsInfo == nullptr)
        m_loopsInfo = QSharedPointer<LoopsInfo>(new LoopsInfo(m_problem->scene()));
    else
        m_loopsInfo->clear();
}

bool MeshGeneratorTriangleFast::writeToTriangle()
{
    // basic check
    if (m_problem->scene()->nodes->length() < 3)
    {
        Agros::log()->printWarning(tr("Fast Mesh Generator"), tr("Invalid number of nodes (%1 < 3)").arg(m_problem->scene()->nodes->length()));
        return false;
    }
    if (m_problem->scene()->faces->length() < 3)
    {
        Agros::log()->printWarning(tr("Fast Mesh Generator"), tr("Invalid number of edges (%1 < 3)").arg(m_problem->scene()->faces->length()));
        return false;
    }

    struct triangulateio triIn;

    // nodes
    QList<MeshNode> inNodes;
    int nodesCount = 0;
    for (int i = 0; i<m_problem->scene()->nodes->length(); i++)
    {
        inNodes.append(MeshNode(i,
                                m_problem->scene()->nodes->at(i)->point().x,
                                m_problem->scene()->nodes->at(i)->point().y,
                                0));
        nodesCount++;
    }

    // edges
    QList<MeshEdge> inEdges;
    int edgesCount = 0;
    for (int i = 0; i<m_problem->scene()->faces->length(); i++)
    {
        if (m_problem->scene()->faces->at(i)->angle() == 0)
        {
            inEdges.append(MeshEdge(m_problem->scene()->nodes->items().indexOf(m_problem->scene()->faces->at(i)->nodeStart()),
                                    m_problem->scene()->nodes->items().indexOf(m_problem->scene()->faces->at(i)->nodeEnd()),
                                    i+1));

            edgesCount++;
        }
        else
        {
            // arc
            // add pseudo nodes
            Point center = m_problem->scene()->faces->at(i)->center();
            double radius = m_problem->scene()->faces->at(i)->radius();
            double startAngle = atan2(center.y - m_problem->scene()->faces->at(i)->nodeStart()->point().y,
                                      center.x - m_problem->scene()->faces->at(i)->nodeStart()->point().x) - M_PI;

            int segments = m_problem->scene()->faces->at(i)->segments() * 2;
            double theta = deg2rad(m_problem->scene()->faces->at(i)->angle()) / double(segments);

            int nodeStartIndex = 0;
            int nodeEndIndex = 0;
            for (int j = 0; j < segments; j++)
            {
                double arc = startAngle + j*theta;

                double x = radius * cos(arc);
                double y = radius * sin(arc);

                nodeEndIndex = nodesCount + 1;
                if (j == 0)
                {
                    nodeStartIndex = m_problem->scene()->nodes->items().indexOf(m_problem->scene()->faces->at(i)->nodeStart());
                    nodeEndIndex = nodesCount;
                }
                if (j == segments - 1)
                {
                    nodeEndIndex = m_problem->scene()->nodes->items().indexOf(m_problem->scene()->faces->at(i)->nodeEnd());
                }
                if ((j > 0) && (j < segments))
                {
                    inNodes.append(MeshNode(nodesCount,
                                            center.x + x,
                                            center.y + y,
                                            0));

                    nodesCount++;
                }

                inEdges.append(MeshEdge(nodeStartIndex,
                                        nodeEndIndex,
                                        i+1));

                edgesCount++;
                nodeStartIndex = nodeEndIndex;
            }
        }
    }

    // labels and holes
    QList<MeshLabel> inLabels;
    int labelsCount = 0;
    QList<MeshNode> inHoles;
    int holesCount = 0;
    foreach (SceneLabel *label, m_problem->scene()->labels->items())
    {
        if (label->markersCount() > 0)
        {
            inLabels.append(MeshLabel(labelsCount,
                                      label->point().x,
                                      label->point().y,
                                      m_problem->scene()->labels->items().indexOf(label) + 1,
                                      label->area()));
            labelsCount++;
        }
        else
        {
            inHoles.append(MeshNode(holesCount,
                                    label->point().x,
                                    label->point().y,
                                    -1));

            holesCount++;
        }
    }

    // vertices
    triIn.numberofpoints = inNodes.count();
    triIn.numberofpointattributes = 0;
    triIn.pointlist = (REAL *) malloc(triIn.numberofpoints * 2 * sizeof(REAL));
    triIn.pointmarkerlist = (int *) malloc(triIn.numberofpoints * sizeof(int));

    for (int i = 0; i < inNodes.count(); i++)
    {
        triIn.pointlist[2*i] = inNodes[i].x;
        triIn.pointlist[2*i+1] = inNodes[i].y;
        triIn.pointmarkerlist[i] = -1;
    }

    // segments
    triIn.numberofsegments = inEdges.count();
    triIn.segmentlist = (int *) malloc(triIn.numberofsegments * 2 * sizeof(int));
    triIn.segmentmarkerlist = (int *) malloc(triIn.numberofsegments * sizeof(int));

    for (int i = 0; i < inEdges.count(); i++)
    {
        triIn.segmentlist[2*i] = inEdges[i].node[0];
        triIn.segmentlist[2*i+1] = inEdges[i].node[1];
        triIn.segmentmarkerlist[i] = inEdges[i].marker;
    }

    // regions
    triIn.numberofregions = inLabels.count();
    triIn.regionlist = (REAL *) malloc(triIn.numberofregions * 4 * sizeof(REAL));

    for (int i = 0; i < inLabels.count(); i++)
    {
        triIn.regionlist[4*i] = inLabels[i].x;
        triIn.regionlist[4*i+1] = inLabels[i].y;
        triIn.regionlist[4*i+2] = inLabels[i].marker;
        triIn.regionlist[4*i+3] = inLabels[i].area;
    }

    // holes
    triIn.numberofholes = inHoles.count();
    triIn.holelist = (REAL *) malloc(triIn.numberofholes * 2 * sizeof(REAL));

    for (int i = 0; i < inHoles.count(); i++)
    {
        triIn.holelist[2*i] = inHoles[i].x;
        triIn.holelist[2*i+1] = inHoles[i].y;
    }

    // report(&triIn, 1, 0, 0, 1, 0, 0);

    triOut.pointlist = (REAL *) NULL;            /* Not needed if -N switch used. */
    triOut.pointmarkerlist = (int *) NULL; /* Not needed if -N or -B switch used. */
    triOut.trianglelist = (int *) NULL;          /* Not needed if -E switch used. */
    /* Not needed if -E switch used or number of triangle attributes is zero: */
    triOut.triangleattributelist = (REAL *) NULL;
    triOut.neighborlist = (int *) NULL;         /* Needed only if -n switch used. */
    /* Needed only if segments are output (-p or -c) and -P not used: */
    triOut.segmentlist = (int *) NULL;
    /* Needed only if segments are output (-p or -c) and -P and -B not used: */
    triOut.segmentmarkerlist = (int *) NULL;
    triOut.edgelist = (int *) NULL;             /* Needed only if -e switch used. */
    triOut.edgemarkerlist = (int *) NULL;   /* Needed if -e used and -B not used. */

    //    -p  Triangulates a Planar Straight Line Graph (.poly file).
    //    -r  Refines a previously generated mesh.
    //    -q  Quality mesh generation.  A minimum angle may be specified.
    //    -a  Applies a maximum triangle area constraint.
    //    -u  Applies a user-defined triangle constraint.
    //    -A  Applies attributes to identify triangles in certain regions.
    //    -c  Encloses the convex hull with segments.
    //    -D  Conforming Delaunay:  all triangles are truly Delaunay.
    //    -j  Jettison unused vertices from output .node file.
    //    -e  Generates an edge list.
    //    -v  Generates a Voronoi diagram.
    //    -n  Generates a list of triangle neighbors.
    //    -g  Generates an .off file for Geomview.
    //    -B  Suppresses output of boundary information.
    //    -P  Suppresses output of .poly file.
    //    -N  Suppresses output of .node file.
    //    -E  Suppresses output of .ele file.
    //    -I  Suppresses mesh iteration numbers.
    //    -O  Ignores holes in .poly file.
    //    -X  Suppresses use of exact arithmetic.
    //    -z  Numbers all items starting from zero (rather than one).
    //    -o2 Generates second-order subparametric elements.
    //    -Y  Suppresses boundary segment splitting.
    //    -S  Specifies maximum number of added Steiner points.
    //    -i  Uses incremental method, rather than divide-and-conquer.
    //    -F  Uses Fortune's sweepline algorithm, rather than d-and-c.
    //    -l  Uses vertical cuts only, rather than alternating cuts.
    //    -s  Force segments into mesh by splitting (instead of using CDT).
    //    -C  Check consistency of final mesh.
    //    -Q  Quiet:  No terminal output except errors.
    char const *triSwitches = "peAazInQ";
    triangulate((char *) triSwitches, &triIn, &triOut, (struct triangulateio *) NULL);

    free(triIn.pointlist);
    free(triIn.pointmarkerlist);
    free(triIn.segmentlist);
    free(triIn.segmentmarkerlist);
    free(triIn.regionlist);
    free(triIn.holelist);

    return true;
}

bool MeshGeneratorTriangleFast::readTriangleMeshFormat()
{
    // triangle nodes
    int numberOfNodes = triOut.numberofpoints;
    for (int i = 0; i < numberOfNodes; i++)
    {
        nodeList.append(Point(triOut.pointlist[2 * i],
                        triOut.pointlist[2 * i + 1]));
    }

    // triangle edges
    int numberOfEdges = triOut.numberofedges;
    // for curvature
    std::map<std::pair<int, int>, Point> centers;
    std::map<std::pair<int, int>, double> sizes;
    for (int i = 0; i < numberOfEdges; i++)
    {
        // marker conversion from triangle, where it starts from 1
        edgeList.append(MeshEdge(triOut.edgelist[2 * i], triOut.edgelist[2 * i + 1], triOut.edgemarkerlist[i] - 1));

        if (triOut.edgemarkerlist[i] > 0)
        {
            SceneFace* sceneEdge = m_problem->scene()->faces->at(triOut.edgemarkerlist[i] - 1);

            if (sceneEdge->angle() > 0.0)
            {
                int node_indices[2] = { triOut.edgelist[2 * i], triOut.edgelist[2 * i + 1] };

                centers.insert(std::pair<std::pair<int, int>, Point>(std::pair<int, int>(node_indices[0], node_indices[1]), sceneEdge->center()));
                sizes.insert(std::pair<std::pair<int, int>, double>(std::pair<int, int>(node_indices[0], node_indices[1]), sceneEdge->radius()));

                for (int node_i = 0; node_i < 2; node_i++)
                {
                    Point p = nodeList[node_indices[node_i]];
                    Point c = sceneEdge->center();
                    double r = sceneEdge->radius();
                    nodeList[node_indices[node_i]] = prolong_point_to_arc(p, c, r);
                }
            }
        }
    }
    int edgeCountLinear = edgeList.count();

    // triangle elements
    int numberOfElements = triOut.numberoftriangles;
    for (int i = 0; i < numberOfElements; i++)
    {
        int marker = triOut.triangleattributelist[i];
        if (marker == 0)
        {
            Agros::log()->printError(tr("Mesh Generator"), tr("Some areas do not have a marker"));
            return false;
        }

        // vertices
        int nodeA = triOut.trianglelist[3*i];
        int nodeB = triOut.trianglelist[3*i+1];
        int nodeC = triOut.trianglelist[3*i+2];

        elementList.append(MeshElement(nodeA, nodeB, nodeC, marker - 1)); // marker conversion from triangle, where it starts from 1
    }
    int elementCountLinear = elementList.count();

    // triangle neigh
    for (int i = 0; i < triOut.numberoftriangles; i++)
    {
        elementList[i].neigh[0] = triOut.neighborlist[3*i];
        elementList[i].neigh[1] = triOut.neighborlist[3*i+1];
        elementList[i].neigh[2] = triOut.neighborlist[3*i+2];
    }

    free(triOut.pointlist);
    free(triOut.pointmarkerlist);
    free(triOut.trianglelist);
    free(triOut.triangleattributelist);
    free(triOut.neighborlist);
    free(triOut.segmentlist);
    free(triOut.segmentmarkerlist);
    free(triOut.edgelist);
    free(triOut.edgemarkerlist);

    // elements
    for (int element_i = 0; element_i < elementList.count(); element_i++)
    {
        MeshElement element = elementList[element_i];
        if (element.isUsed)
        {
            SceneLabel *label = m_problem->scene()->labels->at(element.marker);

            if (!m_polygonTriangles.contains(label))
                m_polygonTriangles.insert(label, QList<MeshGeneratorTriangleFast::Triangle>());

            // qInfo() << label << element.node[0] << element.node[1] << element.node[2];
            m_polygonTriangles[label].append(MeshGeneratorTriangleFast::Triangle(
                nodeList[element.node[0]],
                nodeList[element.node[1]],
                nodeList[element.node[2]]));
        }
    }

    // qInfo() << "nodes" << nodeList.size() << "edges" <<  edgeList.size() << "elements" << elementList.size() << "polygonTriangles" << m_polygonTriangles.size();

    return true;
}


const static int LOOPS_NON_EXISTING = -100000;
const static double TOL = 0.001;

// **************************************************************************************

LoopsInfo::LoopsNodeEdgeData::LoopsNodeEdgeData() : node(LOOPS_NON_EXISTING)
{
}

// **************************************************************************************

bool LoopsInfo::LoopsNode::hasUnvisited()
{
    foreach(LoopsNodeEdgeData ned, nodeEdges)
        if(!ned.visited)
            return true;

    return false;
}

LoopsInfo::LoopsNodeEdgeData LoopsInfo::LoopsNode::startLoop()
{
    for (int i = 0; i < nodeEdges.size(); i++){
        LoopsNodeEdgeData ned = nodeEdges.at(i);
        if(!ned.visited){
            nodeEdges[i].visited = true;
            //qDebug() << "start loop " << i << endl;
            return ned;
        }
    }

    assert(0);
}

LoopsInfo::LoopsNodeEdgeData LoopsInfo::LoopsNode::continueLoop(int previousNode)
{
    LoopsNodeEdgeData previousNED;
    int index = -1;
    for (int i = 0; i < nodeEdges.size(); i++){
        LoopsNodeEdgeData ned = nodeEdges.at(i);
        if(ned.node == previousNode){
            previousNED = ned;
            index = i;
            break;
        }
    }

    assert(previousNED.node != LOOPS_NON_EXISTING);
    int nextIdx = (index + 1) % nodeEdges.size();
    //qDebug() << "continue loop, next node:" << nodeEdges[nextIdx].node << ", next edge:" << nodeEdges[nextIdx].edge << ", " << nodeEdges.at(nextIdx).visited;
    //assert(!data.at(nextIdx).visited);
    if (nodeEdges.at(nextIdx).visited)
        throw AgrosGeometryException(QObject::tr("Node %1 connected by edge %2 already visited.").arg(nodeEdges[nextIdx].node).arg(nodeEdges[nextIdx].edge));
    nodeEdges[nextIdx].visited = true;
    return nodeEdges[nextIdx];
}

void LoopsInfo::LoopsNode::insertEdge(int endNode, int edgeIdx, bool reverse, double angle)
{
    int index = 0;

    for (int i = 0; i < nodeEdges.size(); i++)
        if(angle < nodeEdges[i].angle)
            index = i+1;
    nodeEdges.insert(index, LoopsNodeEdgeData(endNode, edgeIdx, reverse, angle));
}

// ***********************************************************************

LoopsInfo::LoopsGraph::LoopsGraph(int numNodes)
{
    for (int i = 0; i < numNodes; i++)
        nodes.push_back(LoopsNode());
}

void LoopsInfo::LoopsGraph::addEdge(int startNode, int endNode, int edgeIdx, double angle)
{
    double angle2 = angle + M_PI;
    if(angle2 >= 2 * M_PI)
        angle2 -= 2 * M_PI;

    nodes[startNode].insertEdge(endNode, edgeIdx, false, angle);
    nodes[endNode].insertEdge(startNode, edgeIdx, true, angle2);
}

void LoopsInfo::LoopsGraph::print()
{
    for (int i = 0; i < nodes.size(); i++)
    {
        qDebug() << "node " << i;
        foreach(LoopsNodeEdgeData ned, nodes[i].nodeEdges)
        {
            qDebug() << "     node " << ned.node << ", edge " << (ned.reverse ? "-" : "") << ned.edge << ", angle " << ned.angle << ", visited " << ned.visited;
        }
    }
    qDebug() << "\n";
}

bool LoopsInfo::isInsideSeg(double angleSegStart, double angleSegEnd, double angle)
{
    if (angleSegEnd > angleSegStart)
    {
        if ((angle >= angleSegStart) && (angle <= angleSegEnd))
            return true;
        else
            return false;
    }
    else
    {
        assert(angleSegStart >= 0);
        assert(angleSegEnd <= 0);
        if((angle >= angleSegStart) || (angle <= angleSegEnd))
            return true;
        else
            return false;
    }
}

LoopsInfo::Intersection LoopsInfo::intersects(Point point, double tangent, SceneFace* edge, Point& intersection)
{
    double x1 = edge->nodeStart()->point().x;
    double y1 = edge->nodeStart()->point().y;
    double x2 = edge->nodeEnd()->point().x;
    double y2 = edge->nodeEnd()->point().y;

    if (edge->angle() != 0)
    {
        double xC = edge->center().x;
        double yC = edge->center().y;

        double coef = - tangent * point.x + point.y - yC;
        double a = 1 + tangent*tangent;
        double b = - 2*xC + 2*tangent*coef;
        double c = xC*xC + coef*coef - edge->radius() * edge->radius();

        double disc = b*b - 4*c*a;
        if(disc <= 0)
            return Intersection_No;
        else
        {
            double xI1 = (-b + sqrt(disc))/ (2*a);
            double xI2 = (-b - sqrt(disc))/ (2*a);

            double yI1 = tangent * (xI1 - point.x) + point.y;
            double yI2 = tangent * (xI2 - point.x) + point.y;

            double angle1 = atan2(yI1 - yC, xI1 - xC);
            double angle2 = atan2(yI2 - yC, xI2 - xC);

            double angleSegStart = atan2(y1 - yC, x1 - xC);
            double angleSegEnd = atan2(y2 - yC, x2 - xC);

            int leftInter = 0;
            int rightInter = 0;

            //qDebug() << "circle center " << xC << ", " << yC << ", radius " << edge->radius() << endl;
            //qDebug() << "xI1 "<< xI1 << ", yI1 "<< yI1 << ", xI2 "<< xI2 << ", yI2 "<< yI2 << endl;
            //qDebug() << "first: anglestart " << angleSegStart << ", end " << angleSegEnd << ", angle1 " << angle1 << ", x1" << x1 << ", point.x " << point.x << ", inside " << isInsideSeg(angleSegStart, angleSegEnd, angle1) << endl;
            //qDebug() << "second: anglestart " << angleSegStart << ", end " << angleSegEnd << ", angle2 " << angle2 << ", x2" << x2 << ", point.x " << point.x << ", inside " << isInsideSeg(angleSegStart, angleSegEnd, angle2) << endl;

            if (isInsideSeg(angleSegStart, angleSegEnd, angle1))
            {
                if(xI1 < point.x)
                    leftInter++;
                else
                    rightInter++;
                intersection.x = xI1;
                intersection.y = yI1;
            }
            if (isInsideSeg(angleSegStart, angleSegEnd, angle2))
            {
                if(xI2 < point.x)
                    leftInter++;
                else
                    rightInter++;
                intersection.x = xI2;
                intersection.y = yI2;
            }

            //qDebug() << "left " << leftInter << ", right " << rightInter << endl;

            if(leftInter == 2)
                return Intersection_No;
            else if(rightInter == 2)
                return Intersection_No;
            else if((leftInter == 1) && (rightInter == 0))
                return Intersection_Left;
            else if((leftInter == 0) && (rightInter == 1))
                return Intersection_Right;
            else if((leftInter == 1) && (rightInter == 1))
                return Intersection_Both;
            else if((leftInter == 0) && (rightInter == 0))
                return Intersection_No;
            else
                assert(0);
        }
    }

    if(fabs(x2-x1) > fabs(y2-y1))
    {
        double tangentSeg = (y2-y1) / (x2-x1);
        if(fabs(tangent - tangentSeg) < TOL)
            return Intersection_Uncertain;
        double xI = (tangentSeg * x1 - tangent * point.x - y1 + point.y) / (tangentSeg - tangent);

        if(fabs(x1 - xI) < TOL * edge->length())
            return Intersection_Uncertain;
        if(fabs(x2 - xI) < TOL * edge->length())
            return Intersection_Uncertain;

        if((xI > max(x1, x2)) || (xI < min(x1, x2)))
            return Intersection_No;

        if(xI < point.x)
            return Intersection_Left;
        else
            return Intersection_Right;
    }
    else
    {
        double invTangentSeg = (x2-x1) / (y2-y1);
        if(fabs(1/tangent - invTangentSeg) < TOL)
            return Intersection_Uncertain;
        double yI = (invTangentSeg * y1 - 1/tangent * point.y - x1 + point.x) / (invTangentSeg - 1/tangent);

        if(fabs(y1 - yI) < TOL * edge->length())
            return Intersection_Uncertain;
        if(fabs(y2 - yI) < TOL * edge->length())
            return Intersection_Uncertain;

        if((yI > max(y1, y2)) || (yI < min(y1, y2)))
            return Intersection_No;

        if(yI < point.y)
            return Intersection_Left;
        else
            return Intersection_Right;
    }
}

LoopsInfo::Intersection LoopsInfo::intersects(Point point, double tangent, SceneFace* edge)
{
    Point intersection;
    return intersects(point, tangent, edge, intersection);
}

// *********************************************************************************************

LoopsInfo::LoopsInfo(Scene *scene) : QObject(), m_scene(scene)
{
}

int LoopsInfo::intersectionsParity(Point point, QList<LoopsNodeEdgeData> loop)
{
    bool rejectTangent;
    double tangent = 0.;
    int left, right;
    do {
        tangent += 0.1;
        if (tangent > 10)
            throw AgrosGeometryException(tr("Intersection parity failed (tangent > 10)."));
        rejectTangent = false;
        left = right = 0;

        foreach (LoopsNodeEdgeData ned, loop)
        {
            Intersection result = intersects(point, tangent, m_scene->faces->at(ned.edge));
            if(result == Intersection_Uncertain)
            {
                rejectTangent = true;
                //qDebug() << "rejected tangent\n";
                break;
            }
            else if(result == Intersection_Left)
                left++;
            else if(result == Intersection_Right)
                right++;
            else if(result == Intersection_Both)
            {
                left++;
                right++;
            }

        }

    }
    while (rejectTangent);

    //qDebug() << "intersections left " << left << ", right " << right << endl;
    assert(left%2 == right%2);
    return left%2;
}

int LoopsInfo::windingNumber(Point point, QList<LoopsNodeEdgeData> loop)
{
    QList<double> angles;
    angles.reserve(loop.size() * 2);
    foreach (LoopsNodeEdgeData ned, loop)
    {
        // use two segments instead of arc. Point on arc to be found as intersection of arc with line going through
        // edge center and point
        SceneFace* edge = m_scene->faces->at(ned.edge);
        if (!edge->isStraight())
        {
            Point intersection;
            Point edgePoint = edge->center();

            // if impossible to define tangent, we use different point on the edge (not center)
            if(fabs(edgePoint.x - point.x)/edge->length() < 0.00001)
                edgePoint = (edge->center() + edge->nodeEnd()->point()) / 2;

            double tangent = (edgePoint.y - point.y) / (edgePoint.x - point.x);
            Intersection intersectionType = intersects(point, tangent, edge, intersection);
            if ((intersectionType == Intersection_Left) || (intersectionType == Intersection_Right))
            {
                double additionalAngle = atan2(intersection.y - point.y,
                                               intersection.x - point.x);
                angles.append(additionalAngle);
            }
        }

        // regular points
        Point nodePoint = m_scene->nodes->at(ned.node)->point();
        double angle = atan2(nodePoint.y - point.y,
                             nodePoint.x - point.x);

        angles.append(angle);
    }

    double totalAngle = 0;
    for (int i = 0; i < angles.size(); i++)
    {
        double angle = angles[(i+1) % angles.size()] - angles[i];
        while (angle >= M_PI) angle -= 2*M_PI;
        while (angle < -M_PI) angle += 2*M_PI;
        assert((angle <= M_PI) && (angle >= -M_PI));

        totalAngle += angle;
    }

    double winding = totalAngle / (2*M_PI);
    int intWinding = floor(winding + 0.5);

    // check that total angle was multiple of 2*M_PI
    assert(fabs(winding - (double) intWinding) < 0.00001);
    return intWinding;
}

bool LoopsInfo::areSameLoops(QList<LoopsNodeEdgeData> loop1, QList<LoopsNodeEdgeData> loop2)
{
    if(loop1.size() != loop2.size())
        return false;

    QList<int> nodes1;
    nodes1.reserve(loop1.size());
    QList<int> nodes2;
    nodes2.reserve(loop2.size());

    foreach(LoopsNodeEdgeData ned, loop1)
        nodes1.push_back(ned.node);
    foreach(LoopsNodeEdgeData ned, loop2)
        nodes2.push_back(ned.node);

    for (int i = 0; i < nodes1.size(); i++)
        if(!nodes2.contains(nodes1.at(i)))
            return false;

    return true;
}

bool LoopsInfo::areEdgeDuplicities(QList<LoopsNodeEdgeData> loop)
{
    for (int i = 0; i < loop.length(); i++)
    {
        LoopsNodeEdgeData data = loop.at(i);

        for (int j = 0; j < loop.length(); j++)
        {
            if (i != j)
            {
                LoopsNodeEdgeData dataTest = loop.at(j);

                if (data.edge == dataTest.edge)
                    return true;
            }
        }
    }

    return false;
}

int LoopsInfo::longerLoop(int idx1, int idx2)
{
    int size1 = m_loops[idx1].size();
    int size2 = m_loops[idx2].size();

    if(size1 > size2)
        return idx1;
    else if(size1 < size2)
        return idx2;
    else
        assert(0);
}

bool LoopsInfo::shareEdge(int idx1, int idx2)
{
    foreach(LoopsNodeEdgeData ned1, m_loops[idx1])
    {
        foreach(LoopsNodeEdgeData ned2, m_loops[idx2])
        {
            if (ned1.edge == ned2.edge)
                return true;
        }
    }
    return false;
}

void LoopsInfo::switchOrientation(int idx)
{
    for (int i = 0; i < m_loops[idx].size() / 2; i++)
        swap(m_loops[idx][i], m_loops[idx][m_loops[idx].size() - 1 - i]);
    for (int i = 0; i < m_loops[idx].size(); i++)
        m_loops[idx][i].reverse = !m_loops[idx][i].reverse;
}

void LoopsInfo::addEdgePoints(QList<Point> *polyline, const SceneFace &edge, bool reverse)
{
    QList<Point> localPolyline;

    if (!reverse)
        localPolyline.append(Point(edge.nodeStart()->point().x,
                                   edge.nodeStart()->point().y));

    if (!edge.isStraight())
    {
        Point center = edge.center();
        double radius = edge.radius();
        double startAngle = atan2(center.y - edge.nodeStart()->point().y,
                                  center.x - edge.nodeStart()->point().x) / M_PI*180.0 - 180.0;

        int segments = edge.angle() / 5.0;
        if (segments < 2) segments = 2;

        double theta = edge.angle() / double(segments);

        for (int i = 1; i < segments; i++)
        {
            double arc = (startAngle + i*theta)/180.0*M_PI;

            double x = radius * cos(arc);
            double y = radius * sin(arc);

            if (reverse)
                localPolyline.insert(0, Point(center.x + x, center.y + y));
            else
                localPolyline.append(Point(center.x + x, center.y + y));
        }
    }

    if (reverse)
        localPolyline.insert(0, Point(edge.nodeEnd()->point().x,
                                      edge.nodeEnd()->point().y));

    polyline->append(localPolyline);
}

void LoopsInfo::processLoops()
{
    if (!m_scene->crossings().empty())
        throw AgrosGeometryException(tr("There are some edges crossed."));

    m_scene->checkTwoNodesSameCoordinates();

    // find loops
    LoopsGraph graph(m_scene->nodes->length());
    for (int edgeIdx = 0; edgeIdx < m_scene->faces->length(); edgeIdx++)
    {
        SceneNode* startNode = m_scene->faces->at(edgeIdx)->nodeStart();
        SceneNode* endNode = m_scene->faces->at(edgeIdx)->nodeEnd();
        int startNodeIdx = m_scene->nodes->items().indexOf(startNode);
        int endNodeIdx = m_scene->nodes->items().indexOf(endNode);

        if (startNodeIdx == endNodeIdx)
            throw AgrosGeometryException(QObject::tr("Edge %1 begins and ends in the same point %2. Remove the edge.").arg(edgeIdx).arg(startNodeIdx));

        double angle = atan2(endNode->point().y - startNode->point().y,
                             endNode->point().x - startNode->point().x);
        if (angle < 0)
            angle += 2 * M_PI;

        graph.addEdge(startNodeIdx, endNodeIdx, edgeIdx, angle);
    }

    // graph.print();

    m_loops.clear();
    for (int i = 0; i < graph.nodes.size(); i++)
    {
        //qDebug() << "** starting with node " << i << endl;
        LoopsNode& node = graph.nodes[i];
        int previousNodeIdx, currentNodeIdx;
        while (node.hasUnvisited())
        {
            //graph.print();
            //qDebug() <<  "has unvisited";

            QList<LoopsNodeEdgeData> loop;
            LoopsNodeEdgeData ned = node.startLoop();
            previousNodeIdx = i;
            loop.push_back(ned);
            do
            {
                currentNodeIdx = ned.node;
                //                qDebug() << "previous " << previousNodeIdx << ", current" << currentNodeIdx;
                LoopsNode& actualNode = graph.nodes[currentNodeIdx];
                ned = actualNode.continueLoop(previousNodeIdx);
                previousNodeIdx = currentNodeIdx;
                loop.push_back(ned);
            } while (ned.node != i);

            if (areEdgeDuplicities(loop))
                throw AgrosGeometryException(QObject::tr("Two loops connected by one edge."));

            // for simple domains, we have the same loop twice. Do not include it second times
            if (m_loops.isEmpty() || !areSameLoops(loop, m_loops.last()))
                m_loops.append(loop);
        }
    }

    QList<QList< SceneLabel* > > labelsInsideLoop;
    QMap<SceneLabel*, QList<int> > loopsContainingLabel;
    QMap<SceneLabel*, int> principalLoopOfLabel;

    QMap<QPair<SceneLabel*, int>, int> windingNumbers;

    // find what labels are inside what loops
    for (int loopIdx = 0; loopIdx < m_loops.size(); loopIdx++)
    {
        labelsInsideLoop.push_back(QList<SceneLabel*>());
        for (int labelIdx = 0; labelIdx < m_scene->labels->count(); labelIdx++)
        {
            SceneLabel* label = m_scene->labels->at(labelIdx);
            int wn = windingNumber(label->point(), m_loops[loopIdx]);
            //qDebug() << "winding number " << wn << endl;
            assert(wn < 2);
            windingNumbers[QPair<SceneLabel*, int>(label, loopIdx)] = wn;
            int ip = intersectionsParity(label->point(), m_loops[loopIdx]);
            // assert(abs(wn) == ip);
            if (ip == 1)
            {
                labelsInsideLoop[loopIdx].push_back(label);
                if(!loopsContainingLabel.contains(label))
                    loopsContainingLabel[label] = QList<int>();
                loopsContainingLabel[label].push_back(loopIdx);
            }
        }
        if (labelsInsideLoop[loopIdx].isEmpty())
            throw AgrosGeometryException(tr("Some areas do not have a marker"));
    }

    for (int labelIdx = 0; labelIdx < m_scene->labels->count(); labelIdx++)
    {
        SceneLabel* label = m_scene->labels->at(labelIdx);
        if(!loopsContainingLabel.contains(label))
            throw AgrosGeometryException(tr("There is a label outside of the domain"));
    }

    // direct super and sub domains (indexed by loop indices)
    QList<int> superDomains;
    QList<QList<int> > subDomains;

    for (int i = 0; i < m_loops.size(); i++)
    {
        superDomains.push_back(-1);
        subDomains.push_back(QList<int>());
    }

    // outiside loops, not to be considered
    m_outsideLoops.clear();
    for (int labelIdx = 0; labelIdx < m_scene->labels->count(); labelIdx++)
    {
        SceneLabel* actualLabel = m_scene->labels->at(labelIdx);
        QList<int> loopsWithLabel = loopsContainingLabel[actualLabel];
        if(loopsWithLabel.size() == 0)
            throw AgrosGeometryException(tr("There is no label in some subdomain"));


        // sort
        for (int i = 0; i < loopsWithLabel.size(); i++)
        {
            for (int j = 0; j < loopsWithLabel.size() - 1; j++)
            {
                int numLabelsJ = labelsInsideLoop[loopsWithLabel[j]].size();
                int numLabelsJPlus1 = labelsInsideLoop[loopsWithLabel[j+1]].size();
                if(numLabelsJ > numLabelsJPlus1)
                    swap(loopsWithLabel[j+1], loopsWithLabel[j]);
            }
        }
        //        assert(labelsInsideLoop[loopsWithLabel[0]].size() == 1);
        //        assert(labelsInsideLoop[loopsWithLabel[0]][0] == actualLabel);

        int indexOfOutmost = loopsWithLabel[loopsWithLabel.size() - 1];
        int indexOfInmost = loopsWithLabel[0];
        principalLoopOfLabel[actualLabel] = indexOfInmost;

        // switch orientation if neccessary
        int windNum = windingNumbers[QPair<SceneLabel*, int>(actualLabel, indexOfInmost)];
        assert(abs(windNum) == 1);
        if(windNum == -1)
            switchOrientation(indexOfInmost);

        if ((labelsInsideLoop[indexOfOutmost].size() > 1) && (indexOfOutmost != indexOfInmost)
                && shareEdge(indexOfOutmost, indexOfInmost))
            m_outsideLoops.append(indexOfOutmost);

        for (int j = 0; j < loopsWithLabel.size() -1; j++)
        {
            int numLabelsJ = labelsInsideLoop[loopsWithLabel[j]].size();
            int numLabelsPlus1 = labelsInsideLoop[loopsWithLabel[j+1]].size();
            if (numLabelsJ == numLabelsPlus1)
                throw AgrosGeometryException(tr("There is no label in some subdomain"));
        }

        for (int i = 0; i < loopsWithLabel.size() - 1; i++)
        {
            int smallerLoop = loopsWithLabel[i];
            int biggerLoop = loopsWithLabel[i+1];
            // assert(superDomains[smallerLoop] == biggerLoop || superDomains[smallerLoop] == -1);
            if ((superDomains[smallerLoop] == biggerLoop || superDomains[smallerLoop] == -1))
            {
                superDomains[smallerLoop] = biggerLoop;
                if(!subDomains[biggerLoop].contains(smallerLoop))
                    subDomains[biggerLoop].append(smallerLoop);
            }
            else
            {
                throw AgrosGeometryException(tr("Unknown error"));
            }
        }
    }

    m_labelLoops.clear();
    for (int labelIdx = 0; labelIdx < m_scene->labels->count(); labelIdx++)
    {
        SceneLabel* label = m_scene->labels->at(labelIdx);
        if(!principalLoopOfLabel.contains(label))
            throw AgrosGeometryException(tr("There is a label outside of the domain"));

        int principalLoop = principalLoopOfLabel[label];
        m_labelLoops[label] = QList<int>();
        m_labelLoops[label].push_back(principalLoop);
        for (int i = 0; i < subDomains[principalLoop].count(); i++)
        {
            m_labelLoops[label].append(subDomains[principalLoop][i]);
        }
    }

    // check for multiple labels
    QList<int> usedLoops;
    foreach (SceneLabel *label, principalLoopOfLabel.keys())
    {
        if (!usedLoops.contains(principalLoopOfLabel[label]))
            usedLoops.append(principalLoopOfLabel[label]);
        else
            throw AgrosGeometryException(tr("There are multiple labels in the domain"));
    }

    // use the loops to determine what is the right and left label for each edge

    foreach(SceneFace* edge, m_scene->faces->items())
    {
        edge->unsetRightLeftLabelIdx();
    }

    // loops
    QList<QList<int> > edgesLoop;
    QList<QList<bool> > edgesReverse;
    for (int i = 0; i < m_loops.size(); i++)
    {
        edgesLoop.append(QList<int>());
        edgesReverse.append(QList<bool>());
        // qDebug() << "loop" << i;

        if (!m_outsideLoops.contains(i))
        {
            for (int j = 0; j < m_loops.at(i).size(); j++)
            {
                edgesLoop.last().append(m_loops.at(i)[j].edge);
                edgesReverse.last().append(m_loops.at(i)[j].reverse);
                // qDebug() << "edge" << m_loops.at(i)[j].edge << " reverse " << m_loops.at(i)[j].reverse;
            }
        }
    }

    // faces
    if (m_loops.size() > 0)
    {
        // qDebug() << "edges loop size" << edgesLoop.size();
        for (int labelIdx = 0; labelIdx < m_scene->labels->count(); labelIdx++)
        {
            SceneLabel* label = m_scene->labels->at(labelIdx);
            // qDebug() << "label" << labelIdx;

            if (!label->isHole())
            {
                for (int j = 0; j < m_labelLoops[label].count(); j++)
                {
                    int loopIdx = m_labelLoops[label][j];
                    bool isInside = labelsInsideLoop[loopIdx].contains(label);
                    // qDebug() << loopIdx << "inside" << isInside << "hole" << label->isHole();

                    for (int k = 0; k < edgesLoop[loopIdx].size(); k++)
                    {
                        // qDebug() <<  edgesLoop[loopIdx][k];

                        SceneFace *edge = m_scene->faces->at(edgesLoop[loopIdx][k]);

                        int index = label->isHole() ? MARKER_IDX_NOT_EXISTING : labelIdx;
                        bool reverse = edgesReverse[loopIdx][k];

                        if (isInside)
                        {
                            if (!reverse)
                                edge->setLeftLabelIdx(index);
                            else
                                edge->setRightLabelIdx(index);
                        }
                        else
                        {
                            if (!reverse)
                                edge->setRightLabelIdx(index);
                            else
                                edge->setLeftLabelIdx(index);
                        }
                    }
                }
            }
        }
    }
}

void LoopsInfo::processPolygons()
{
    try
    {
        {
#ifdef DEAL_II_WITH_TBB
            tbb::mutex::scoped_lock lock(processMutex);
#endif
            clear();

            // find loops
            try
            {
                processLoops();

                QList<QList<Point> > polylines;
                for (auto & loop : m_loops)
                {
                    QList<Point> polyline;

                    // QList<Point> contour;
                    for (auto & innerLoop : loop)
                    {
                        SceneFace *edge = m_scene->faces->items().at(innerLoop.edge);
                        if ((edge->nodeStart()->numberOfConnectedEdges() > 0) && (edge->nodeEnd()->numberOfConnectedEdges() > 0))
                        {
                            if (innerLoop.reverse)
                                addEdgePoints(&polyline, SceneFace(edge->scene(), edge->nodeStart(), edge->nodeEnd(), Value(nullptr, edge->angle())), true);
                            else
                                addEdgePoints(&polyline, SceneFace(edge->scene(), edge->nodeStart(), edge->nodeEnd(), Value(nullptr, edge->angle())));
                        }
                    }

                    polylines.append(polyline);
                }

                foreach (SceneLabel *label, m_scene->labels->items())
                {
                    // if (!label->isHole() && loopsInfo.labelToLoops[label].count() > 0)
                    if (m_labelLoops[label].count() > 0)
                    {
                        // main polyline
                        QList<Point> polyline = polylines[m_labelLoops[label][0]];

                        // holes
                        QList<QList<Point> > holes;
                        for (int j = 1; j < m_labelLoops[label].count(); j++)
                        {
                            QList<Point> hole = polylines[m_labelLoops[label][j]];
                            holes.append(hole);
                        }

                        if (!polyline.isEmpty())
                        {
                            // QList<Triangle> triangles = triangulateLabel(polyline, holes, label->point());
                        }
                    }
                }

                // clear polylines
                foreach (QList<Point> polyline, polylines)
                    polyline.clear();
                polylines.clear();

                m_isProcessPolygonError = false;
            }
            catch (AgrosGeometryException &e)
            {
                // do nothing
                m_isProcessPolygonError = true;
            }
        }
    }
    catch (...)
    {
        // DO NOTHING
    }
}

void LoopsInfo::clear()
{
    m_loops.clear();
    m_labelLoops.clear();
    m_outsideLoops.clear();
}

