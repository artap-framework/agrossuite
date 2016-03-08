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

#include "recipedialog.h"

#include "util/global.h"
#include "gui/lineeditdouble.h"

#include "scene.h"
#include "solver/problem.h"
#include "solver/problem_config.h"

#include "solver/plugin_interface.h"

RecipeDialog::RecipeDialog(ResultRecipe *recipe, QWidget *parent)
    : QDialog(parent), m_recipe(recipe)
{    
    setAttribute(Qt::WA_DeleteOnClose);
}

void RecipeDialog::createControls()
{
    setWindowTitle(tr("Recipe - %1: %2").arg(resultRecipeTypeString(m_recipe->type())).arg(m_recipe->name()));

    txtName = new QLineEdit(m_recipe->name(), this);
    connect(txtName, SIGNAL(textChanged(QString)), this, SLOT(recipeNameTextChanged(QString)));
    lblType = new QLabel(resultRecipeTypeString(m_recipe->type()));

    QGridLayout *nameLayout = new QGridLayout();
    nameLayout->addWidget(new QLabel(tr("Name:")), 0, 0);
    nameLayout->addWidget(txtName, 0, 1);
    nameLayout->addWidget(new QLabel(tr("Type:")), 1, 0);
    nameLayout->addWidget(lblType, 1, 1);

    QGroupBox *grpName = new QGroupBox(tr("Name and type"));
    grpName->setLayout(nameLayout);

    cmbField = new QComboBox(this);
    foreach (QString fieldId, Agros2D::problem()->fieldInfos().keys())
        cmbField->addItem(Agros2D::problem()->fieldInfo(fieldId)->name(), fieldId);
    connect(cmbField, SIGNAL(currentIndexChanged(int)), this, SLOT(fieldChanged(int)));
    cmbField->setCurrentIndex(cmbField->findData(m_recipe->fieldId()));

    cmbVariable = new QComboBox(this);
    lblVariableComp = new QLabel(tr("Component:"));
    cmbVariableComp = new QComboBox(this);
    txtTimeStep = new QSpinBox(this);
    txtTimeStep->setMinimum(-1);
    txtTimeStep->setValue(m_recipe->timeStep());
    txtAdaptivityStep = new QSpinBox(this);
    txtAdaptivityStep->setMinimum(-1);
    txtAdaptivityStep->setValue(m_recipe->adaptivityStep());

    QGridLayout *fieldLayout = new QGridLayout();
    fieldLayout->addWidget(new QLabel(tr("Time step:")), 0, 0);
    fieldLayout->addWidget(txtTimeStep, 0, 1);
    fieldLayout->addWidget(new QLabel(tr("Adaptive step:")), 1, 0);
    fieldLayout->addWidget(txtAdaptivityStep, 1, 1);
    fieldLayout->addWidget(new QLabel(tr("Field:")), 2, 0);
    fieldLayout->addWidget(cmbField, 2, 1);
    fieldLayout->addWidget(new QLabel(tr("Variable:")), 3, 0);
    fieldLayout->addWidget(cmbVariable, 3, 1);
    fieldLayout->addWidget(lblVariableComp, 4, 0);
    fieldLayout->addWidget(cmbVariableComp, 4, 1);

    QGroupBox *grpField = new QGroupBox(tr("Field and variable"));
    grpField->setLayout(fieldLayout);

    // update variables
    fieldChanged(cmbField->currentIndex());

    lblError = new QLabel();

    QPalette palette = lblError->palette();
    palette.setColor(QPalette::WindowText, QColor(Qt::red));
    lblError->setPalette(palette);
    lblError->setVisible(false);

    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layoutParametersWidget = new QVBoxLayout();
    layoutParametersWidget->addWidget(grpName);
    layoutParametersWidget->addWidget(grpField);
    layoutParametersWidget->addWidget(createRecipeControls());
    layoutParametersWidget->addWidget(lblError);
    layoutParametersWidget->addStretch();
    layoutParametersWidget->addWidget(buttonBox);

    setLayout(layoutParametersWidget);

    recipeNameTextChanged(txtName->text());
}

void RecipeDialog::doAccept()
{
    if (save())
        accept();
}

void RecipeDialog::doReject()
{
    reject();
}

