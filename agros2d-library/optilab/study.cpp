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

#include "study.h"
#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"

#include "study_sweep.h"
#include "study_genetic.h"
#include "study_nlopt.h"
#include "study_bayesopt.h"

#include "qcustomplot/qcustomplot.h"

// consts
const QString TYPE = "type";
const QString PARAMETERS = "parameters";
const QString FUNCTIONAL = "functionals";
const QString COMPUTATIONS = "computations";
const QString COMPUTATIONSET = "computationset";
const QString STUDIES = "studies";


LogOptimizationDialog::LogOptimizationDialog(Study *study) : QDialog(QApplication::activeWindow()),
    m_study(study), m_chart(nullptr), m_objectiveGraph(nullptr), m_progress(nullptr)
{
    setModal(true);

    setWindowIcon(icon("run"));
    setWindowTitle(study->name());
    setAttribute(Qt::WA_DeleteOnClose);

    createControls();

    connect(btnAbort, SIGNAL(clicked()), m_study, SLOT(doAbortSolve()));
    connect(m_study, SIGNAL(updateChart()), this, SLOT(updateChart()));
    connect(m_study, SIGNAL(solved()), this, SLOT(solved()));
    connect(m_study, SIGNAL(updateParameters(QList<Parameter>, const Computation *)), this, SLOT(updateParameters(QList<Parameter>, const Computation *)));

    int w = 2.0/3.0 * QApplication::desktop()->screenGeometry().width();
    int h = 2.0/3.0 * QApplication::desktop()->screenGeometry().height();

    setMinimumSize(w, h);
    setMaximumSize(w, h);

    move(QApplication::activeWindow()->pos().x() + (QApplication::activeWindow()->width() - width()) / 2.0,
         QApplication::activeWindow()->pos().y() + (QApplication::activeWindow()->height() - height()) / 2.0);
}

LogOptimizationDialog::~LogOptimizationDialog()
{
}

void LogOptimizationDialog::closeEvent(QCloseEvent *e)
{
    if (m_study->isSolving())
        e->ignore();
}

void LogOptimizationDialog::reject()
{
    if (m_study->isSolving())
        m_study->doAbortSolve();
    else
        close();
}

void LogOptimizationDialog::tryClose()
{
    if (m_study->isSolving())
    {
        Agros2D::log()->printError(tr("Solver"), tr("Stydy is being aborted."));
    }
    else
    {
        close();
    }
}

void LogOptimizationDialog::createControls()
{
    m_logWidget = new LogWidget(this);

#ifdef Q_WS_WIN
    int fontSize = 7;
#endif
#ifdef Q_WS_X11
    int fontSize = 8;
#endif

    QFont fontProgress = font();
    fontProgress.setPointSize(fontSize);

    btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(tryClose()));
    btnClose->setEnabled(false);

    btnAbort = new QPushButton(tr("Abort"));

    QHBoxLayout *layoutStatus = new QHBoxLayout();
    layoutStatus->addStretch();
    layoutStatus->addWidget(btnAbort, 0, Qt::AlignRight);
    layoutStatus->addWidget(btnClose, 0, Qt::AlignRight);

    QPen pen;
    pen.setColor(Qt::darkGray);
    pen.setWidth(2);

    QPen penError;
    penError.setColor(Qt::darkRed);
    penError.setWidth(2);

    QFont fontTitle(font());
    fontTitle.setBold(true);

    QFont fontChart(font());
    fontChart.setPointSize(fontSize);

    // transient
    m_chart = new QCustomPlot(this);
    QCPPlotTitle *timeTitle = new QCPPlotTitle(m_chart, tr("Optimization"));
    timeTitle->setFont(fontTitle);
    m_chart->plotLayout()->insertRow(0);
    m_chart->plotLayout()->addElement(0, 0, timeTitle);
    m_chart->legend->setVisible(true);
    m_chart->legend->setFont(fontChart);

    m_chart->xAxis->setTickLabelFont(fontChart);
    m_chart->xAxis->setLabelFont(fontChart);
    // m_chart->xAxis->setTickStep(1.0);
    m_chart->xAxis->setAutoTickStep(true);
    m_chart->xAxis->setLabel(tr("number of steps"));

    m_chart->yAxis->setScaleType(QCPAxis::stLogarithmic);
    m_chart->yAxis->setTickLabelFont(fontChart);
    m_chart->yAxis->setLabelFont(fontChart);
    m_chart->yAxis->setLabel(tr("objective function"));

    m_objectiveGraph = m_chart->addGraph(m_chart->xAxis, m_chart->yAxis);
    m_objectiveGraph->setLineStyle(QCPGraph::lsLine);
    m_objectiveGraph->setPen(pen);
    m_objectiveGraph->setBrush(QBrush(QColor(0, 0, 255, 20)));
    m_objectiveGraph->setName(tr("objective function"));

    m_progress = new QProgressBar(this);
    m_progress->setMaximum(10000);

    QVBoxLayout *layoutObjective = new QVBoxLayout();
    layoutObjective->addWidget(m_chart, 2);
    layoutObjective->addWidget(m_progress, 1);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutObjective);
    layout->addWidget(m_logWidget);
    // layout->addStretch();
    layout->addLayout(layoutStatus);

    setLayout(layout);
}

