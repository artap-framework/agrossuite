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

#include "resultsview.h"

#include "util/constants.h"
#include "util/global.h"

#include "gui/common.h"
#include "gui/valuelineedit.h"

#include "scene.h"
#include "solver/plugin_interface.h"
#include "solver/module.h"
#include "solver/field.h"
#include "solver/solutionstore.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "sceneview_post.h"
#include "postprocessorview.h"

#include <ctemplate/template.h>

ResultsView::ResultsView(QWidget *parent, PostprocessorWidget *postprocessorWidget) : QWidget(parent),
    m_sceneModePostprocessor(SceneModePostprocessor_Empty)
{
    setObjectName("ResultsView");

    QSettings settings;
    trvWidget = new QTreeWidget(this);
    trvWidget->setHeaderHidden(false);
    trvWidget->setHeaderLabels(QStringList() << tr("Name") << tr("Var.") << tr("Value"));
    // trvWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setUniformRowHeights(true);
    trvWidget->setColumnCount(3);
    trvWidget->setColumnWidth(0, settings.value("ResultsView/TreeColumnWidth0", 200).toInt());
    trvWidget->setColumnWidth(1, settings.value("ResultsView/TreeColumnWidth1", 150).toInt());
    trvWidget->setColumnWidth(2, settings.value("ResultsView/TreeColumnWidth2", 70).toInt());
    trvWidget->setIndentation(trvWidget->indentation() - 2);

    connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));

    // main widget
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(trvWidget);
    layout->setContentsMargins(0, 0, 0, 0);

    setLayout(layout);

    // reconnect computation slots
    connect(postprocessorWidget, SIGNAL(connectComputation(QSharedPointer<Computation>)), this, SLOT(connectComputation(QSharedPointer<Computation>)));
}

ResultsView::~ResultsView()
{
    QSettings settings;
    settings.setValue("ResultsView/TreeColumnWidth0", trvWidget->columnWidth(0));
    settings.setValue("ResultsView/TreeColumnWidth1", trvWidget->columnWidth(1));
    settings.setValue("ResultsView/TreeColumnWidth2", trvWidget->columnWidth(2));
}

void ResultsView::doContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *current = trvWidget->itemAt(pos);
    if (current && !current->data(0, Qt::UserRole).isNull())
    {

        QAction *actCopy = new QAction(tr("Copy value"), this);
        connect(actCopy, SIGNAL(triggered(bool)), this, SLOT(doCopy(bool)));

        QMenu *mnuView = new QMenu(this);
        mnuView->addAction(actCopy);

        mnuView->exec(QCursor::pos());
    }
}

void ResultsView::doCopy(bool state)
{
    if (trvWidget->currentItem() && !trvWidget->currentItem()->data(0, Qt::UserRole).isNull())
    {
        QApplication::clipboard()->setText(QString::number(trvWidget->currentItem()->data(0, Qt::UserRole).toDouble()));
    }
}

void ResultsView::connectComputation(QSharedPointer<Computation> computation)
{
    if (!m_computation.isNull())
    {
        connect(m_computation.data()->postDeal(), SIGNAL(processed()), this, SLOT(doShowResults()));
    }

    m_computation = computation;

    if (!m_computation.isNull())
    {
        connect(m_computation.data()->postDeal(), SIGNAL(processed()), this, SLOT(doShowResults()));
    }
}

void ResultsView::doPostprocessorModeGroupChanged(SceneModePostprocessor sceneModePostprocessor)
{
    m_sceneModePostprocessor = sceneModePostprocessor;

    doShowResults();
}

void ResultsView::doShowResults()
{
    if (m_sceneModePostprocessor == SceneModePostprocessor_Empty)
        showEmpty();
    if (m_sceneModePostprocessor == SceneModePostprocessor_LocalValue)
        showPoint();
    if (m_sceneModePostprocessor == SceneModePostprocessor_SurfaceIntegral)
        showSurfaceIntegral();
    if (m_sceneModePostprocessor == SceneModePostprocessor_VolumeIntegral)
        showVolumeIntegral();
}

void ResultsView::showPoint(const Point &point)
{
    m_point = point;

    showPoint();
}