bool RecipeDialog::save()
{
    if (checkRecipe(txtName->text()))
    {
        m_recipe->setName(txtName->text());
        m_recipe->setFieldId(cmbField->currentData().toString());
        m_recipe->setVariable(cmbVariable->currentData().toString());
        m_recipe->setTimeStep(txtTimeStep->value());
        m_recipe->setAdaptivityStep(txtAdaptivityStep->value());

        return true;
    }

    return false;
}

int RecipeDialog::showDialog()
{
    createControls();

    return exec();
}

void RecipeDialog::recipeNameTextChanged(const QString &str)
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(checkRecipe(str));
}

bool RecipeDialog::checkRecipe(const QString &str)
{
    try
    {
        Agros2D::problem()->config()->checkVariableName(str);
    }
    catch (AgrosException &e)
    {
        lblError->setText(e.toString());
        lblError->setVisible(true);

        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return false;
    }

    foreach (ResultRecipe *recipe, Agros2D::problem()->recipes()->items())
    {
        if (str == m_recipe->name())
            continue;

        if (str == recipe->name())
        {
            lblError->setText(tr("Recipe already exists."));
            lblError->setVisible(true);

            buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            return false;
        }
    }

    lblError->setVisible(false);

    return true;
}

// *************************************************************************************

LocalValueRecipeDialog::LocalValueRecipeDialog(LocalValueRecipe *recipe, QWidget *parent)
    : RecipeDialog(recipe, parent)
{
}

QWidget *LocalValueRecipeDialog::createRecipeControls()
{
    connect(cmbVariable, SIGNAL(currentIndexChanged(int)), this, SLOT(variableChanged(int)));
    lblVariableComp->setVisible(true);
    cmbVariableComp->setVisible(true);
    variableChanged(cmbVariable->currentIndex());

    txtPointX = new LineEditDouble(recipe()->point().x);
    txtPointY = new LineEditDouble(recipe()->point().y);

    QGridLayout *pointLayout = new QGridLayout();
    pointLayout->addWidget(new QLabel(tr("X:")), 0, 0);
    pointLayout->addWidget(txtPointX, 0, 1);
    pointLayout->addWidget(new QLabel(tr("Y:")), 1, 0);
    pointLayout->addWidget(txtPointY, 1, 1);

    QGroupBox *grpPoint = new QGroupBox(tr("Point"));
    grpPoint->setLayout(pointLayout);

    return grpPoint;
}

void LocalValueRecipeDialog::fieldChanged(int index)
{
    FieldInfo *fieldInfo = Agros2D::problem()->fieldInfo(cmbField->currentData(Qt::UserRole).toString());

    cmbVariable->clear();
    foreach (Module::LocalVariable variable, fieldInfo->viewScalarVariables(Agros2D::problem()->config()->coordinateType()))
        cmbVariable->addItem(variable.name(), variable.id());

    int selected = cmbVariable->findData(recipe()->variable());
    if (selected == -1)
        cmbVariable->setCurrentIndex(0);
    else
        cmbVariable->setCurrentIndex(selected);
}

void LocalValueRecipeDialog::variableChanged(int index)
{
    FieldInfo *fieldInfo = Agros2D::problem()->fieldInfo(cmbField->currentData(Qt::UserRole).toString());
    Module::LocalVariable physicFieldVariable = fieldInfo->localVariable(Agros2D::problem()->config()->coordinateType(),
                                                                         cmbVariable->itemData(index).toString());

    cmbVariableComp->clear();
    if (physicFieldVariable.isScalar())
    {
        cmbVariableComp->addItem(tr("Scalar"), PhysicFieldVariableComp_Scalar);
    }
    else
    {
        cmbVariableComp->addItem(tr("Magnitude"), PhysicFieldVariableComp_Magnitude);
        cmbVariableComp->addItem(Agros2D::problem()->config()->labelX(), PhysicFieldVariableComp_X);
        cmbVariableComp->addItem(Agros2D::problem()->config()->labelY(), PhysicFieldVariableComp_Y);
    }

    int selected = cmbVariable->findData(recipe()->variableComponent());
    if (selected == -1)
        cmbVariableComp->setCurrentIndex(0);
    else
        cmbVariableComp->setCurrentIndex(selected);
}

bool LocalValueRecipeDialog::save()
{
    if (RecipeDialog::save())
    {
        recipe()->setVariableComponent((PhysicFieldVariableComp) cmbVariableComp->currentData().toInt());
        recipe()->setPoint(txtPointX->value(), txtPointY->value());

        return true;
    }

    return false;
}

