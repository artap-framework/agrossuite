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

#include "meshgenerator_gmsh.h"

#include "util/global.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "solver/problem.h"
#include "solver/problem_config.h"
#include "util/loops.h"

#include "logview.h"


MeshGeneratorGMSH::MeshGeneratorGMSH(ProblemBase *problem)
    : MeshGenerator(problem), m_isError(false)
{
}

bool MeshGeneratorGMSH::mesh()
{
    Agros::log()->printMessage(tr("Mesh Generator"), tr("GMSH"));

    // create gmsh files
    if (prepare(true) && writeToGmsh())
    {
        QString gmshBinary = "gmsh";
        if (QCoreApplication::instance() && QFile::exists(QCoreApplication::applicationDirPath() + QDir::separator() + "gmsh.exe"))
            gmshBinary = "\"" + QCoreApplication::applicationDirPath() + QDir::separator() + "gmsh.exe\"";
        if (QCoreApplication::instance() && QFile::exists(QCoreApplication::applicationDirPath() + QDir::separator() + "gmsh"))
            gmshBinary = QCoreApplication::applicationDirPath() + QDir::separator() + "gmsh";

        QString triangleGMSH = QString("%1 -format msh22 -2 \"%2.geo\"").arg(gmshBinary).arg(tempProblemFileName());


        QSharedPointer<QProcess> process = QSharedPointer<QProcess>(new QProcess());
        process->setStandardOutputFile(tempProblemFileName() + ".gmsh.out");
        process->setStandardErrorFile(tempProblemFileName() + ".gmsh.err");
        process->startCommand(triangleGMSH);

        if (!process->waitForStarted())
        {
            Agros::log()->printError(tr("Mesh Generator"), tr("Could not start GMSH"));
            process->kill();
            process->close();

            return false;
        }

        if (process->waitForFinished(-1))
        {
            if (process->exitCode() == 0 && readGmshMeshFile())
            {

                //  remove gmsh temp files
                QFile::remove(tempProblemFileName() + ".gmsh.out");
                QFile::remove(tempProblemFileName() + ".gmsh.err");
            }
            else
            {
                m_isError = true;

                QString errorMessage = readFileContent(tempProblemFileName() + ".gmsh.err");
                errorMessage.insert(0, "\n");
                errorMessage.append("\n");
                Agros::log()->printError(tr("Mesh Generator"), errorMessage);
            }

        }
        else
        {
            QString solverOutputMessage = readFileContent(tempProblemDir() + "/gmsh.out").trimmed();
            if (!solverOutputMessage.isEmpty())
            {
                solverOutputMessage.insert(0, "\n");
                Agros::log()->printWarning(tr("External solver"), solverOutputMessage);
                std::cerr << solverOutputMessage.toStdString() << std::endl;
            }

            QString errorMessage = readFileContent(tempProblemDir() + "/gmsh.err");
            errorMessage.insert(0, "\n");
            errorMessage.append("\n");

            Agros::log()->printError(tr("External solver"), errorMessage);
        }

    }

    return !m_isError;
}

