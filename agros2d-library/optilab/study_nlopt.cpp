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

#include "study_nlopt.h"

#include "study.h"
#include "parameter.h"

#include "gui/lineeditdouble.h"
#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

const QString ALGORITHM = "algorithm";
const QString TOL_REL = "tol_rel";
const QString TOL_ABS = "tol_abs";
const QString N_ITERATIONS = "n_iterations";

class NLoptProblem
{
public:
    NLoptProblem(StudyNLoptAnalysis *study) :  m_study(study),
        opt((nlopt::algorithm) study->value(Study::NLopt_algorithm).toInt(), study->parameters().count())
    {
    }

    double objectiveFunction(const std::vector<double> &x, std::vector<double> &grad, void *data)
    {
        if (m_study->isAborted())
        {
            opt.set_force_stop(1);
            return numeric_limits<double>::max();
        }

        qDebug() << "opt.get_population()" << opt.get_population();

        // computation
        QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(true);
        m_study->addComputation(computation);

        // set parameters
        for (int i = 0; i < m_study->parameters().count(); i++)
        {
            Parameter parameter = m_study->parameters()[i];
            computation->config()->setParameter(parameter.name(), x[i]);
        }

        // evaluate step
        try
        {
            double value = m_study->evaluateStep(computation);
            return value;
        }
        catch (AgrosException &e)
        {
            qDebug() << e.toString();

            opt.set_force_stop(2);
            return numeric_limits<double>::max();
        }
    }

    nlopt::opt opt;

private:
    StudyNLoptAnalysis *m_study;
};

double objFunctionWrapper(const std::vector<double> &x, std::vector<double> &grad, void *data)
{
    NLoptProblem *obj = static_cast<NLoptProblem *>(data);
    return obj->objectiveFunction(x, grad, data);
}

StudyNLoptAnalysis::StudyNLoptAnalysis() : Study()
{
}

void StudyNLoptAnalysis::solve()
{
    m_computationSets.clear();
    m_isSolving = true;

    std::vector<double> initialGuess(m_parameters.count());
    std::vector<double> lowerBound(m_parameters.count());
    std::vector<double> upperBound(m_parameters.count());

    // set bounding box
    for (int i = 0; i < m_parameters.count(); i++)
    {
        Parameter parameter = m_parameters[i];

        lowerBound[i] = parameter.lowerBound();
        upperBound[i] = parameter.upperBound();
        initialGuess[i] = (parameter.lowerBound() + parameter.upperBound()) / 2.0;
    }

    NLoptProblem nLoptProblem(this);
    nLoptProblem.opt.set_min_objective(objFunctionWrapper, &nLoptProblem);
    nLoptProblem.opt.set_lower_bounds(lowerBound);
    nLoptProblem.opt.set_upper_bounds(upperBound);
    nLoptProblem.opt.set_xtol_rel(value(NLopt_tol_rel).toDouble());
    nLoptProblem.opt.set_ftol_rel(value(NLopt_tol_rel).toDouble());
    nLoptProblem.opt.set_xtol_abs(value(NLopt_tol_abs).toDouble());
    nLoptProblem.opt.set_ftol_abs(value(NLopt_tol_abs).toDouble());
    nLoptProblem.opt.set_maxeval(value(NLopt_n_iterations).toInt());

    try
    {
        double minimum = 0.0; // numeric_limits<double>::max();
        nlopt::result result = nLoptProblem.opt.optimize(initialGuess, minimum);

        m_isSolving = false;

        if (result > 0)
        {
            if (result == nlopt::SUCCESS)
                Agros2D::log()->printMessage(tr("NLopt"), tr("Successful"));
            else if (result == nlopt::FTOL_REACHED)
                Agros2D::log()->printMessage(tr("NLopt"), tr("Functional tolerance reached"));
            else if (result == nlopt::XTOL_REACHED)
                Agros2D::log()->printMessage(tr("NLopt"), tr("Parameter tolerance reached"));
            else if (result == nlopt::MAXEVAL_REACHED)
                Agros2D::log()->printMessage(tr("NLopt"), tr("Maximum iterations reached"));

            // sort computations
            // QString parameterName = m_functionals[0].name();
            // m_computationSets.last().sort(parameterName);

            emit solved();
        }
        else
        {
            qDebug() << "err: " << result;
        }
    }
    catch (nlopt::forced_stop &e)
    {
        Agros2D::log()->printError(tr("NLopt"), e.what());
        m_isSolving = false;
    }
}

void StudyNLoptAnalysis::setDefaultValues()
{    
    Study::setDefaultValues();

    m_settingDefault[NLopt_tol_rel] = 1e-3;
    m_settingDefault[NLopt_tol_abs] = 1e-3;
    m_settingDefault[NLopt_n_iterations] = 100;
    m_settingDefault[NLopt_algorithm] = nlopt::LN_BOBYQA;
}

