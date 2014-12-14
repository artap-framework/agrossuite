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

#include "meshgenerator_cubit.h"

#include "util/global.h"

#include "scene.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "sceneview_common.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "logview.h"

#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "util/loops.h"

#include <QThread>

MeshGeneratorCubitExternal::MeshGeneratorCubitExternal()
    : MeshGenerator()
{
}

bool MeshGeneratorCubitExternal::mesh()
{
    m_isError = !prepare();

    // create triangle files
    if (writeToCubit())
    {
        // exec cubit
        m_process = QSharedPointer<QProcess>(new QProcess());
        m_process.data()->setStandardOutputFile(tempProblemFileName() + ".cubit.out");
        m_process.data()->setStandardErrorFile(tempProblemFileName() + ".cubit.err");
        connect(m_process.data(), SIGNAL(error(QProcess::ProcessError)), this, SLOT(meshCubitError(QProcess::ProcessError)));
        connect(m_process.data(), SIGNAL(finished(int)), this, SLOT(meshCubitCreated(int)));

        QString cubitBinary = "cubit";
        if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "triangle.exe"))
            cubitBinary = "\"" + QApplication::applicationDirPath() + QDir::separator() + "triangle.exe\"";
        if (QFile::exists(QApplication::applicationDirPath() + QDir::separator() + "triangle"))
            cubitBinary = QApplication::applicationDirPath() + QDir::separator() + "triangle";

        QString cubitCommand = QString("%1 -nographics -nojournal -batch -input \"%2.jou\"").
                arg(cubitBinary).
                arg(tempProblemFileName());

        qDebug() << cubitCommand;
        m_process.data()->start(cubitCommand, QIODevice::ReadOnly);

        // execute an event loop to process the request (nearly-synchronous)
        QEventLoop eventLoop;
        connect(m_process.data(), SIGNAL(finished(int)), &eventLoop, SLOT(quit()));
        connect(m_process.data(), SIGNAL(error(QProcess::ProcessError)), &eventLoop, SLOT(quit()));
        eventLoop.exec();
    }
    else
    {
        m_isError = true;
    }

    return !m_isError;
}

void MeshGeneratorCubitExternal::meshCubitError(QProcess::ProcessError error)
{
    m_isError = true;
    Agros2D::log()->printError(tr("Mesh generator"), tr("Could not start Triangle"));
    m_process.data()->kill();
    m_process.data()->close();
}

void MeshGeneratorCubitExternal::meshCubitCreated(int exitCode)
{
    if (exitCode == 0)
    {
        // Agros2D::log()->printDebug(tr("Mesh generator"), tr("Mesh files were created"));

        // convert triangle mesh to L mesh
        if (readLSDynaMeshFormat())
        {
            // Agros2D::log()->printDebug(tr("Mesh generator"), tr("Mesh was converted to deai.II mesh file"));

            //  remove triangle temp files
            // QFile::remove(tempProblemFileName() + ".jou");
            // QFile::remove(tempProblemFileName() + ".k");
            // QFile::remove(tempProblemFileName() + ".cubit.out");
            // QFile::remove(tempProblemFileName() + ".cubit.err");
        }
        else
        {
            m_isError = true;
            QFile::remove(Agros2D::problem()->config()->fileName() + ".msh");
        }
    }
    else
    {
        m_isError = true;
        QString errorMessage = readFileContent(tempProblemFileName() + ".triangle.err");
        errorMessage.insert(0, "\n");
        errorMessage.append("\n");
        Agros2D::log()->printError(tr("Mesh generator"), errorMessage);
    }

    m_process.data()->close();
}