void ResultsView::showPoint()
{
    if (!(m_computation->isSolved() && m_computation->postDeal()->isProcessed()))
        return;

    trvWidget->setUpdatesEnabled(false);
    trvWidget->clear();

    QFont fnt = trvWidget->font();
    fnt.setBold(true);

    // point
    QTreeWidgetItem *itemPoint = new QTreeWidgetItem(trvWidget);
    itemPoint->setText(0, tr("Point"));
    itemPoint->setFont(0, fnt);
    itemPoint->setIcon(0, iconAlphabet('P', AlphabetColor_Bluegray));
    trvWidget->setItemWidget(itemPoint, 1, new QLabel(QString("<i>t</i> (s)")));
    itemPoint->setText(2, QString("%1").arg(m_computation->timeStepToTotalTime(m_computation->postDeal()->activeTimeStep()), 0, 'e', 3));
    itemPoint->setExpanded(true);

    QTreeWidgetItem *itemPointX = new QTreeWidgetItem(itemPoint);
    itemPointX->setText(0, "");
    trvWidget->setItemWidget(itemPointX, 1, new QLabel(QString("<i>%1</i> (m)").arg(m_computation->config()->labelX().toLower())));
    itemPointX->setText(2, QString("%1").arg(m_point.x, 0, 'e', 3));

    QTreeWidgetItem *itemPointY = new QTreeWidgetItem(itemPoint);
    itemPointY->setText(0, "");
    trvWidget->setItemWidget(itemPointY, 1, new QLabel(QString("<i>%1</i> (m)").arg(m_computation->config()->labelY().toLower())));
    itemPointY->setText(2, QString("%1").arg(m_point.y, 0, 'e', 3));

    foreach (FieldInfo *fieldInfo, m_computation->fieldInfos())
    {
        // field
        QTreeWidgetItem *fieldNode = new QTreeWidgetItem(trvWidget);
        fieldNode->setText(0, fieldInfo->name());
        fieldNode->setFont(0, fnt);
        fieldNode->setIcon(0, iconAlphabet(fieldInfo->fieldId().at(0), AlphabetColor_Green));
        fieldNode->setExpanded(true);

        std::shared_ptr<LocalValue> value = fieldInfo->plugin()->localValue(m_computation.data(),
                                                                            fieldInfo,
                                                                            m_computation->postDeal()->activeTimeStep(),
                                                                            m_computation->postDeal()->activeAdaptivityStep(),
                                                                            m_point);
        QMap<QString, LocalPointValue> values = value->values();
        if (values.size() > 0)
        {
            foreach (Module::LocalVariable variable, fieldInfo->localPointVariables(m_computation->config()->coordinateType()))
            {
                if (variable.isScalar())
                {
                    // scalar variable
                    QTreeWidgetItem *itemNode = new QTreeWidgetItem(fieldNode);
                    itemNode->setText(0, variable.name());
                    itemNode->setData(0, Qt::UserRole, values[variable.id()].scalar);
                    trvWidget->setItemWidget(itemNode, 1, new QLabel(QString("%1 (%2)").arg(variable.shortnameHtml()).arg(variable.unitHtml())));
                    itemNode->setText(2, QString("%1").arg(values[variable.id()].scalar, 0, 'e', 3));
                }
                else
                {
                    // vector variable
                    QTreeWidgetItem *itemNode = new QTreeWidgetItem(fieldNode);
                    itemNode->setText(0, variable.name());
                    itemNode->setData(0, Qt::UserRole, values[variable.id()].vector.magnitude());
                    trvWidget->setItemWidget(itemNode, 1, new QLabel(QString("%1 (%2)").arg(variable.shortnameHtml()).arg(variable.unitHtml())));
                    itemNode->setText(2, QString("%1").arg(values[variable.id()].vector.magnitude(), 0, 'e', 3));
                    itemNode->setExpanded(true);

                    QTreeWidgetItem *itemNodeX = new QTreeWidgetItem(itemNode);
                    itemNodeX->setText(0, "");
                    itemNodeX->setData(0, Qt::UserRole, values[variable.id()].vector.x);
                    trvWidget->setItemWidget(itemNodeX, 1, new QLabel(QString("%1<sub><i>%2</i></sub> (%3)").
                                                                      arg(variable.shortnameHtml()).
                                                                      arg(m_computation->config()->labelX().toLower()).
                                                                      arg(variable.unitHtml())));
                    itemNodeX->setText(2, QString("%1").arg(values[variable.id()].vector.x, 0, 'e', 3));

                    QTreeWidgetItem *itemNodeY = new QTreeWidgetItem(fieldNode);
                    itemNodeY->setText(0, "");
                    itemNodeY->setData(0, Qt::UserRole, values[variable.id()].vector.y);
                    trvWidget->setItemWidget(itemNodeY, 1, new QLabel(QString("%1<sub><i>%2</i></sub> (%3)").
                                                                      arg(variable.shortnameHtml()).
                                                                      arg(m_computation->config()->labelY().toLower()).
                                                                      arg(variable.unitHtml())));
                    itemNodeY->setText(2, QString("%1").arg(values[variable.id()].vector.y, 0, 'e', 3));
                }
            }
        }
    }

    trvWidget->setUpdatesEnabled(true);
}

