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

#include "meshgenerator_triangle.h"

#include "util/global.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "solver/problem.h"
#include "solver/problem_config.h"
#include "logview.h"

#include <QThread>

MeshGeneratorTriangleExternal::MeshGeneratorTriangleExternal(ProblemBase *problem)
   : MeshGenerator(problem)
{
}

bool MeshGeneratorTriangleExternal::mesh()
{
    // create triangle files
    if (prepare() && writeToTriangle())
    {
        // exec triangle
       QSharedPointer<QProcess> process = QSharedPointer<QProcess>(new QProcess());
       process->setStandardOutputFile(tempProblemFileName() + ".triangle.out");
       process->setStandardErrorFile(tempProblemFileName() + ".triangle.err");

       QString triangleBinary = "triangle";
       if (QFile::exists(QCoreApplication::applicationDirPath() + "/" + "triangle.exe"))
           triangleBinary = QCoreApplication::applicationDirPath() + "/" + "triangle.exe";
       if (QFile::exists(QCoreApplication::applicationDirPath() + QDir::separator() + "triangle"))
           triangleBinary = QCoreApplication::applicationDirPath() + QDir::separator() + "triangle";

       QString command = QString("%1").arg(triangleBinary);
       QStringList args;
       args.append("-pPq31.0eAazQIno2");
       args.append(QString("%2").arg(tempProblemFileName()));

        // Windows - could you try?
        args.append(QString("%2").arg(tempProblemFileName()));

       process->start(command, args, QIODeviceBase::ReadOnly);

       if (!process->waitForStarted())
       {
           Agros::log()->printError(tr("Mesh Generator"), tr("Could not start Triangle"));
           process->kill();
           process->close();

           return false;
       }

       if (process->waitForFinished(-1))
       {
           if ((process->exitCode() == 0) && readTriangleMeshFormat())
           {
               //  remove triangle temp files
               QFile::remove(tempProblemFileName() + ".poly");
               QFile::remove(tempProblemFileName() + ".node");
               QFile::remove(tempProblemFileName() + ".edge");
               QFile::remove(tempProblemFileName() + ".ele");
               QFile::remove(tempProblemFileName() + ".neigh");
               QFile::remove(tempProblemFileName() + ".triangle.out");
               QFile::remove(tempProblemFileName() + ".triangle.err");

               return true;
           }
           else
           {
               QString errorMessage = readFileContent(tempProblemFileName() + ".triangle.err");
               errorMessage.insert(0, "\n");
               errorMessage.append("\n");
               Agros::log()->printError(tr("Mesh Generator"), errorMessage);

               return false;
           }
       }
   }

   return false;
}

