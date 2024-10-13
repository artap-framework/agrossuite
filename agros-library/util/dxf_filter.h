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

#ifndef UTIL_DXF_FILTER_H
#define UTIL_DXF_FILTER_H

#include "util/util.h"

#include "libdxfrw.h"

class Scene;

class DxfInterfaceDXFRW : public DRW_Interface
{
public:
    DxfInterfaceDXFRW(Scene *scene, const QString &fileName);

    void read();
    void write();

    /** Called for every AppId entry. */
    virtual void addAppId(const DRW_AppId& data) override {}
    virtual void writeAppId() override;

    virtual void addArc(const DRW_Arc &a) override;
    virtual void addLine(const DRW_Line &l) override;
    virtual void addCircle(const DRW_Circle& c) override;
    virtual void addPolyline(const DRW_Polyline& data) override;
    virtual void addLWPolyline(const DRW_LWPolyline& data) override;
    virtual void addSpline(const DRW_Spline* data) override;
    virtual void addBlock(const DRW_Block& data) override;
    virtual void setBlock(const int handle) override;
    virtual void endBlock() override;
    virtual void addInsert(const DRW_Insert& data) override;

    // Methods from DRW_CreationInterface:
    virtual void addHeader(const DRW_Header* data) override {}
    virtual void addLType(const DRW_LType& data) override {}
    virtual void addLayer(const DRW_Layer& data) override {}
    virtual void addDimStyle(const DRW_Dimstyle& data) override {}
    virtual void addVport(const DRW_Vport& data) override {}
    virtual void addTextStyle(const DRW_Textstyle& data) override {}
    virtual void addPoint(const DRW_Point& data) override {}
    virtual void addRay(const DRW_Ray& data) override {}
    virtual void addXline(const DRW_Xline& data) override {}
    virtual void addEllipse(const DRW_Ellipse& data) override {}
    virtual void addText(const DRW_Text& data) override {}
    virtual void addKnot(const DRW_Entity&) override {}
    virtual void addTrace(const DRW_Trace& data) override {}
    virtual void addSolid(const DRW_Solid& data) override {}
    virtual void addMText(const DRW_MText& data) override {}
    virtual void addDimAlign(const DRW_DimAligned *data) override {}
    virtual void addDimLinear(const DRW_DimLinear *data) override {}
    virtual void addDimRadial(const DRW_DimRadial *data) override {}
    virtual void addDimDiametric(const DRW_DimDiametric *data) override {}
    virtual void addDimAngular(const DRW_DimAngular *data) override {}
    virtual void addDimAngular3P(const DRW_DimAngular3p *data) override {}
    virtual void addDimOrdinate(const DRW_DimOrdinate *data) override {}
    virtual void addLeader(const DRW_Leader *data) override {}
    virtual void addHatch(const DRW_Hatch* data) override {}
    virtual void addViewport(const DRW_Viewport& data) override {}
    virtual void addImage(const DRW_Image* data) override {}
    virtual void linkImage(const DRW_ImageDef* data) override {}

    virtual void add3dFace(const DRW_3Dface& data) override {}
    virtual void addComment(const char*) override {}

    // Export:
    virtual void writeHeader(DRW_Header& data) override;
    virtual void writeEntities() override;
    virtual void writeLTypes() override {}
    virtual void writeLayers() override {}
    virtual void writeTextstyles() override {}
    virtual void writeVports() override {}
    virtual void writeBlockRecords() override {}
    virtual void writeBlocks() override {}
    virtual void writeDimstyles() override {}

private:
    Scene *m_scene;
    dxfRW *m_dxf;

    struct DXFInsert
    {
        QString blockName;

        QMap<QString, QList<DRW_Line> > lines;
        QMap<QString, QList<DRW_Arc> > arcs;

        void clear()
        {
            blockName.clear();

            // entities
            lines.clear();
        }

        inline bool isEmpty() { return blockName.isEmpty(); }
    };

    bool m_isBlock;
    DXFInsert m_activeInsert;
};

void readFromDXF(Scene *scene, const QString &fileName);
void writeToDXF(Scene *scene, const QString &fileName);

#endif // UTIL_DXF_FILTER_H