void LogOptimizationDialog::updateChart()
{
    QVector<double> steps;
    QVector<double> objective;

    for (int i = 0; i < m_study->computationSets().count(); i++)
    {
        QList<QSharedPointer<Computation> > computations = m_study->computationSets()[i].computations();

        for (int j = 0; j < computations.count(); j++)
        {
            steps.append(steps.count() + 1);

            // TODO: more functionals !!!
            assert(m_study->functionals().size() == 1);
            QString parameterName = m_study->functionals()[0].name();

            double value = computations[j]->results()->resultValue(parameterName);
            objective.append(value);
        }
    }

    m_objectiveGraph->setData(steps, objective);
    m_chart->rescaleAxes();
    m_chart->replot(QCustomPlot::rpImmediate);

    QApplication::processEvents();
}

void LogOptimizationDialog::solved()
{
    btnAbort->setEnabled(false);
    btnClose->setEnabled(true);
}

void LogOptimizationDialog::updateParameters(QList<Parameter> parameters, const Computation *computation)
{
    QString params = "";
    foreach (Parameter parameter, parameters)
    {
        params += QString("%1 = %2, ").arg(parameter.name()).arg(computation->config()->parameter(parameter.name()));
    }
    if (params.size() > 0)
        params = params.left(params.size() - 2);

    QString res = "";
    foreach (QString name, computation->results()->results().keys())
    {
        res += QString("%1 = %2, ").arg(name).arg(computation->results()->resultValue(name));
    }
    if (res.size() > 0)
        res = res.left(res.size() - 2);

    Agros2D::log()->printMessage(tr("Study"), tr("Parameters: %1, results: %2").arg(params).arg(res));
}

ComputationSet::ComputationSet(QList<QSharedPointer<Computation> > set, const QString &name)
    : m_computations(set), m_name(name)
{
}

ComputationSet::~ComputationSet()
{
    m_computations.clear();
}

void ComputationSet::load(QJsonObject &object)
{
    QJsonArray computationSetJson = object[COMPUTATIONSET].toArray();
    for (int i = 0; i < computationSetJson.size(); i++)
    {
        QMap<QString, QSharedPointer<Computation> > computations = Agros2D::computations();
        QSharedPointer<Computation> computation = computations[computationSetJson[i].toString()];
        m_computations.append(computation);
    }
}

void ComputationSet::save(QJsonObject &object)
{
    QJsonArray computationSetJson;
    foreach (QSharedPointer<Computation> computation, m_computations)
        computationSetJson.append(computation->problemDir());
    object[COMPUTATIONSET] = computationSetJson;
}

void ComputationSet::sort(const QString &parameterName)
{
    // sort individuals
    std::sort(m_computations.begin(), m_computations.end(), ComputationParameterCompare(parameterName));
}

// *****************************************************************************************************************

Study::Study(QList<ComputationSet> computations)
    : m_computationSets(computations), m_name(""), m_abort(false), m_isSolving(false)
{
}

Study::~Study()
{
    clear();
}

void Study::clear()
{
    m_parameters.clear();
    m_functionals.clear();
    m_computationSets.clear();
}

void Study::load(QJsonObject &object)
{
    // parameters
    QJsonArray parametersJson = object[PARAMETERS].toArray();
    for (int i = 0; i < parametersJson.size(); i++)
    {
        QJsonObject parameterJson = parametersJson[i].toObject();
        Parameter parameter;
        parameter.load(parameterJson);
        m_parameters.append(parameter);
    }

    // functionals
    QJsonArray functionalsJson = object[FUNCTIONAL].toArray();
    for (int i = 0; i < functionalsJson.size(); i++)
    {
        QJsonObject parameterJson = functionalsJson[i].toObject();
        Functional functional;
        functional.load(parameterJson);
        m_functionals.append(functional);
    }

    // computations
    QJsonArray computationsJson = object[COMPUTATIONS].toArray();
    for (int i = 0; i < computationsJson.size(); i++)
    {
        QJsonObject computationSetJson = computationsJson[i].toObject();
        ComputationSet computationSet;
        computationSet.load(computationSetJson);
        m_computationSets.append(computationSet);
    }
}

