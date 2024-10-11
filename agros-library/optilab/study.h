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

#ifndef STUDY_H
#define STUDY_H

#include <algorithm>
#include <iostream>
#include <vector>

#include "util/enums.h"
#include "parameter.h"
#include "goal_function.h"
#include "solver/problem.h"
#include "solver/problem_result.h"

#include <QtCore>

class Computation;
class Study;

class AGROS_LIBRARY_API Statistics
{
public:
    Statistics(const QVector<double> x) : m_values(x), m_min(0.0), m_max(0.0), m_sum(0.0), m_mean(0.0), m_meanError(0.0), m_median(0.0), m_variance(0.0), m_stdDev(0.0)
    {
        if (x.isEmpty())
            return;

        // min and max
        auto mm = std::minmax_element(x.begin(), x.end());
        m_min = *mm.first;
        m_max = *mm.second;

        // sum
        m_sum = std::accumulate(x.begin(), x.end(), 0.0);

        // mean value
        m_mean = m_sum / x.size();

        // mean error
        m_meanError = m_mean / sqrt(x.size());

        // variance
        m_variance = 0.0;
        foreach (double val, x)
            m_variance += (m_mean - val)*(m_mean - val);
        m_variance = m_variance / x.size();

        // standard deviation
        m_stdDev = sqrt(m_variance);

        // median
        m_sortedValues = x;
        std::sort(m_sortedValues.begin(), m_sortedValues.end());
        if (m_sortedValues.size() % 2 == 0)
            m_median = (m_sortedValues[(m_sortedValues.size() / 2) - 1] + m_sortedValues[m_sortedValues.size() / 2]) / 2.0;
        else
            m_median = m_sortedValues[m_sortedValues.size() / 2];
    }

    inline double min() const { return m_min; }
    inline double max() const { return m_max; }
    inline double sum() const { return m_sum; }
    inline double mean() const { return m_mean; }
    inline double meanError() const { return m_meanError; }
    inline double median() const { return m_median; }
    inline double variance() const { return m_variance; }
    inline double stdDev() const { return m_stdDev; }

    inline int const size() { return m_values.size(); }
    inline QVector<double> const values() { return m_values; }
    inline QVector<double> const sortedValues() { return m_sortedValues; }

private:
    QVector<double> m_values;
    QVector<double> m_sortedValues;

    double m_min;
    double m_max;
    double m_sum;
    double m_mean;
    double m_meanError;
    double m_median;
    double m_variance;
    double m_stdDev;
};

class AGROS_LIBRARY_API StatisticsCorrelation
{
public:
    StatisticsCorrelation(const Statistics &x, const Statistics &y) : m_statsX(x), m_statsY(y), m_covariance(0.0), m_correlation(0.0)
    {
        compute();
    }

    StatisticsCorrelation(const QVector<double> x, const QVector<double> y) : m_statsX(Statistics(x)), m_statsY(Statistics(y)), m_covariance(0.0), m_correlation(0.0)
    {
        compute();
    }

    inline double covariance() const { return m_covariance; }
    inline double correlation() const { return m_correlation; }

private:
    Statistics m_statsX;
    Statistics m_statsY;

    void compute()
    {
        assert(m_statsX.size() == m_statsY.size());

        // Covariance
        m_covariance = 0.0;
        for (int i = 0; i < m_statsX.size(); i++)
            m_covariance += (m_statsX.values()[i] - m_statsX.mean()) * (m_statsY.values()[i] - m_statsY.mean());

        m_covariance /= m_statsX.size();

        // Pearson product-moment correlation coefficient
        m_correlation = m_covariance / (m_statsX.stdDev() * m_statsY.stdDev());
    }

    double m_covariance;
    double m_correlation;
};

class AGROS_LIBRARY_API SolutionUncertainty
{
public:
    SolutionUncertainty() : lowerBound(0.0), upperBound(0.0), uncertainty(0.0) {}

    double lowerBound;
    double upperBound;
    double uncertainty;
};

class AGROS_LIBRARY_API ComputationParameterCompare
{
public:
    ComputationParameterCompare(const QString &parameterName) : m_parameterName(parameterName) {}

    inline bool operator() (QSharedPointer<Computation> i, QSharedPointer<Computation> j)
    {
        return (i->results()->items()[m_parameterName] > j->results()->items()[m_parameterName]);
    }

protected:
    QString m_parameterName;
};

class AGROS_LIBRARY_API ComputationSet
{
public:
    ComputationSet(QList<QSharedPointer<Computation> > set = QList<QSharedPointer<Computation> >(), const QString &name = "");
    virtual ~ComputationSet();

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);

    inline QString name() const { return m_name; }
    inline void setName(const QString &name) { m_name = name; }

    inline void addComputation(QSharedPointer<Computation> computation) { m_computations.append(computation); }
    inline void removeComputation(QSharedPointer<Computation> computation) { m_computations.removeAll(computation); }
    QList<QSharedPointer<Computation> > computations() const { return m_computations; }
    QList<QSharedPointer<Computation> > &computations() { return m_computations; }

    void sort(const QString &parameterName);

protected:
    QString m_name;
    QList<QSharedPointer<Computation> > m_computations;
};

using StudyUpdate = std::function<void(QSharedPointer<Computation>, SolutionUncertainty)>;

class AGROS_LIBRARY_API Study : public QObject
{
public:
    enum ResultType
    {
        ResultType_Parameter,
        ResultType_Goal,
        ResultType_Recipe,
        ResultType_Steps
    };

    inline QString resultTypeToStringKey(ResultType type)
    {
        if (type == ResultType::ResultType_Parameter) return "parameter";
        else if (type == ResultType::ResultType_Goal) return "goal";
        else if (type == ResultType::ResultType_Recipe) return "recipe";
        else assert(0);
    }