// *************************************************************************************

SurfaceIntegralRecipeDialog::SurfaceIntegralRecipeDialog(SurfaceIntegralRecipe *recipe, QWidget *parent)
    : RecipeDialog(recipe, parent)
{

}

QWidget *SurfaceIntegralRecipeDialog::createRecipeControls()
{
    lblVariableComp->setVisible(false);
    cmbVariableComp->setVisible(false);

    lstEdges = new QListWidget(this);

    for (int i = 0; i < Agros2D::problem()->scene()->faces->items().count(); i++)
    {
        QListWidgetItem *item = new QListWidgetItem(lstEdges);
        item->setText(QString("%1").arg(i));
        item->setData(Qt::UserRole, i);
        item->setCheckState(recipe()->edges().contains(i) ? Qt::Checked : Qt::Unchecked);
    }

    QGridLayout *edgesLayout = new QGridLayout();
    edgesLayout->addWidget(lstEdges, 0, 0);

    QGroupBox *grpEdges = new QGroupBox(tr("Edges"));
    grpEdges->setLayout(edgesLayout);

    return grpEdges;
}

void SurfaceIntegralRecipeDialog::fieldChanged(int index)
{
    FieldInfo *fieldInfo = Agros2D::problem()->fieldInfo(cmbField->currentData(Qt::UserRole).toString());

    cmbVariable->clear();
    foreach (Module::Integral variable, fieldInfo->surfaceIntegrals(Agros2D::problem()->config()->coordinateType()))
        cmbVariable->addItem(variable.name(), variable.id());

    int selected = cmbVariable->findData(recipe()->variable());
    if (selected == -1)
        cmbVariable->setCurrentIndex(0);
    else
        cmbVariable->setCurrentIndex(selected);
}

bool SurfaceIntegralRecipeDialog::save()
{
    if (RecipeDialog::save())
    {
        recipe()->clear();
        for (int i = 0; i < lstEdges->count(); i++)
        {
            if (lstEdges->item(i)->checkState() == Qt::Checked)
                recipe()->addEdge(i);
        }

        return true;

    }

    return false;
}

// *************************************************************************************

VolumeIntegralRecipeDialog::VolumeIntegralRecipeDialog(VolumeIntegralRecipe *recipe, QWidget *parent)
    : RecipeDialog(recipe, parent)
{

}

QWidget *VolumeIntegralRecipeDialog::createRecipeControls()
{
    lblVariableComp->setVisible(false);
    cmbVariableComp->setVisible(false);

    lstVolumes = new QListWidget(this);

    for (int i = 0; i < Agros2D::problem()->scene()->labels->items().count(); i++)
    {
        QListWidgetItem *item = new QListWidgetItem(lstVolumes);
        item->setText(QString("%1").arg(i));
        item->setData(Qt::UserRole, i);
        item->setCheckState(recipe()->labels().contains(i) ? Qt::Checked : Qt::Unchecked);
    }

    QGridLayout *volumesLayout = new QGridLayout();
    volumesLayout->addWidget(lstVolumes, 0, 0);

    QGroupBox *grpVolumes = new QGroupBox(tr("Labels"));
    grpVolumes->setLayout(volumesLayout);

    return grpVolumes;
}

void VolumeIntegralRecipeDialog::fieldChanged(int index)
{
    FieldInfo *fieldInfo = Agros2D::problem()->fieldInfo(cmbField->currentData(Qt::UserRole).toString());

    cmbVariable->clear();
    foreach (Module::Integral variable, fieldInfo->volumeIntegrals(Agros2D::problem()->config()->coordinateType()))
        cmbVariable->addItem(variable.name(), variable.id());

    int selected = cmbVariable->findData(recipe()->variable());
    if (selected == -1)
        cmbVariable->setCurrentIndex(0);
    else
        cmbVariable->setCurrentIndex(selected);
}

bool VolumeIntegralRecipeDialog::save()
{
    if (RecipeDialog::save())
    {
        recipe()->clear();
        for (int i = 0; i < lstVolumes->count(); i++)
        {
            if (lstVolumes->item(i)->checkState() == Qt::Checked)
                recipe()->addLabel(i);
        }

        return true;
    }

    return false;
}