void Study::save(QJsonObject &object)
{
    // parameters
    QJsonArray parametersJson;
    foreach (Parameter parameter, m_parameters)
    {
        QJsonObject parameterJson;
        parameter.save(parameterJson);
        parametersJson.append(parameterJson);
    }
    object[PARAMETERS] = parametersJson;

    // functionals
    QJsonArray functionalsJson;
    foreach (Functional functional, m_functionals)
    {
        QJsonObject functionalJson;
        functional.save(functionalJson);
        functionalsJson.append(functionalJson);
    }
    object[FUNCTIONAL] = functionalsJson;

    // computations
    QJsonArray computationsJson;
    foreach (ComputationSet computationSet, m_computationSets)
    {
        QJsonObject computationSetJson;
        computationSet.save(computationSetJson);
        computationsJson.append(computationSetJson);
    }
    object[COMPUTATIONS] = computationsJson;
}

bool Study::evaluateFunctionals(QSharedPointer<Computation> computation)
{
    bool successfulRun = false;
    currentPythonEngine()->useTemporaryDict();
    foreach (Functional functional, m_functionals)
        successfulRun = functional.evaluateExpression(computation);
    currentPythonEngine()->useGlobalDict();

    return successfulRun;
}

void Study::addComputation(QSharedPointer<Computation> computation, bool newComputationSet)
{
    if (m_computationSets.isEmpty() || newComputationSet)
        m_computationSets.append(ComputationSet());

    m_computationSets.last().addComputation(computation);
}

void Study::fillTreeView(QTreeWidget *trvComputations)
{
    for (int i = 0; i < m_computationSets.size(); i++)
    {
        QTreeWidgetItem *itemComputationSet = new QTreeWidgetItem(trvComputations);

        QString computationSetName= tr("Set %1").arg(i + 1);
        if (!m_computationSets[i].name().isEmpty())
            computationSetName = m_computationSets[i].name();

        itemComputationSet->setText(0, tr("%1 (%2 computations)").arg(computationSetName).arg(m_computationSets[i].computations().size()));
        if (i == m_computationSets.size() - 1)
            itemComputationSet->setExpanded(true);

        foreach (QSharedPointer<Computation> computation, m_computationSets[i].computations())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(itemComputationSet);
            item->setText(0, computation->problemDir());
            item->setText(1, QString("%1 / %2").arg(computation->isSolved() ? tr("solved") : tr("not solved")).arg(computation->results()->hasResults() ? tr("results") : tr("no results")));
            item->setData(0, Qt::UserRole, computation->problemDir());
        }
    }
}

QList<QSharedPointer<Computation> > &Study::computations(int index)
{
    if (index == -1)
        return m_computationSets.last().computations();
    else
        return m_computationSets[index].computations();
}

QVariant Study::variant()
{
    QVariant v;
    v.setValue(this);
    return v;
}

void Study::doAbortSolve()
{
    m_abort = true;
    Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Aborting calculation..."));
}

// *****************************************************************************************************************

Studies::Studies(QObject *parent) : QObject(parent)
{
    //connect(this, SIGNAL(invalidated()), this, SLOT(save())); TODO: only GUI
}

void Studies::addStudy(Study *study)
{
    m_studies.append(study);
    emit invalidated();
}

void Studies::removeStudy(Study *study)
{
    m_studies.removeOne(study);
    emit invalidated();
}

void Studies::clear()
{
    for (int i = 0; i < m_studies.size(); i++)
        delete m_studies[i];
    m_studies.clear();
    emit invalidated();
}

bool Studies::load(const QString &fileName)
{
    blockSignals(true);
    clear();

    QFile file(fileName);
    if (!file.exists())
        return true; // no study

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't open study file.");
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject rootJson = doc.object();

    QJsonArray studiesJson = rootJson[STUDIES].toArray();
    for (int i = 0; i < studiesJson.size(); i++)
    {
        QJsonObject studyJson = studiesJson[i].toObject();
        StudyType type = studyTypeFromStringKey(studyJson[TYPE].toString());

        Study *study = nullptr;
        if (type == StudyType_SweepAnalysis)
            study = new StudySweepAnalysis();
        else if (type == StudyType_Genetic)
            study = new StudyGenetic();
        else if (type == StudyType_BayesOptAnalysis)
            study = new StudyBayesOptAnalysis();
        else if (type == StudyType_NLoptAnalysis)
            study = new StudyNLoptAnalysis();
        else
            assert(0);

        study->load(studyJson);
        m_studies.append(study);
    }

    blockSignals(false);
    emit invalidated();

    return true;
}

bool Studies::save(const QString &fileName)
{
    if (m_studies.isEmpty())
    {
        if (QFile::exists(fileName))
            QFile::remove(fileName);

        return true;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning("Couldn't open studies file.");
        return false;
    }

    QJsonObject rootJson;
    QJsonArray studiesJson;

    foreach (Study *study, m_studies)
    {
        QJsonObject studyJson;
        studyJson[TYPE] = studyTypeToStringKey(study->type());
        study->save(studyJson);
        studiesJson.append(studyJson);
    }
    rootJson[STUDIES] = studiesJson;

    // save
    QJsonDocument doc(rootJson);
    file.write(doc.toJson());

    return true;
}
