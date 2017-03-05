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

#include "doe.h"

#include "parameter.h"

void SweepDoE::compute(QSharedPointer<Computation> computation)
{
    // parameters
    bayesopt::Parameters par = initialize_parameters_to_default();
    par.n_init_samples = m_n_samples;
    par.n_iterations = 0;
    par.n_iter_relearn = 10000;
    par.init_method = m_method;
    par.noise = 0;
    par.random_seed = 0;
    par.verbose_level = 0;

    Sweep sweep(this, par);
    sweep.initializeOptimization();

    foreach (QVector<double> variants, m_variants)
    {
        double valueVariants = m_study->doeEvaluatePoint(variants);
        m_values.append(valueVariants);
    }

    Statistics stat(m_values);
    computation->results()->set("doe_min", stat.min(), ComputationResultType_Other);
    computation->results()->set("doe_max", stat.max(), ComputationResultType_Other);
    computation->results()->set("doe_mean", stat.mean(), ComputationResultType_Other);
    computation->results()->set("doe_variance", stat.variance(), ComputationResultType_Other);
    computation->results()->set("doe_stddev", stat.stdDev(), ComputationResultType_Other);
}

SweepDoE::Sweep::Sweep(SweepDoE *doe, bayesopt::Parameters par)
    : bayesopt::ContinuousModel(doe->study()->parameters().count(), par), m_doe(doe)
{
    vectord lowerBound(doe->init().count());
    vectord upperBound(doe->init().count());

    // set bounding box
    for (int i = 0; i < doe->init().count(); i++)
    {
        Parameter parameter = doe->study()->parameters()[i];
        double diff = doe->devFrac() / 100.0 * (parameter.upperBound() - parameter.lowerBound());

        double lb = (doe->init()[i] - diff) > parameter.lowerBound() ? doe->init()[i] - diff : parameter.lowerBound();
        double ub = (doe->init()[i] + diff) < parameter.upperBound() ? doe->init()[i] + diff : parameter.upperBound();
        lowerBound.insert_element(i, lb);
        upperBound.insert_element(i, ub);
    }

    setBoundingBox(lowerBound, upperBound);
}

double SweepDoE::Sweep::evaluateSample(const vectord& x)
{
    QVector<double> var(m_doe->study()->parameters().count());

    for (int i = 0; i < m_doe->study()->parameters().count(); i++)
        var[i] = x[i];

    m_doe->addVariant(var);

    return 0;
}