bool MeshGeneratorCubitExternal::writeToCubit()
{
    // basic check
    if (Agros2D::scene()->nodes->length() < 3)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Invalid number of nodes (%1 < 3)").arg(Agros2D::scene()->nodes->length()));
        return false;
    }
    if (Agros2D::scene()->edges->length() < 3)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Invalid number of edges (%1 < 3)").arg(Agros2D::scene()->edges->length()));
        return false;
    }

    QFile file(tempProblemFileName() + ".jou");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Could not create CUBIT journal file (%1)").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    QString outCommands;

    // mesh size
    RectPoint rect = Agros2D::scene()->boundingBox();
    double charEdge = 0; // qMax(rect.width(), rect.height()) / 2.0;

    foreach (SceneEdge *edge, Agros2D::scene()->edges->items())
        if (edge->length() > charEdge)
            charEdge = edge->length();

    // nodes
    QString outNodes;
    int nodesCount = 0;
    for (int i = 0; i < Agros2D::scene()->nodes->length(); i++)
    {
        outNodes += QString("Create Vertex %1\t%2\t0.0\n").
                arg(Agros2D::scene()->nodes->at(i)->point().x, 0, 'f', 10).
                arg(Agros2D::scene()->nodes->at(i)->point().y, 0, 'f', 10);
        nodesCount++;
    }

    // edges
    QString outEdges;
    int edgesCount = 0;
    for (int i = 0; i<Agros2D::scene()->edges->length(); i++)
    {
        if (Agros2D::scene()->edges->at(i)->isStraight())
        {
            // line .. increase edge index to count from 1
            outEdges += QString("Create Curve %1 %2 # straight\n").
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeStart()) + 1).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeEnd()) + 1);
        }
        else
        {
            // arc
            // add pseudo nodes
            Point center = Agros2D::scene()->edges->at(i)->center();
            outNodes += QString("Create Vertex %1\t%2\t0.0\n").
                    arg(center.x, 0, 'f', 10).
                    arg(center.y, 0, 'f', 10);
            nodesCount++;

            outEdges += QString("Create Curve Arc Center Vertex %1 %2 %3 # curved\n").
                    arg(nodesCount).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeStart()) + 1).
                    arg(Agros2D::scene()->nodes->items().indexOf(Agros2D::scene()->edges->at(i)->nodeEnd()) + 1);

            // arg(nodesCount - 1).
        }
        // outEdges += QString("Nodeset %1 Curve %1\n").arg(edgesCount + 1);
        edgesCount++;
    }

    try
    {
        Agros2D::scene()->loopsInfo()->processLoops();
    }
    catch (AgrosMeshException& ame)
    {
        Agros2D::log()->printError(tr("Mesh generator"), ame.toString());
        std::cout << "Missing Label";
        return false;
    }

    QString outLoops;
    QString outMesh;

    int loopsCount = 1;
    for (int i = 0; i < Agros2D::scene()->loopsInfo()->loops().size(); i++)
    {
        if (!Agros2D::scene()->loopsInfo()->outsideLoops().contains(i))
        {
            outLoops.append(QString("Create Surface Curve "));
            for(int j = 0; j < Agros2D::scene()->loopsInfo()->loops().at(i).size(); j++)
            {
                outLoops.append(QString("%1").arg(Agros2D::scene()->loopsInfo()->loops().at(i)[j].edge + 1));
                if (j < Agros2D::scene()->loopsInfo()->loops().at(i).size() - 1)
                    outLoops.append(" ");
            }
            outLoops.append(QString("\n"));
            // mesh surface
            // outMesh.append(QString("Mesh Surface %1\n").arg(loopsCount));
            loopsCount++;
        }
    }
    outLoops.append("\n");

    int lastSurface = loopsCount;
    for (int i = 0; i < Agros2D::scene()->labels->count(); i++)
    {
        SceneLabel* label = Agros2D::scene()->labels->at(i);

        if(!label->isHole())
        {
            // subtract holes from first loop (main loop)
            if (Agros2D::scene()->loopsInfo()->labelLoops()[label].count() > 1)
            {
                QString holes;
                for (int j = 1; j < Agros2D::scene()->loopsInfo()->labelLoops()[label].count(); j++)
                {
                    holes.append(QString("%1").arg(Agros2D::scene()->loopsInfo()->labelLoops()[label][j]));
                    if (j < Agros2D::scene()->loopsInfo()->labelLoops()[label].count() - 1)
                        holes.append(" ");
                }
                outCommands.append(QString("Subtract Body %2 From Body %1 Imprint\n").
                                   arg(Agros2D::scene()->loopsInfo()->labelLoops()[label][0]).
                                   arg(holes));
                // outCommands.append(QString("Surface %1 Id %2\n").arg(lastSurface).arg(Agros2D::scene()->loopsInfo()->labelLoops()[label][0]));
                // outCommands.append(QString("Compress Surface\n"));
                lastSurface++;
            }
        }
    }

    // output
    out << "Reset\n";

    outNodes.insert(0, QString("\n# nodes\n"));
    out << outNodes;
    outEdges.insert(0, QString("\n# edges\n"));
    out << outEdges;
    outLoops.insert(0, QString("\n# loops\n"));
    out << outLoops;
    outCommands.insert(0, QString("\n# commands\n"));
    out << outCommands;
    outMesh.insert(0, QString("\n# mesh\n"));
    out << outMesh;

    out << QString("\nExport lsdyna \"%1.k\" overwrite\n").arg(tempProblemFileName());

    file.waitForBytesWritten(0);
    file.close();

    return true;
}