bool MeshGeneratorGMSH::writeToGmsh()
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

    QDir dir;
    dir.mkdir(QDir::temp().absolutePath() + "/agros2d");
    QFile file(tempProblemFileName() + ".geo");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        Agros::log()->printError(tr("Mesh Generator"), tr("Could not create GMSH geometry file (%1)").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);

    // mesh size
    RectPoint rect = m_problem->scene()->boundingBox();
    out << QString("mesh_size = %1;\n").arg(qMin(rect.width(), rect.height()) / 6.0);
    //out << QString("mesh_size = 0;\n");

    // nodes
    QString outNodes;
    int nodesCount = 0;
    for (int i = 0; i<m_problem->scene()->nodes->length(); i++)
    {
        outNodes += QString("Point(%1) = {%2, %3, 0, mesh_size};\n").
                arg(i).
                arg(m_problem->scene()->nodes->at(i)->point().x, 0, 'f', 10).
                arg(m_problem->scene()->nodes->at(i)->point().y, 0, 'f', 10);
        nodesCount++;
    }

    // edges
    QString outEdges;
    int edgesCount = 0;
    for (int i = 0; i<m_problem->scene()->faces->length(); i++)
    {
        if (m_problem->scene()->faces->at(i)->angle() == 0)
        {
            // line .. increase edge index to count from 1
            outEdges += QString("Line(%1) = {%2, %3};\n").
                    arg(edgesCount+1).
                    arg(m_problem->scene()->nodes->items().indexOf(m_problem->scene()->faces->at(i)->nodeStart())).
                    arg(m_problem->scene()->nodes->items().indexOf(m_problem->scene()->faces->at(i)->nodeEnd()));
            edgesCount++;
        }
        else
        {
            // arc
            // add pseudo nodes
            Point center = m_problem->scene()->faces->at(i)->center();
            outNodes += QString("Point(%1) = {%2, %3, 0};\n").
                    arg(nodesCount).
                    arg(center.x, 0, 'f', 10).
                    arg(center.y, 0, 'f', 10);
            nodesCount++;

            outEdges += QString("Circle(%1) = {%2, %3, %4};\n").
                    arg(edgesCount+1).
                    arg(m_problem->scene()->nodes->items().indexOf(m_problem->scene()->faces->at(i)->nodeStart())).
                    arg(nodesCount - 1).
                    arg(m_problem->scene()->nodes->items().indexOf(m_problem->scene()->faces->at(i)->nodeEnd()));

            edgesCount++;
        }
    }

    /*
                    // holes
                    int holesCount = 0;
                    foreach (SceneLabel *label, Agros::scene()->labels->items())
                        if (label->markersCount() == 0)
                            holesCount++;

                    QString outHoles = QString("%1\n").arg(holesCount);
                    holesCount = 0;
                    foreach (SceneLabel *label, Agros::scene()->labels->items())
                    {
                        if (label->markersCount() == 0)
                        {
                            outHoles += QString("%1  %2  %3\n").
                                    arg(holesCount).
                                    // arg(Agros::scene()->labels->items().indexOf(label) + 1).
                                    arg(label->point().x, 0, 'f', 10).
                                    arg(label->point().y, 0, 'f', 10);

                            holesCount++;
                        }
                    }

                    // labels
                    QString outLabels;
                    int labelsCount = 0;
                    foreach (SceneLabel *label, Agros::scene()->labels->items())
                    {
                        if (label->markersCount() > 0)
                        {
                            outLabels += QString("%1  %2  %3  %4  %5\n").
                                    arg(labelsCount).
                                    arg(label->point().x, 0, 'f', 10).
                                    arg(label->point().y, 0, 'f', 10).
                                    // arg(labelsCount + 1). // triangle returns zero region number for areas without marker, markers must start from 1
                                    arg(Agros::scene()->labels->items().indexOf(label) + 1).
                                    arg(label->area());
                            labelsCount++;
                        }
                    }

                    out << outHoles;
                    outLabels.insert(0, QString("%1 1\n").
                                     arg(labelsCount)); // - holes
                    out << outLabels;
                    */

    //    try
    //    {
    //        m_problem->scene()->loopsInfo()->processLoops();
    //    }
    //    catch (AgrosMeshException& ame)
    //    {
    //        Agros::log()->printError(tr("Mesh Generator"), ame.toString());
    //        std::cout << "Missing Label";
    //        return false;
    //    }

    QString outLoops;
    for(int i = 0; i < m_problem->scene()->loopsInfo()->loops().size(); i++)
    {
        if (!m_problem->scene()->loopsInfo()->outsideLoops().contains(i))
        {
            outLoops.append(QString("Line Loop(%1) = {").arg(i+1));
            for(int j = 0; j < m_problem->scene()->loopsInfo()->loops().at(i).size(); j++)
            {
                if (m_problem->scene()->loopsInfo()->loops().at(i)[j].reverse)
                    outLoops.append("-");
                outLoops.append(QString("%1").arg(m_problem->scene()->loopsInfo()->loops().at(i)[j].edge + 1));
                if (j < m_problem->scene()->loopsInfo()->loops().at(i).size() - 1)
                    outLoops.append(",");
            }
            outLoops.append(QString("};\n"));
        }
    }
    outLoops.append("\n");

    QList<int> surfaces;
    int surfaceCount = 0;
    for (int i = 0; i < m_problem->scene()->labels->count(); i++)
    {
        surfaceCount++;
        SceneLabel* label = m_problem->scene()->labels->at(i);
        if(!label->isHole())
        {
            surfaces.push_back(surfaceCount);
            outLoops.append(QString("Plane Surface(%1) = {").arg(surfaceCount));
            for (int j = 0; j < m_problem->scene()->loopsInfo()->labelLoops()[label].count(); j++)
            {
                outLoops.append(QString("%1").arg(m_problem->scene()->loopsInfo()->labelLoops()[label][j]+1));
                if (j < m_problem->scene()->loopsInfo()->labelLoops()[label].count() - 1)
                    outLoops.append(",");
            }
            outLoops.append(QString("};\n"));
        }
    }

    //    outLoops.append(QString("Physical Surface(1) = {"));
    //    for(int i = 0; i < surfaceCount; i++)
    //    {
    //        outLoops.append(QString("%1").arg(i+1));
    //        if(i < surfaceCount - 1)
    //            outLoops.append(",");
    //    }
    //    outLoops.append(QString("};\n"));

    // quad mesh
    if (m_problem->config()->meshType() == MeshType_GMSH_Quad)
    {
        outLoops.append(QString("Recombine Surface {"));
        for(int i = 0; i <  surfaces.count(); i++)
        {
            outLoops.append(QString("%1").arg(surfaces.at(i)));
            if(i < surfaces.count() - 1)
                outLoops.append(",");
        }
        outLoops.append(QString("};\n"));
    }
    //    QString outLoops;
    //    outLoops.append(QString("Line Loop(1) = {0, 1, 2, 3};\n"));
    //    outLoops.append(QString("Plane Surface(1) = {1};\n"));
    //    outLoops.append(QString("Line Loop(2) = {4, 5, 6, -1};\n"));
    //    outLoops.append(QString("Plane Surface(2) = {2};\n"));
    //    outLoops.append("\n");

    //    // quad mesh
    //    if (m_problem->config()->meshType() == MeshType_GMSH_Quad)
    //        outLoops.append(QString("Recombine Surface {1, 2};\n"));

    // Mesh.Algorithm - 1=MeshAdapt, 2=Automatic, 5=Delaunay, 6=Frontal, 7=bamg, 8=delquad
    QString outCommands;
    if (m_problem->config()->meshType() == MeshType_GMSH_Quad)
    {
        outCommands.append(QString("Mesh.Algorithm = 2;\n"));
        outCommands.append(QString("Mesh.SubdivisionAlgorithm = 1;\n"));
    }
    // else if (m_problem->config()->meshType() == MeshType_GMSH_QuadDelaunay_Experimental)
    // {
    //     outCommands.append(QString("Mesh.Algorithm = 8;\n"));
    //     outCommands.append(QString("Mesh.SubdivisionAlgorithm = 1;\n"));
    // }

    outNodes.insert(0, QString("\n// nodes\n"));
    out << outNodes;
    outEdges.insert(0, QString("\n// edges\n"));
    out << outEdges;
    outLoops.insert(0, QString("\n// loops\n"));
    out << outLoops;
    outCommands.insert(0, QString("\n// commands\n"));
    out << outCommands;

    file.waitForBytesWritten(0);
    file.close();

    // set system locale
    // setlocale(LC_NUMERIC, plocale);

    return true;
}

