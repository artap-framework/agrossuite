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

#ifndef STUDY_DIALOG_H
#define STUDY_DIALOG_H

#include <QWidget>

#include "gui/other.h"
#include "util/enums.h"
#include "logview.h"
#include "solver/problem.h"
#include "solver/problem_result.h"

#include "gui/logwidget.h"
#include "optilab/study.h"
#include "optilab/parameter.h"
#include "optilab/functional.h"

class LineEditDouble;
class Computation;
class Study;
class QCustomPlot;
class QCPGraph;

class AGROS_LIBRARY_API LogOptimizationDialog : public QDialog
{
    Q_OBJECT
public:
    LogOptimizationDialog(Study *study);
    ~LogOptimizationDialog();

protected:
    virtual void closeEvent(QCloseEvent *e);
    virtual void reject();

private:
    Study *m_study;

    LogWidget *m_logWidget;

    QPushButton *btnClose;
    QPushButton *btnAbort;

    QCustomPlot *m_totalChart;
    QList<QCustomPlot *> m_charts;
    QProgressBar *m_progress;
    int m_computationSetsCount;

    // steps
    int m_step;

    void createControls();

private slots:
    void updateChart(QList<double> values, double totalValue, SolutionUncertainty solutionUncertainty);
    void updateParameters(QList<Parameter> parameters, const Computation *computation);

    void solved();
    void aborted();

    void tryClose();
};

class StudySelectDialog : public QDialog
{
    Q_OBJECT
public:
    StudySelectDialog(QWidget *parent);

    inline StudyType selectedStudyType() { return m_selectedStudyType; }

private slots:
    void doItemSelected(QListWidgetItem *item);
    void doItemDoubleClicked(QListWidgetItem *item);

private:
    QListWidget *lstStudies;

    StudyType m_selectedStudyType;

    QDialogButtonBox *buttonBox;
};

class StudyDialog : public QDialog
{
    Q_OBJECT

public:
    StudyDialog(Study *study, QWidget *parent = 0);

    static StudyDialog *factory(Study *study, QWidget *parent = 0);

    int showDialog();

protected:
    Study *m_study;

    QTabWidget *tabStudy;

    void createControls();
    virtual QLayout *createStudyControls() { return new QHBoxLayout(this); }
    QWidget *createParametersAndFunctionals();
    QWidget *createParameters();
    QWidget *createFunctionals();

    QTreeWidget *trvParameterWidget;
    QPushButton *btnParameterAdd;
    QPushButton *btnParameterEdit;
    QPushButton *btnParameterRemove;
    void readParameters();

    QTreeWidget *trvFunctionalWidget;
    QPushButton *btnFunctionalAdd;
    QPushButton *btnFunctionalEdit;
    QPushButton *btnFunctionalRemove;
    void readFunctionals();

    QCheckBox *chkClearSolution;
    QCheckBox *chkSolveProblem;

    virtual void load();
    virtual void save();

private slots:
    void doAccept();

    void doParameterItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doParameterItemDoubleClicked(QTreeWidgetItem *item, int role);
    void doParameterAdd(bool checked);
    void doParameterEdit(bool checked);
    void doParameterRemove(bool checked);

    void doFunctionalItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doFunctionalItemDoubleClicked(QTreeWidgetItem *item, int role);
    void doFunctionalAdd(bool checked);
    void doFunctionalEdit(bool checked);
    void doFunctionalRemove(bool checked);

    void doDuplicate();
};

class StudyFunctionalDialog : public QDialog
{
    Q_OBJECT

public:
    StudyFunctionalDialog(Study *study, Functional *functional, QWidget *parent = 0);

private:
    Study *m_study;
    Functional *m_functional;

    QDialogButtonBox *buttonBox;
    QLabel *lblError;
    QLineEdit *txtName;
    QLineEdit *txtExpression;
    QSpinBox *txtWeight;

    void createControls();

    bool checkFunctional(const QString &str);

private slots:
    void doAccept();
    void functionalNameTextChanged(const QString &str);
};

class StudyParameterDialog : public QDialog
{
    Q_OBJECT

public:
    StudyParameterDialog(Study *study, Parameter *parameter, QWidget *parent = 0);

private:
    Study *m_study;
    Parameter *m_parameter;

    QDialogButtonBox *buttonBox;
    QLabel *lblError;
    QLabel *lblName;

    LineEditDouble *txtUpperBound;
    LineEditDouble *txtLowerBound;

    // penalty
    QCheckBox *chkPenaltyEnabled;
    LineEditDouble *txtScale;
    LineEditDouble *txtMu;
    LineEditDouble *txtSigma;
    QCustomPlot *m_chart;
    QCPGraph *m_penaltyChart;

    void createControls();

private slots:
    bool checkRange();

    void doAccept();
};

class ParameterSelectDialog : public QDialog
{
    Q_OBJECT
public:
    ParameterSelectDialog(Study *study, QWidget *parent);

    inline QString selectedParameterName() { return m_selectedParameterName; }

private slots:
    void doItemSelected(QListWidgetItem *item);
    void doItemDoubleClicked(QListWidgetItem *item);

private:
    Study *m_study;

    QListWidget *lstParameters;

    QString m_selectedParameterName;

    QDialogButtonBox *buttonBox;
};

#endif // STUDY_DIALOG_H