bool MeshGeneratorTriangleExternal::writeToTriangle()
{
   // basic check
   if (m_problem->scene()->nodes->length() < 3)
   {
       Agros::log()->printError(tr("Mesh Generator"), tr("Invalid number of nodes (%1 < 3)").arg(m_problem->scene()->nodes->length()));
       return false;
   }
   if (m_problem->scene()->faces->length() < 3)
   {
       Agros::log()->printError(tr("Mesh Generator"), tr("Invalid number of edges (%1 < 3)").arg(m_problem->scene()->faces->length()));
       return false;
   }

   // save current locale
   // char *plocale = setlocale (LC_NUMERIC, "");
   // setlocale (LC_NUMERIC, "C");

   QFile file(tempProblemFileName() + ".poly");

   if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
   {
       Agros::log()->printError(tr("Mesh Generator"), tr("Could not create Triangle poly mesh file (%1)").arg(file.errorString()));
       return false;
   }
   QTextStream out(&file);


   // nodes
   QString outNodes;
   int nodesCount = 0;
   for (int i = 0; i < m_problem->scene()->nodes->length(); i++)
   {
       outNodes += QString("%1  %2  %3  %4\n").
               arg(i).
               arg(m_problem->scene()->nodes->at(i)->point().x, 0, 'f', 10).
               arg(m_problem->scene()->nodes->at(i)->point().y, 0, 'f', 10).
               arg(0);
       nodesCount++;
   }

   // edges
   QString outEdges;
   int edgesCount = 0;
   for (int i = 0; i < m_problem->scene()->faces->length(); i++)
   {
       if (fabs(m_problem->scene()->faces->at(i)->angle()) < EPS_ZERO)
       {
           // line
           outEdges += QString("%1  %2  %3  %4\n").
                   arg(edgesCount).
                   arg(m_problem->scene()->nodes->items().indexOf(m_problem->scene()->faces->at(i)->nodeStart())).
                   arg(m_problem->scene()->nodes->items().indexOf(m_problem->scene()->faces->at(i)->nodeEnd())).
                   arg(i+1);
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

           int segments = m_problem->scene()->faces->at(i)->segments();
           double theta = deg2rad(m_problem->scene()->faces->at(i)->angle()) / double(segments);

           int nodeStartIndex = 0;
           int nodeEndIndex = 0;
           for (int j = 0; j < segments; j++)
           {
               double arc = startAngle + j*theta;

               double x = radius * cos(arc);
               double y = radius * sin(arc);

               nodeEndIndex = nodesCount+1;
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
                   outNodes += QString("%1  %2  %3  %4\n").
                           arg(nodesCount).
                           arg(center.x + x, 0, 'f', 10).
                           arg(center.y + y, 0, 'f', 10).
                           arg(0);
                   nodesCount++;
               }
               outEdges += QString("%1  %2  %3  %4\n").
                       arg(edgesCount).
                       arg(nodeStartIndex).
                       arg(nodeEndIndex).
                       arg(i+1);
               edgesCount++;
               nodeStartIndex = nodeEndIndex;
           }
       }
   }

   // holes
   int holesCount = 0;
   foreach (SceneLabel *label, m_problem->scene()->labels->items())
       if (label->markersCount() == 0)
           holesCount++;

   QString outHoles = QString("%1\n").arg(holesCount);
   holesCount = 0;
   foreach (SceneLabel *label, m_problem->scene()->labels->items())
   {
       if (label->markersCount() == 0)
       {
           outHoles += QString("%1  %2  %3\n").
                   arg(holesCount).
                   // arg(Agros::problem()->scene()->labels->items().indexOf(label) + 1).
                   arg(label->point().x, 0, 'f', 10).
                   arg(label->point().y, 0, 'f', 10);

           holesCount++;
       }
   }

   // labels
   QString outLabels;
   int labelsCount = 0;
   foreach (SceneLabel *label, m_problem->scene()->labels->items())
   {
       if (label->markersCount() > 0)
       {
           outLabels += QString("%1  %2  %3  %4  %5\n").
                   arg(labelsCount).
                   arg(label->point().x, 0, 'f', 10).
                   arg(label->point().y, 0, 'f', 10).
                   // arg(labelsCount + 1). // triangle returns zero region number for areas without marker, markers must start from 1
                   arg(m_problem->scene()->labels->items().indexOf(label) + 1).
                   arg(label->area());
           labelsCount++;
       }
   }

   outNodes.insert(0, QString("%1 2 0 1\n").
                   arg(nodesCount)); // + additional Agros::problem()->scene()->nodes
   out << outNodes;
   outEdges.insert(0, QString("%1 1\n").
                   arg(edgesCount)); // + additional edges
   out << outEdges;
   out << outHoles;
   outLabels.insert(0, QString("%1 1\n").
                    arg(labelsCount)); // - holes
   out << outLabels;

   file.waitForBytesWritten(0);
   file.close();

   return true;
}