void ResultsView::showVolumeIntegral()
{
    if (!m_computation->isSolved())
        return;

    trvWidget->setUpdatesEnabled(false);
    trvWidget->clear();

    QFont fnt = trvWidget->font();
    fnt.setBold(true);

    foreach (FieldInfo *fieldInfo, m_computation->fieldInfos())
    {
        // field
        QTreeWidgetItem *fieldNode = new QTreeWidgetItem(trvWidget);
        fieldNode->setText(0, fieldInfo->name());
        fieldNode->setFont(0, fnt);
        fieldNode->setIcon(0, iconAlphabet(fieldInfo->fieldId().at(0), AlphabetColor_Green));
        fieldNode->setExpanded(true);

        std::shared_ptr<IntegralValue> integral = fieldInfo->plugin()->volumeIntegral(m_computation.data(),
                                                                                      fieldInfo,
                                                                                      m_computation->postDeal()->activeTimeStep(),
                                                                                      m_computation->postDeal()->activeAdaptivityStep());

        QMap<QString, double> values = integral->values();
        if (values.size() > 0)
        {
            foreach (Module::Integral integral, fieldInfo->volumeIntegrals(m_computation->config()->coordinateType()))
            {
                // integral
                QTreeWidgetItem *itemNode = new QTreeWidgetItem(fieldNode);
                itemNode->setText(0, integral.name());
                itemNode->setData(0, Qt::UserRole, values[integral.id()]);
                trvWidget->setItemWidget(itemNode, 1, new QLabel(QString("%1 (%2)").arg(integral.shortnameHtml()).arg(integral.unitHtml())));
                itemNode->setText(2, QString("%1").arg(values[integral.id()], 0, 'e', 3));
            }
        }
    }

    trvWidget->setUpdatesEnabled(true);
}

void ResultsView::showSurfaceIntegral()
{
    if (!m_computation->isSolved())
        return;

    trvWidget->setUpdatesEnabled(false);
    trvWidget->clear();

    QFont fnt = trvWidget->font();
    fnt.setBold(true);

    foreach (FieldInfo *fieldInfo, m_computation->fieldInfos())
    {
        // field
        QTreeWidgetItem *fieldNode = new QTreeWidgetItem(trvWidget);
        fieldNode->setText(0, fieldInfo->name());
        fieldNode->setFont(0, fnt);
        fieldNode->setIcon(0, iconAlphabet(fieldInfo->fieldId().at(0), AlphabetColor_Green));
        fieldNode->setExpanded(true);


        std::shared_ptr<IntegralValue> integral = fieldInfo->plugin()->surfaceIntegral(m_computation.data(),
                                                                                       fieldInfo,
                                                                                       m_computation->postDeal()->activeTimeStep(),
                                                                                       m_computation->postDeal()->activeAdaptivityStep());
        QMap<QString, double> values = integral->values();
        if (values.size() > 0)
        {
            foreach (Module::Integral integral, fieldInfo->surfaceIntegrals(m_computation->config()->coordinateType()))
            {
                // integral
                QTreeWidgetItem *itemNode = new QTreeWidgetItem(fieldNode);
                itemNode->setText(0, integral.name());
                itemNode->setData(0, Qt::UserRole, values[integral.id()]);
                trvWidget->setItemWidget(itemNode, 1, new QLabel(QString("%1 (%2)").arg(integral.shortnameHtml()).arg(integral.unitHtml())));
                itemNode->setText(2, QString("%1").arg(values[integral.id()], 0, 'e', 3));
            }
        }
    }

    trvWidget->setUpdatesEnabled(true);
}

void ResultsView::showEmpty()
{
    trvWidget->clear();
}

LocalPointValueDialog::LocalPointValueDialog(Point point, Computation *computation, QWidget *parent) : QDialog(parent)
{
    setWindowIcon(icon("scene-node"));
    setWindowTitle(tr("Local point value"));

    setModal(true);

    txtPointX = new ValueLineEdit();
    txtPointX->setNumber(point.x);
    txtPointY = new ValueLineEdit();
    txtPointY->setNumber(point.y);

    connect(txtPointX, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtPointY, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));

    QFormLayout *layoutPoint = new QFormLayout();
    layoutPoint->addRow(computation->config()->labelX() + " (m):", txtPointX);
    layoutPoint->addRow(computation->config()->labelY() + " (m):", txtPointY);

    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutPoint);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

Point LocalPointValueDialog::point()
{
    return Point(txtPointX->value().number(), txtPointY->value().number());
}

void LocalPointValueDialog::evaluated(bool isError)
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!isError);
}
