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

#ifndef RECIPEDIALOG_H
#define RECIPEDIALOG_H

#include "app/sceneview_study.h"
#include "util/util.h"
#include "gui/other.h"
#include "solver/problem_result.h"

class LineEditDouble;

class RecipeDialog : public QDialog
{
    Q_OBJECT
public:
    RecipeDialog(ResultRecipe *recipe, QWidget *parent);

    int showDialog();

protected:
    ResultRecipe *m_recipe;

    QLabel *lblError;

    QLabel *lblType;
    QLineEdit *txtName;
    QComboBox *cmbField;
    QComboBox *cmbVariable;

    QLabel *lblVariableComp;
    QComboBox *cmbVariableComp;

    QSpinBox *txtTimeStep;
    QSpinBox *txtAdaptivityStep;

    QDialogButtonBox *buttonBox;

    void createControls();
    virtual QWidget *createRecipeControls() = 0;

    bool checkRecipe(const QString &str);

protected slots:
    void doAccept();
    void doReject();

    virtual bool save();

    virtual void fieldChanged(int index) = 0;
    void recipeNameTextChanged(const QString &str);
};

class LocalValueRecipeDialog : public RecipeDialog
{
    Q_OBJECT
public:
    LocalValueRecipeDialog(LocalValueRecipe *recipe, QWidget *parent);

protected:
    LineEditDouble *txtPointX;
    LineEditDouble *txtPointY;

    inline LocalValueRecipe *recipe() const { return dynamic_cast<LocalValueRecipe *>(m_recipe); }

    QWidget *createRecipeControls() override;

    SceneViewStudy *m_sceneViewStudy;

protected slots:
    void fieldChanged(int index);
    void variableChanged(int index);

    void selectedPoint(const Point &p) const;
    void doEditingFinished() const;

    virtual bool save();
};

class SurfaceIntegralRecipeDialog : public RecipeDialog
{
    Q_OBJECT
public:
    SurfaceIntegralRecipeDialog(SurfaceIntegralRecipe *recipe, QWidget *parent);

protected:
    inline SurfaceIntegralRecipe *recipe() const { return dynamic_cast<SurfaceIntegralRecipe *>(m_recipe); }

    QWidget *createRecipeControls() override;

    SceneViewStudy *m_sceneViewStudy;

protected slots:
    void fieldChanged(int index) override;

    bool save() override;
};

class VolumeIntegralRecipeDialog : public RecipeDialog
{
    Q_OBJECT
public:
    VolumeIntegralRecipeDialog(VolumeIntegralRecipe *recipe, QWidget *parent);

protected:
    inline VolumeIntegralRecipe *recipe() const { return dynamic_cast<VolumeIntegralRecipe *>(m_recipe); }

    QWidget *createRecipeControls() override;

    SceneViewStudy *m_sceneViewStudy;

protected slots:
    void fieldChanged(int index) override;

    bool save() override;
};

#endif // RECIPEDIALOG_H