void StudyNLoptAnalysis::setStringKeys()
{
    Study::setStringKeys();

    m_settingKey[NLopt_tol_rel] = "NLopt_tol_rel";
    m_settingKey[NLopt_tol_abs] = "NLopt_tol_abs";
    m_settingKey[NLopt_n_iterations] = "NLopt_n_iterations";
    m_settingKey[NLopt_algorithm] = "NLopt_algorithm";    
}

// *****************************************************************************************************

StudyNLoptAnalysisDialog::StudyNLoptAnalysisDialog(Study *study, QWidget *parent)
    : StudyDialog(study, parent)
{

}

QWidget *StudyNLoptAnalysisDialog::createStudyControls()
{
    txtRelTol = new LineEditDouble(0.0);
    txtAbsTol = new LineEditDouble(0.0);

    txtNIterations = new QSpinBox(this);
    txtNIterations->setMinimum(0);
    txtNIterations->setMaximum(10000);

    cmbAlgorithm = new QComboBox(this);
    cmbAlgorithm->addItem(tr("Global - DIviding RECTangles (locally biased)"), nlopt::GN_DIRECT_L);
    cmbAlgorithm->addItem(tr("Global - Multi-Level Single-Linkage"), nlopt::GN_MLSL);
    cmbAlgorithm->addItem(tr("Global - Controlled Random Search (local mutation)"), nlopt::GN_CRS2_LM);
    cmbAlgorithm->addItem(tr("Global - Improved Stochastic Ranking Evolution Strategy"), nlopt::GN_ISRES);
    cmbAlgorithm->addItem(tr("Global - ESCH (evolutionary algorithm)"), nlopt::GN_ESCH);
    cmbAlgorithm->addItem(tr("Local - BOBYQA"), nlopt::LN_BOBYQA);
    cmbAlgorithm->addItem(tr("Local - COBYLA"), nlopt::LN_COBYLA);
    cmbAlgorithm->addItem(tr("Local - Nelder-Mead Simplex"), nlopt::LN_NELDERMEAD);
    cmbAlgorithm->addItem(tr("Local - Sbplx"), nlopt::LN_SBPLX);

    QGridLayout *layoutInitialization = new QGridLayout(this);
    layoutInitialization->addWidget(new QLabel(tr("Algorithm:")), 0, 0);
    layoutInitialization->addWidget(cmbAlgorithm, 0, 1);
    layoutInitialization->addWidget(new QLabel(tr("Number of iterations:")), 1, 0);
    layoutInitialization->addWidget(txtNIterations, 1, 1);

    QGroupBox *grpInitialization = new QGroupBox(tr("Initialization"), this);
    grpInitialization->setLayout(layoutInitialization);

    QGridLayout *layoutConfig = new QGridLayout(this);
    layoutConfig->addWidget(new QLabel(tr("Relative tolerance:")), 0, 0);
    layoutConfig->addWidget(txtRelTol, 0, 1);
    layoutConfig->addWidget(new QLabel(tr("Absolute tolerance:")), 1, 0);
    layoutConfig->addWidget(txtAbsTol, 1, 1);

    QGroupBox *grpConfig = new QGroupBox(tr("Config"), this);
    grpConfig->setLayout(layoutConfig);

    QVBoxLayout *layoutMain = new QVBoxLayout(this);
    layoutMain->addWidget(grpInitialization);
    layoutMain->addWidget(grpConfig);
    layoutMain->addStretch();

    QWidget *widget = new QWidget(this);
    widget->setLayout(layoutMain);

    return widget;
}

void StudyNLoptAnalysisDialog::load()
{
    txtRelTol->setValue(study()->value(Study::NLopt_tol_rel).toDouble());
    txtAbsTol->setValue(study()->value(Study::NLopt_tol_abs).toDouble());
    txtNIterations->setValue(study()->value(Study::NLopt_n_iterations).toDouble());
    cmbAlgorithm->setCurrentIndex(cmbAlgorithm->findData(study()->value(Study::NLopt_algorithm).toInt()));
}

void StudyNLoptAnalysisDialog::save()
{
    study()->setValue(Study::NLopt_tol_rel, txtRelTol->value());
    study()->setValue(Study::NLopt_tol_abs, txtAbsTol->value());
    study()->setValue(Study::NLopt_n_iterations, txtNIterations->value());
    study()->setValue(Study::NLopt_algorithm, (nlopt::algorithm) cmbAlgorithm->itemData(cmbAlgorithm->currentIndex()).toInt());
}