bool MeshGeneratorTriangleExternal::readTriangleMeshFormat()
{
   QFile fileNode(tempProblemFileName() + ".node");
   if (!fileNode.open(QIODevice::ReadOnly | QIODevice::Text))
   {
       Agros::log()->printError(tr("Mesh Generator"), tr("Could not read Triangle node file"));
       return false;
   }
   QTextStream inNode(&fileNode);

   QFile fileEdge(tempProblemFileName() + ".edge");
   if (!fileEdge.open(QIODevice::ReadOnly | QIODevice::Text))
   {
       Agros::log()->printError(tr("Mesh Generator"), tr("Could not read Triangle edge file"));
       return false;
   }
   QTextStream inEdge(&fileEdge);

   QFile fileEle(tempProblemFileName() + ".ele");
   if (!fileEle.open(QIODevice::ReadOnly | QIODevice::Text))
   {
       Agros::log()->printError(tr("Mesh Generator"), tr("Could not read Triangle elements file"));
       return false;
   }
   QTextStream inEle(&fileEle);

   QFile fileNeigh(tempProblemFileName() + ".neigh");
   if (!fileNeigh.open(QIODevice::ReadOnly | QIODevice::Text))
   {
       Agros::log()->printError(tr("Mesh Generator"), tr("Could not read Triangle neighbors elements file"));
       return false;
   }
   QTextStream inNeigh(&fileNeigh);

   // white chars
   QRegularExpression whiteChar("\\s+");

   // triangle nodes
   QString lineNode = inNode.readLine().trimmed();
   int numberOfNodes = lineNode.split(whiteChar).at(0).toInt();
   for (int i = 0; i < numberOfNodes; i++)
   {
       // suspisious code, causes the "Concave element ...." exception
       QStringList parsedLine = inNode.readLine().trimmed().split(whiteChar);

       nodeList.append(Point(parsedLine.at(1).toDouble(),
                             parsedLine.at(2).toDouble()));
   }

   // triangle edges
   QString lineEdge = inEdge.readLine().trimmed();
   int numberOfEdges = lineEdge.split(whiteChar).at(0).toInt();

   // for curvature
   std::map<std::pair<int, int>, Point> centers;
   std::map<std::pair<int, int>, double> sizes;
   for (int i = 0; i < numberOfEdges; i++)
   {
       QStringList parsedLine = inEdge.readLine().trimmed().split(whiteChar);

       // marker conversion from triangle, where it starts from 1
       edgeList.append(MeshEdge(parsedLine.at(1).toInt(),
                                parsedLine.at(2).toInt(),
                                parsedLine.at(3).toInt() - 1));

       if (parsedLine.at(3).toInt() > 0)
       {
           SceneFace* sceneEdge = m_problem->scene()->faces->at(parsedLine.at(3).toInt() - 1);

           if (sceneEdge->angle() > 0.0)
           {
               int node_indices[2] = { parsedLine.at(1).toInt(), parsedLine.at(2).toInt() };

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
   QString lineElement = inEle.readLine().trimmed();
   int numberOfElements = lineElement.split(whiteChar).at(0).toInt();
   QSet<int> labelMarkersCheck;
   for (int i = 0; i < numberOfElements; i++)
   {
       QStringList parsedLine = inEle.readLine().trimmed().split(whiteChar);
       if (parsedLine.count() == 7)
       {
           Agros::log()->printError(tr("Mesh Generator"), tr("Some areas do not have a marker"));
           return false;
       }
       int marker = parsedLine.at(7).toInt();

       if (marker == 0)
       {
           Agros::log()->printError(tr("Mesh Generator"), tr("Some areas do not have a marker"));
           return false;
       }

       // vertices
       int nodeA = parsedLine.at(1).toInt();
       int nodeB = parsedLine.at(2).toInt();
       int nodeC = parsedLine.at(3).toInt();
       // 2nd order nodes (in the middle of edges)
       int nodeNA = parsedLine.at(4).toInt();
       int nodeNB = parsedLine.at(5).toInt();
       int nodeNC = parsedLine.at(6).toInt();

       // handle curvature
       int endpoints_to_midpoints[6][3] = {
           { nodeA, nodeB, nodeNC },
           { nodeB, nodeA, nodeNC },
           { nodeC, nodeB, nodeNA },
           { nodeB, nodeC, nodeNA },
           { nodeA, nodeC, nodeNB },
           { nodeC, nodeA, nodeNB }
       };

       for (int test_i = 0; test_i < 6; test_i++)
       {
           if (centers.find(std::pair<int, int>(endpoints_to_midpoints[test_i][0], endpoints_to_midpoints[test_i][1])) != centers.end())
           {
               Point p = nodeList[endpoints_to_midpoints[test_i][2]];
               Point c = centers.find(std::pair<int, int>(endpoints_to_midpoints[test_i][0], endpoints_to_midpoints[test_i][1]))->second;
               double r = sizes.find(std::pair<int, int>(endpoints_to_midpoints[test_i][0], endpoints_to_midpoints[test_i][1]))->second;
               nodeList[endpoints_to_midpoints[test_i][2]] = prolong_point_to_arc(p, c, r);
           }
       }

       if (Agros::problem()->config()->meshType() == MeshType_Triangle_QuadFineDivision)
       {
           // add additional node - centroid
           nodeList.append(Point((nodeList[nodeA].x + nodeList[nodeB].x + nodeList[nodeC].x) / 3.0,
                                 (nodeList[nodeA].y + nodeList[nodeB].y + nodeList[nodeC].y) / 3.0));

           // add three quad elements
           elementList.append(MeshElement(nodeNB, nodeA, nodeNC, nodeList.count() - 1, marker - 1)); // marker conversion from triangle, where it starts from 1
           elementList.append(MeshElement(nodeNC, nodeB, nodeNA, nodeList.count() - 1, marker - 1)); // marker conversion from triangle, where it starts from 1
           elementList.append(MeshElement(nodeNA, nodeC, nodeNB, nodeList.count() - 1, marker - 1)); // marker conversion from triangle, where it starts from 1
       }

       labelMarkersCheck.insert(marker - 1);
   }
   // update number of elements
   numberOfElements = elementList.count();

   // triangle neigh
   QString lineNeigh = inNeigh.readLine().trimmed();
   int numberOfNeigh = lineNeigh.split(whiteChar).at(0).toInt();
   for (int i = 0; i < numberOfNeigh; i++)
   {
       QStringList parsedLine = inNeigh.readLine().trimmed().split(whiteChar);

       elementList[i].neigh[0] = parsedLine.at(1).toInt();
       elementList[i].neigh[1] = parsedLine.at(2).toInt();
       elementList[i].neigh[2] = parsedLine.at(3).toInt();
   }

   fileNode.close();
   fileEdge.close();
   fileEle.close();
   fileNeigh.close();

   // heterogeneous mesh
   // element division
   if (Agros::problem()->config()->meshType() == MeshType_Triangle_QuadFineDivision)
   {
       for (int i = 0; i < edgeCountLinear; i++)
       {
           if (edgeList[i].marker != -1)
           {
               for (int j = 0; j < elementList.count() / 3; j++)
               {
                   for (int k = 0; k < 3; k++)
                   {
                       if (edgeList[i].node[0] == elementList[3*j + k].node[1] && edgeList[i].node[1] == elementList[3*j + (k + 1) % 3].node[1])
                       {
                           edgeList.append(MeshEdge(edgeList[i].node[0], elementList[3*j + (k + 1) % 3].node[0], edgeList[i].marker));
                           edgeList[i].node[0] = elementList[3*j + (k + 1) % 3].node[0];
                       }
                   }
               }
           }
       }
   }

   fillNeighborStructures();
   // moveNodesOnCurvedEdges();

   writeTodealii();

   nodeList.clear();
   edgeList.clear();
   elementList.clear();

   return true;
}

// ****************************************************************************

MeshGeneratorTriangle::MeshGeneratorTriangle(ProblemBase *problem)
    : MeshGenerator(problem)
{
}


bool MeshGeneratorTriangle::mesh()
{
    Agros::log()->printMessage(tr("Mesh Generator"), tr("Triangle"));

    // create triangle files
    if (prepare() && writeToTriangle())
    {
        // Agros::log()->printDebug(tr("Mesh Generator"), tr("Mesh files were created"));

        // convert triangle mesh to hermes mesh
        if (readTriangleMeshFormat())
            return true;
    }

    return false;
}

bool MeshGeneratorTriangle::writeToTriangle()
{
    // basic check
    if (m_problem->scene()->nodes->length() < 3)
    {
        Agros::log()->printError(tr("Mesh Generator"), tr("Invalid number of nodes (%1 < 3)").arg(m_problem->scene()->nodes->length()));
        return false;
    }
    if (m_problem->scene()->faces->length() < 3)
    {
        Agros::log()->printError(tr("Mesh Generator"), tr("Invalid number of edges (%1 < 3)").arg(m_problem->scene()->faces->length()));
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

            // TODO: REMOVE MULTIPLICATION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            int segments = m_problem->scene()->faces->at(i)->segments() * 1; // TODO: REMOVE MULTIPLICATION
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

    // set mesh quality angle
    
    #ifdef _MSC_VER  
        char const *triSwitches = "pq31.0eAazInQo2";
    #else
        char const* triSwitches = QString("pq%1.0eAazInQo2").arg(Agros::problem()->config()->value(ProblemConfig::MeshQualityAngle).toInt()).toStdString().c_str();
    #endif
    triangulate((char*)triSwitches, &triIn, &triOut, (struct triangulateio*)NULL);
    free(triIn.pointlist);
    free(triIn.pointmarkerlist);
    free(triIn.segmentlist);
    free(triIn.segmentmarkerlist);
    free(triIn.regionlist);
    free(triIn.holelist);

    return true;
}

bool MeshGeneratorTriangle::readTriangleMeshFormat()
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
        if (Agros::problem()->config()->meshType() == MeshType_Triangle_QuadFineDivision)
        {
            edgeList.append(MeshEdge(triOut.edgelist[2 * i], triOut.edgelist[2 * i + 1], triOut.edgemarkerlist[i] - 1));
        }

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
            Agros::log()->printError(tr("Mesh Generator"), tr("Some areas in mesh do not have a marker"));
            return false;
        }

        // vertices
        int nodeA = triOut.trianglelist[6 * i];
        int nodeB = triOut.trianglelist[6 * i + 1];
        int nodeC = triOut.trianglelist[6 * i + 2];
        // 2nd order nodes (in the middle of edges)
        int nodeNA = triOut.trianglelist[6 * i + 3];
        int nodeNB = triOut.trianglelist[6 * i + 4];
        int nodeNC = triOut.trianglelist[6 * i + 5];

        // handle curvature
        int endpoints_to_midpoints[6][3] = {
            { nodeA, nodeB, nodeNC },
            { nodeB, nodeA, nodeNC },
            { nodeC, nodeB, nodeNA },
            { nodeB, nodeC, nodeNA },
            { nodeA, nodeC, nodeNB },
            { nodeC, nodeA, nodeNB }
        };

        for (int test_i = 0; test_i < 6; test_i++)
        {
            if (centers.find(std::pair<int, int>(endpoints_to_midpoints[test_i][0], endpoints_to_midpoints[test_i][1])) != centers.end())
            {
                Point p = nodeList[endpoints_to_midpoints[test_i][2]];
                Point c = centers.find(std::pair<int, int>(endpoints_to_midpoints[test_i][0], endpoints_to_midpoints[test_i][1]))->second;
                double r = sizes.find(std::pair<int, int>(endpoints_to_midpoints[test_i][0], endpoints_to_midpoints[test_i][1]))->second;
                nodeList[endpoints_to_midpoints[test_i][2]] = prolong_point_to_arc(p, c, r);
            }
        }

        // if (Agros::problem()->config()->meshType() == MeshType_Triangle_QuadJoin ||
        //         Agros::problem()->config()->meshType() == MeshType_Triangle_QuadRoughDivision)
        // {
        //     elementList.append(MeshElement(nodeA, nodeB, nodeC, marker - 1)); // marker conversion from triangle, where it starts from 1
        // }

        if (Agros::problem()->config()->meshType() == MeshType_Triangle_QuadFineDivision)
        {
            // add additional node - incenter
            double a = (nodeList[nodeB] - nodeList[nodeC]).magnitude();
            double b = (nodeList[nodeC] - nodeList[nodeA]).magnitude();
            double c = (nodeList[nodeA] - nodeList[nodeB]).magnitude();

            nodeList.append(Point((a * nodeList[nodeA].x + b * nodeList[nodeB].x + c * nodeList[nodeC].x) / (a + b + c),
                                  (a * nodeList[nodeA].y + b * nodeList[nodeB].y + c * nodeList[nodeC].y) / (a + b + c)));

            // add three quad elements
            elementList.append(MeshElement(nodeNB, nodeA, nodeNC, nodeList.count() - 1, marker - 1)); // marker conversion from triangle, where it starts from 1
            elementList.append(MeshElement(nodeNC, nodeB, nodeNA, nodeList.count() - 1, marker - 1)); // marker conversion from triangle, where it starts from 1
            elementList.append(MeshElement(nodeNA, nodeC, nodeNB, nodeList.count() - 1, marker - 1)); // marker conversion from triangle, where it starts from 1
        }
    }

    // triangle neigh
    for (int i = 0; i < triOut.numberoftriangles; i++)
    {
        elementList[i].neigh[0] = triOut.neighborlist[3*i];
        elementList[i].neigh[1] = triOut.neighborlist[3*i+1];
        elementList[i].neigh[2] = triOut.neighborlist[3*i+2];
    }

    // heterogeneous mesh
    // element division
    if (Agros::problem()->config()->meshType() == MeshType_Triangle_QuadFineDivision)
    {
        for (int i = 0; i < edgeCountLinear; i++)
        {
            if (edgeList[i].marker != -1)
            {
                for (int j = 0; j < elementList.count() / 3; j++)
                {
                    for (int k = 0; k < 3; k++)
                    {
                        if (edgeList[i].node[0] == elementList[3*j + k].node[1] && edgeList[i].node[1] == elementList[3*j + (k + 1) % 3].node[1])
                        {
                            edgeList.append(MeshEdge(edgeList[i].node[0], elementList[3*j + (k + 1) % 3].node[0], edgeList[i].marker));
                            edgeList[i].node[0] = elementList[3*j + (k + 1) % 3].node[0];
                        }
                    }
                }
            }
        }
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

    fillNeighborStructures();

    writeTodealii();

    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    return true;
}
