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

#ifndef PYTHONLABGEOMETRY_H
#define PYTHONLABGEOMETRY_H

#include "util/global.h"
#include "scene.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

class SwigGeometry
{
    public:
        SwigGeometry() {}
        ~SwigGeometry() {}

        void activate();

        // add operations
        int addNode(std::string x, std::string y);
        int addEdge(std::string x1, std::string y1, std::string x2, std::string y2, std::string angle, int segments, int curvilinear,
                    const map<std::string, int> &refinements, const map<std::string, std::string> &boundaries);
        int addEdgeByNodes(int nodeStartIndex, int nodeEndIndex, std::string angle, int segments, int curvilinear,
                           const map<std::string, int> &refinements, const map<std::string, std::string> &boundaries);
        int addLabel(std::string x, std::string y, double area, const map<std::string, int> &refinements,
                     const map<std::string, int> &orders, const map<std::string, std::string> &materials);

        int nodesCount() const;
        int edgesCount() const;
        int labelsCount() const;

        // modify operations
        void modifyEdge(int index, std::string angle, int segments, int isCurvilinear, const map<std::string, int> &refinements,
                        const map<std::string, std::string> &boundaries);
        void modifyLabel(int index, double area, const map<std::string, int> &refinements,
                         const map<std::string, int> &orders, const map<std::string, std::string> &materials);

        // remove operations
        void removeNodes(const vector<int> &nodes);
        void removeEdges(const vector<int> &edges);
        void removeLabels(const vector<int> &labels);

        // select operations
        void selectNodes(const vector<int> &nodes);
        void selectEdges(const vector<int> &edges);
        void selectLabels(const vector<int> &labels);

        void selectNodeByPoint(double x, double y);
        void selectEdgeByPoint(double x, double y);
        void selectLabelByPoint(double x, double y);

        void selectNone();

        // transform operations
        void moveSelection(double dx, double dy, bool copy, bool withMarkers);
        void rotateSelection(double x, double y, double angle, bool copy, bool withMarkers);
        void scaleSelection(double x, double y, double scale, bool copy, bool withMarkers);
        void removeSelection();

        // vtk
        void exportVTK(const std::string &fileName) const;
        void exportSVG(const std::string &fileName) const;
        std::string exportSVG() const;

private:
        void testAngle(double angle) const;
        void testSegments(int segments) const;
        void setBoundaries(SceneFace *edge, const map<std::string, std::string> &boundaries);
        void setMaterials(SceneLabel *label, const map<std::string, std::string> &materials);
        void setRefinements(SceneLabel *label, const map<std::string, int> &refinements);
        void setPolynomialOrders(SceneLabel *label, const map<std::string, int> &orders);
};

#endif // PYTHONLABGEOMETRY_H