    inline ResultType resultTypeFromStringKey(const QString &type)
    {
        if (type == "parameter") return ResultType::ResultType_Parameter;
        else if (type == "goal") return ResultType::ResultType_Goal;
        else if (type == "recipe") return ResultType::ResultType_Recipe;
        else assert(0);
    }

    enum Type
    {
        General_ClearSolution,
        General_SolveProblem,

        NLopt_xtol_rel,
        NLopt_xtol_abs,
        NLopt_ftol_rel,
        NLopt_ftol_abs,
        NLopt_n_iterations,
        NLopt_algorithm,

        Sweep_num_samples,
        Sweep_init_method,

        BayesOpt_n_init_samples,
        BayesOpt_n_iterations,
        BayesOpt_n_iter_relearn,
        BayesOpt_init_method,
        BayesOpt_surr_name,
        BayesOpt_surr_noise,
        BayesOpt_l_type,
        BayesOpt_sc_type,

        NSGA2_popsize,
        NSGA2_ngen,
        NSGA2_pcross,
        NSGA2_pmut,
        NSGA2_eta_c,
        NSGA2_eta_m,
        NSGA2_crowdobj,

        OpenGA_algorithm,
        OpenGA_popsize,
        OpenGA_ngen,
        OpenGA_elite_count,
        OpenGA_mutation_rate,
        OpenGA_crossover_fraction,

        Model_Name,
        Model_Description,
    };

    Study(QList<ComputationSet> computations = QList<ComputationSet>());
    virtual ~Study();

    virtual StudyType type() = 0;

    void clear();
    virtual void solve() = 0;

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);

    void addParameter(Parameter parameter) { m_parameters.append(parameter); }
    void removeParameter(const QString &name);
    Parameter &parameter(const QString &name);
    QList<Parameter> &parameters() { return m_parameters; }

    void addGoalFunction(GoalFunction goal) { m_goalFunctions.append(goal); }
    void removeGoalFunction(const QString &name);
    GoalFunction &goal(const QString &name);
    QList<GoalFunction> &goalFunctions() { return m_goalFunctions; }
    bool evaluateGoalFunctions(QSharedPointer<Computation> computation);
    void evaluateStep(QSharedPointer<Computation> computation, SolutionUncertainty solutionUncertainty = SolutionUncertainty());
    double evaluateSingleGoal(QSharedPointer<Computation> computation) const;
    QList<double> evaluateMultiGoal(QSharedPointer<Computation> computation) const;

    QList<QSharedPointer<Computation> > &computations(int index = -1);
    inline QList<ComputationSet> computationSets() const { return m_computationSets; }
    void addComputationSet(const QString &name = "") { m_computationSets.append(ComputationSet(QList<QSharedPointer<Computation> >(), name.isEmpty() ? tr("Set %1").arg(m_computationSets.count() + 1) : name)); }
    void addComputation(QSharedPointer<Computation> computation, bool newComputationSet = false);
    void removeComputation(QSharedPointer<Computation> computation);
    void removeEmptyComputationSets();

    virtual int estimatedNumberOfSteps() const { return 0; }

    QList<QSharedPointer<Computation> > nondominatedSort(QList<ComputationSet> list);

    QVariant variant();

    bool isSolving() const { return m_isSolving; }
    bool isAborted() const { return m_abort; }

    // postprocessor
    QSharedPointer<Computation> findExtreme(ResultType type, const QString &key, bool minimum);

    // config
    QMap<Type, QString> keys() const { return m_settingKey; }
    inline QVariant value(Type type) const { return m_setting[type]; }
    inline void setValue(Type type, int value) {  m_setting[type] = value; }
    inline void setValue(Type type, double value) {  m_setting[type] = value;}
    inline void setValue(Type type, bool value) {  m_setting[type] = value;}
    inline void setValue(Type type, const std::string &value) { setValue(type, QString::fromStdString(value)); }
    inline void setValue(Type type, const QString &value) { m_setting[type] = value;}

    static Study *factory(StudyType type);

    void abortSolving();
    StudyUpdate updateParametersAndFunctionals;

protected:
    QList<Parameter> m_parameters;
    QList<GoalFunction> m_goalFunctions;
    QList<ComputationSet> m_computationSets;

    bool m_isSolving;
    bool m_abort;

    inline QVariant defaultValue(Type type) {  return m_settingDefault[type]; }

    QMap<Type, QVariant> m_setting;
    QMap<Type, QVariant> m_settingDefault;
    QMap<Type, QString> m_settingKey;

    inline QString typeToStringKey(Type type) { return m_settingKey[type]; }
    inline Type stringKeyToType(const QString &key) { return m_settingKey.key(key); }
    inline QStringList stringKeys() { return m_settingKey.values(); }

    virtual void setDefaultValues();
    virtual void setStringKeys();

    bool dominateComputations(const Computation *l, const Computation *r);

    friend class PyStudy;
    friend class PyStudyBayesOpt;
    friend class SwigStudy;
    friend class SwigStudyBayesOpt;
};

class AGROS_LIBRARY_API Studies : public QObject
{
public:
    Studies(QObject *parent = 0);
    ~Studies() {}

    void clear();

    inline QList<Study *> &items() { return m_studies; }
    void addStudy(Study *study);
    void removeStudy(Study *study);

    void removeComputation(QSharedPointer<Computation> computation);

    Study * operator[] (int idx) { return m_studies[idx]; }
    const Study * operator[] (int idx) const { return m_studies[idx]; }

private:
    QList<Study *> m_studies;
};


Q_DECLARE_METATYPE(Study*)

#endif // STUDY_H
