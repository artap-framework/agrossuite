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
    if (Agros2D::computation()->scene()->nodes->length() < 3)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Invalid number of nodes (%1 < 3)").arg(Agros2D::computation()->scene()->nodes->length()));
        return false;
    }
    if (Agros2D::computation()->scene()->edges->length() < 3)
    {
        Agros2D::log()->printError(tr("Mesh generator"), tr("Invalid number of edges (%1 < 3)").arg(Agros2D::computation()->scene()->edges->length()));
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
    RectPoint rect = Agros2D::computation()->scene()->boundingBox();
    double charEdge = 0; // qMax(rect.width(), rect.height()) / 2.0;

    foreach (SceneEdge *edge, Agros2D::computation()->scene()->edges->items())
        if (edge->length() > charEdge)
            charEdge = edge->length();

    // nodes
    QString outNodes;
    int nodesCount = 0;
    for (int i = 0; i < Agros2D::computation()->scene()->nodes->length(); i++)
    {
        outNodes += QString("Create Vertex %1\t%2\t0.0\n").
                arg(Agros2D::computation()->scene()->nodes->at(i)->point().x, 0, 'f', 10).
                arg(Agros2D::computation()->scene()->nodes->at(i)->point().y, 0, 'f', 10);
        nodesCount++;
    }

    // edges
    QString outEdges;
    int edgesCount = 0;
    double minEdge = numeric_limits<double>::max();
    for (int i = 0; i<Agros2D::computation()->scene()->edges->length(); i++)
    {
        if (Agros2D::computation()->scene()->edges->at(i)->isStraight())
        {
            // line .. increase edge index to count from 1
            outEdges += QString("Create Curve %1 %2 # straight\n").
                    arg(Agros2D::computation()->scene()->nodes->items().indexOf(Agros2D::computation()->scene()->edges->at(i)->nodeStart()) + 1).
                    arg(Agros2D::computation()->scene()->nodes->items().indexOf(Agros2D::computation()->scene()->edges->at(i)->nodeEnd()) + 1);
        }
        else
        {
            // arc
            // add pseudo nodes
            Point center = Agros2D::computation()->scene()->edges->at(i)->center();
            outNodes += QString("Create Vertex %1\t%2\t0.0\n").
                    arg(center.x, 0, 'f', 10).
                    arg(center.y, 0, 'f', 10);
            nodesCount++;

            outEdges += QString("Create Curve Arc Center Vertex %1 %2 %3 # curved\n").
                    arg(nodesCount).
                    arg(Agros2D::computation()->scene()->nodes->items().indexOf(Agros2D::computation()->scene()->edges->at(i)->nodeStart()) + 1).
                    arg(Agros2D::computation()->scene()->nodes->items().indexOf(Agros2D::computation()->scene()->edges->at(i)->nodeEnd()) + 1);

            // arg(nodesCount - 1).
        }

        if (Agros2D::computation()->scene()->edges->at(i)->length() < minEdge)
            minEdge = Agros2D::computation()->scene()->edges->at(i)->length();

        outEdges += QString("Nodeset %1 Curve %1\n").arg(edgesCount + 1);

        edgesCount++;
    }

    try
    {
        Agros2D::computation()->scene()->loopsInfo()->processLoops();
    }
    catch (AgrosMeshException& ame)
    {
        Agros2D::log()->printError(tr("Mesh generator"), ame.toString());
        std::cout << "Missing Label";
        return false;
    }

    // loops
    QMap<int, QString> outLoopsLines;

    for(int i = 0; i < Agros2D::computation()->scene()->loopsInfo()->loops().size(); i++)
    {
        if (!Agros2D::computation()->scene()->loopsInfo()->outsideLoops().contains(i))
        {
            for (int j = 0; j < Agros2D::computation()->scene()->loopsInfo()->loops().at(i).size(); j++)
            {
                // if (Agros2D::problem()->scene()->loopsInfo()->loops().at(i)[j].reverse)
                //     outLoops.append("-");
                outLoopsLines[i+1] += QString("%1 ").arg(Agros2D::computation()->scene()->loopsInfo()->loops().at(i)[j].edge + 1);
            }
        }
    }

    // faces
    QString outLoops;
    int surfaceCount = 0;
    for (int i = 0; i < Agros2D::computation()->scene()->labels->count(); i++)
    {
        SceneLabel* label = Agros2D::computation()->scene()->labels->at(i);
        if (!label->isHole())
        {
            outLoops.append(QString("Create Surface Curve "));
            for (int j = 0; j < Agros2D::computation()->scene()->loopsInfo()->labelLoops()[label].count(); j++)
            {
                // outLoops.append(QString("%1 ").arg(Agros2D::problem()->scene()->loopsInfo()->labelLoops()[label][j] + 1));
                outLoops.append(QString("%1 ").arg(outLoopsLines[Agros2D::computation()->scene()->loopsInfo()->labelLoops()[label][j] + 1]));
            }
            outLoops.append(QString("\n"));

            surfaceCount++;
            outLoops.append(QString("Volume %1 Size %2\n").arg(surfaceCount).arg(minEdge * 2));
        }
    }
    outLoops.append("\n");   

    // output
    out << "Reset\n";

    out << QString("\n# nodes\n");
    out << outNodes;
    out << QString("\n# edges\n");
    out << outEdges;
    out << QString("\n# loops\n");
    out << outLoops;
    out << QString("\n# commands\n");
    out << outCommands;
    out << QString("\n# mesh\n");
    out << QString("Merge All\n"); // All
    out << QString("Mesh Surface All\n\n");
    out << QString("Export lsdyna \"%1.k\" overwrite\n").arg(tempProblemFileName());

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
    QTextStream inLSDyna(&fileInput);

    // white chars
    QRegExp whiteChar("\\s+");

    QString line = "";
    while (!inLSDyna.atEnd())
    {
        // nodes
        if (line == "*NODE")
        {
            // read header
            inLSDyna.readLine();

            while (true)
            {
                line = inLSDyna.readLine().trimmed();
                if (line == "$")
                    break;

                QStringList parsedLine = line.split(whiteChar);

                int index = parsedLine.at(0).toInt() - 1;
                Point point(parsedLine.at(1).toDouble(), parsedLine.at(2).toDouble());

                // resize
                if (nodeList.count() < index + 1)
                {
                    for (int i = nodeList.size(); i < index + 1; i++)
                    {
                        nodeList.append(Point());
                    }
                }

                nodeList[index] = point;
            }
        }

        // elements
        if (line == "*ELEMENT_SHELL")
        {
            int marker = -1;

            while (true)
            {
                line = inLSDyna.readLine();

                if (line.startsWith(" "))
                {
                    QStringList parsedLine = line.trimmed().split(whiteChar);

                    int i1 = parsedLine.at(2).toInt() - 1;
                    int i2 = parsedLine.at(3).toInt() - 1;
                    int i3 = parsedLine.at(4).toInt() - 1;
                    int i4 = parsedLine.at(5).toInt() - 1;

                    if (marker == -1)
                    {
                        // find real marker
                        Point center = nodeList[i1] + nodeList[i2] + nodeList[i3] + nodeList[i4];
                        SceneLabel *label = SceneLabel::findClosestLabel(Agros2D::computation()->scene(), Point(center.x / 4, center.y / 4));
                        assert(label);

                        marker = Agros2D::computation()->scene()->labels->items().indexOf(label);
                    }

                    // fix orientation
                    double area = ((nodeList[i1].x*nodeList[i2].y - nodeList[i2].x*nodeList[i1].y)
                                   + (nodeList[i2].x*nodeList[i3].y - nodeList[i3].x*nodeList[i2].y)
                                   + (nodeList[i3].x*nodeList[i4].y - nodeList[i4].x*nodeList[i3].y)
                                   + (nodeList[i4].x*nodeList[i1].y - nodeList[i1].x*nodeList[i4].y)) / 2.0;

                    if (area > 0)
                        elementList.append(MeshElement(i1, i2, i3, i4, marker));
                    else
                        elementList.append(MeshElement(i4, i3, i2, i1, marker));
                }
                else
                {
                    break;
                }
            }

            continue;
        }

        // edges
        if (line == "*SET_NODE_LIST")
        {
            line = inLSDyna.readLine().trimmed();
            int marker = line.toInt() - 1;
            QList<int> nodes;

            while (true)
            {
                line = inLSDyna.readLine();

                if (line.startsWith(" "))
                {
                    QStringList parsedLine = line.trimmed().split(whiteChar);

                    foreach (QString num, parsedLine)
                        nodes.append(num.toInt() - 1);
                }
                else
                {
                    break;
                }
            }

            for (int i = 0; i < nodes.count() - 1; i++)
                edgeList.append(MeshEdge(nodes[i], nodes[i+1], marker));

            continue;
        }

        if (line == "*END")
        {
            break;
        }

        line = inLSDyna.readLine().trimmed();
    }

    qDebug() << nodeList.size() << edgeList.size() << elementList.size() << "ok";


    /*
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
    */

    // fillNeighborStructures();
    // moveNodesOnCurvedEdges();

    writeTodealii();

    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    return true;
}