bool MeshGeneratorGMSH::readGmshMeshFile()
{
    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    int k = -1;

    QFile fileGMSH(tempProblemFileName() + ".msh");
    if (!fileGMSH.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Agros::log()->printError(tr("Mesh Generator"), tr("Could not read GMSH mesh file"));
        return false;
    }
    QTextStream inGMSH(&fileGMSH);

    // nodes
    inGMSH.readLine();
    inGMSH.readLine();
    inGMSH.readLine();
    inGMSH.readLine();
    k = inGMSH.readLine().toInt();
    // sscanf(inGMSH.readLine().toLatin1().data(), "%i", &k);
    for (int i = 0; i < k; i++)
    {
        QString s = inGMSH.readLine();
        QStringList data = s.split(QRegularExpression("\\s+")); // QString::SkipEmptyParts

        // int n = data[0].toInt();
        double x = data[1].toDouble();
        double y = data[2].toDouble();
        // double z = data[3].toDouble();

        nodeList.append(Point(x, y));
    }

    // elements
    inGMSH.readLine();
    inGMSH.readLine();
    k = inGMSH.readLine().toInt();
    QSet<int> labelMarkersCheck;
    for (int i = 0; i < k; i++)
    {
        QString s = inGMSH.readLine();
        QStringList data = s.split(QRegularExpression("\\s+"));

        // int n = data[0].toInt();
        int type = data[1].toInt();
        // int phys = data[2].toInt();
        // int part = data[3].toInt();
        int marker = data[4].toInt();
        // edge
        if (type == 1)
        {
            int quad0 = data[5].toInt();
            int quad1 = data[6].toInt();

            edgeList.append(MeshEdge(quad0 - 1, quad1 - 1, marker - 1)); // marker conversion from gmsh, where it starts from 1
        }
        // triangle
        if (type == 2)
        {
            int quad0 = data[5].toInt();
            int quad1 = data[6].toInt();
            int quad2 = data[7].toInt();

            elementList.append(MeshElement(quad0 - 1, quad1 - 1, quad2 - 1, marker - 1)); // marker conversion from gmsh, where it starts from 1
        }
        // quad
        if (type == 3)
        {
            int quad0 = data[5].toInt();
            int quad1 = data[6].toInt();
            int quad2 = data[7].toInt();
            int quad3 = data[8].toInt();

            elementList.append(MeshElement(quad0 - 1, quad1 - 1, quad2 - 1, quad3 - 1, marker - 1)); // marker conversion from gmsh, where it starts from 1
        }


        /*
        if (marker == 0)
        {
            Agros::log()->printError(tr("Mesh Generator"), tr("Some areas have no label marker"));
            return false;
        }
        */
        labelMarkersCheck.insert(marker - 1);
    }

    fileGMSH.close();

    fillNeighborStructures();

    writeTodealii();
    /*
    QList<QList<QPair<int, int> > > edges_between_elements;

    // elements
    std::vector<dealii::CellData<2> > cells;
    for (int element_i = 0; element_i < elementList.count(); element_i++)
    {
        edges_between_elements.push_back(QList<QPair<int, int> > ());
    }

    // boundary markers
    dealii::SubCellData subcelldata;
    for (int edge_i = 0; edge_i < edgeList.count(); edge_i++)
    {
        if (edgeList[edge_i].marker == -1)
            continue;

        dealii::CellData<1> cell_data;
        cell_data.vertices[0] = edgeList[edge_i].node[0];
        cell_data.vertices[1] = edgeList[edge_i].node[1];

        if (edgeList[edge_i].neighElem[1] != -1)
        {
            edges_between_elements[edgeList[edge_i].neighElem[0]].push_back(QPair<int, int>(edgeList[edge_i].neighElem[1], edgeList[edge_i].marker + 1));
            edges_between_elements[edgeList[edge_i].neighElem[1]].push_back(QPair<int, int>(edgeList[edge_i].neighElem[0], edgeList[edge_i].marker + 1));

            // do not push the boundary line
            continue;
        }
    }

    dealii::GridIn<2> gridin;
    gridin.attach_triangulation(m_triangulation);
    std::ifstream f(tempProblemFileName().toStdString() + ".msh");
    gridin.read_msh(f);

    // m_triangulation.create_triangulation_compatibility(vertices, cells, subcelldata);
    // m_triangulation.create_triangulation(vertices, cells, subcelldata);

    dealii::Triangulation<2>::cell_iterator cell = m_triangulation.begin();
    dealii::Triangulation<2>::cell_iterator end_cell = m_triangulation.end();

    int cell_idx = 0;
    for(; cell != end_cell; ++cell)
    {
        // todo: probably active is not neccessary
        if(cell->active())
        {
            for(int neigh_i = 0; neigh_i < dealii::GeometryInfo<2>::faces_per_cell; neigh_i++)
            {
                if(cell->face(neigh_i)->boundary_id() == dealii::numbers::internal_face_boundary_id)
                {
                    cell->face(neigh_i)->set_user_index(0);
                }
                else
                {
                    cell->face(neigh_i)->set_user_index(cell->face(neigh_i)->boundary_id());
                    // std::cout << "cell cell_idx: " << cell_idx << ", face  " << neigh_i << " set to " << cell->face(neigh_i)->boundary_indicator() << " -> value " << cell->face(neigh_i)->user_index() << std::endl;
                }

                int neighbor_cell_idx = cell->neighbor_index(neigh_i);
                if(neighbor_cell_idx != -1)
                {
                    assert(cell->face(neigh_i)->user_index() == 0);
                    QPair<int, int> neighbor_edge_pair;
                    foreach(neighbor_edge_pair, edges_between_elements[cell_idx])
                    {
                        if(neighbor_edge_pair.first == neighbor_cell_idx)
                        {
                            cell->face(neigh_i)->set_user_index(neighbor_edge_pair.second);
                            //std::cout << "cell cell_idx: " << cell_idx << ", face adj to " << neighbor_cell_idx << " set to " << neighbor_edge_pair.second << " -> value " << cell->face(neigh_i)->user_index() << std::endl;
                            //dealii::TriaAccessor<1,2,2> line = cell->line(neigh_i);
                            //cell->neighbor()
                        }
                    }
                }
            }
            cell_idx++;
        }
    }


    */




    nodeList.clear();
    edgeList.clear();
    elementList.clear();

    return true;
}