bool MeshGeneratorCubitExternal::readLSDynaMeshFormat()
{
    QFile fileInput(tempProblemFileName() + ".k");
    if (!fileInput.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Could not read LSDyna node file"));
        return false;
    }
    QTextStream inNode(&fileInput);

    QFile fileEdge(tempProblemFileName() + ".edge");
    if (!fileEdge.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Could not read Triangle edge file"));
        return false;
    }
    QTextStream inEdge(&fileEdge);

    QFile fileEle(tempProblemFileName() + ".ele");
    if (!fileEle.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Could not read Triangle elements file"));
        return false;
    }
    QTextStream inEle(&fileEle);

    QFile fileNeigh(tempProblemFileName() + ".neigh");
    if (!fileNeigh.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Could not read Triangle neighbors elements file"));
        return false;
    }
    QTextStream inNeigh(&fileNeigh);

    // white chars
    QRegExp whiteChar("\\s+");

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
    for (int i = 0; i < numberOfEdges; i++)
    {
        QStringList parsedLine = inEdge.readLine().trimmed().split(whiteChar);

        // marker conversion from triangle, where it starts from 1
        edgeList.append(MeshEdge(parsedLine.at(1).toInt(),
                                 parsedLine.at(2).toInt(),
                                 parsedLine.at(3).toInt() - 1));
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
            Agros2D::log()->printError(tr("Mesh generator"), tr("Some areas do not have a marker"));
            return false;
        }
        int marker = parsedLine.at(7).toInt();

        if (marker == 0)
        {
            Agros2D::log()->printError(tr("Mesh generator"), tr("Some areas do not have a marker"));
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

        /*
        if (Agros2D::problem()->config()->meshType() == MeshType_Triangle ||
                Agros2D::problem()->config()->meshType() == MeshType_Triangle_QuadJoin ||
                Agros2D::problem()->config()->meshType() == MeshType_Triangle_QuadRoughDivision)
        {
            elementList.append(MeshElement(nodeA, nodeB, nodeC, marker - 1)); // marker conversion from triangle, where it starts from 1
        }
        */
        // if (Agros2D::problem()->config()->meshType() == MeshType_Triangle_QuadFineDivision)
        {
            // add additional node
            nodeList.append(Point((nodeList[nodeA].x + nodeList[nodeB].x + nodeList[nodeC].x) / 3.0,
                                  (nodeList[nodeA].y + nodeList[nodeB].y + nodeList[nodeC].y) / 3.0));
            // add three quad elements
            elementList.append(MeshElement(nodeNB, nodeA, nodeNC, nodeList.count() - 1, marker - 1)); // marker conversion from triangle, where it starts from 1
            elementList.append(MeshElement(nodeNC, nodeB, nodeNA, nodeList.count() - 1, marker - 1)); // marker conversion from triangle, where it starts from 1
            elementList.append(MeshElement(nodeNA, nodeC, nodeNB, nodeList.count() - 1, marker - 1)); // marker conversion from triangle, where it starts from 1
        }

        labelMarkersCheck.insert(marker - 1);
    }
    int elementCountLinear = elementList.count();

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

    fileInput.close();
    fileEdge.close();
    fileEle.close();
    fileNeigh.close();

    // heterogeneous mesh
    // element division
    // if (Agros2D::problem()->config()->meshType() == MeshType_Triangle_QuadFineDivision)
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

