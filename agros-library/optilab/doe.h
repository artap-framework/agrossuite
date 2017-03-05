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

#ifndef DOE_H
#define DOE_H

#include "util/enums.h"
#include "study.h"

#include "bayesopt/bayesopt.hpp"

class DoE
{
public:
    DoE(Study *study, QVector<double> init, double devFrac = 0.001) : m_study(study), m_init(init), m_devFrac(devFrac) {}

    Study *study() const { return m_study; }
    QVector<double> init() const { return m_init; }
    double devFrac() const { return m_devFrac; }

    virtual void compute(QSharedPointer<Computation> computation) = 0;

    void addVariant(QVector<double> var) { m_variants.append(var); }
    void addValue(double value) { m_values.append(value); }
    QVector<QVector<double> > variants() const { return m_variants; }

protected:
    Study *m_study;
    QVector<QVector<double> > m_variants;

    QVector<double> m_init;
    double m_devFrac;

    QVector<double> m_values;
};

class SweepDoE : public DoE
{
public:
    SweepDoE(Study *study, const QVector<double> init, double devFrac)
        : DoE(study, init, devFrac), m_n_samples(1), m_method(1)
    {
    }

    virtual void compute(QSharedPointer<Computation> computation);

    void setNSamples(int n_samples) { m_n_samples = n_samples; }
    void setMethod(int method) { m_method = method; }

protected:
    class Sweep : public bayesopt::ContinuousModel
    {
    public:
        Sweep(SweepDoE *doe, bayesopt::Parameters par);

        double evaluateSample(const vectord& x);
        bool checkReachability(const vectord &query) { return true; }

    protected:
        SweepDoE *m_doe;
    };

private:
    int m_n_samples;
    int m_method;
};

#endif // DOE_H
