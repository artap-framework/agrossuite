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

#include "study_genetic.h"

#include "study.h"
#include "parameter.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

#include <algorithm>

const QString NUMBEROFPOPULATION = "numberofpopulation";
const QString INITIALPOPULATIONSIZE = "initialpopulationsize";
const QString SELECTIONRATIO = "selectionratio";
const QString ELITISMRATIO = "elitismratio";
const QString MUTATIONPROBABILITY = "mutationprobability";
const QString CROSSOVERRATIO = "crossoverratio";
const QString MUTATIONRATIO = "mutationratio";

const QString POPULATIONS = "populations";
const QString INDIVIDUALS = "individuals";
const QString COMPUTATION = "computation";

GeneticPopulationRandom::GeneticPopulationRandom(QList<Parameter> parameters, int count)
    : ComputationSet()
{
    for (int i = 0; i < count; i++)
    {
         QSharedPointer<Computation> individual = Agros2D::problem()->createComputation(true);
         assert(!individual.isNull());

         foreach (Parameter parameter, parameters)
             individual->config()->setParameter(parameter.name(), parameter.randomValue());

         addComputation(individual);
    }
}

StudyGenetic::StudyGenetic() : Study()
{
    m_numberOfPopulation = 10;
    m_initialpopulationSize = 20;
    m_selectionRatio = 0.8;
    m_elitismRatio = 0.1;
    m_crossoverRatio = 1.2;
    m_mutationProbability = 20;
    m_mutationRatio = 0.1;
}

void StudyGenetic::load(QJsonObject &object)
{
    m_numberOfPopulation = object[NUMBEROFPOPULATION].toInt();
    m_initialpopulationSize = object[INITIALPOPULATIONSIZE].toInt();
    m_selectionRatio = object[SELECTIONRATIO].toDouble();
    m_elitismRatio = object[ELITISMRATIO].toDouble();
    m_mutationProbability = object[MUTATIONPROBABILITY].toDouble();
    m_crossoverRatio = object[CROSSOVERRATIO].toDouble();
    m_mutationRatio = object[MUTATIONRATIO].toDouble();

    Study::load(object);
}

void StudyGenetic::save(QJsonObject &object)
{
    object[NUMBEROFPOPULATION] = m_numberOfPopulation;
    object[INITIALPOPULATIONSIZE] = m_initialpopulationSize;
    object[SELECTIONRATIO] = m_selectionRatio;
    object[ELITISMRATIO] = m_elitismRatio;
    object[MUTATIONPROBABILITY] = m_mutationProbability;
    object[CROSSOVERRATIO] = m_crossoverRatio;
    object[MUTATIONRATIO] = m_mutationRatio;

    Study::save(object);
}

void StudyGenetic::solve()
{
    // create initial population
    ComputationSet currentPopulation = GeneticPopulationRandom(m_parameters, m_initialpopulationSize);

    for (int population = 0; population < m_numberOfPopulation; population++)
    {
        currentPopulation.setName(tr("Population %1").arg(population));
        for (int index = 0; index < currentPopulation.computations().size(); index++)
        {
            QSharedPointer<Computation> individual = currentPopulation.computations().at(index);

            // solve and evaluate
            individual->solve();
            evaluateFunctionals(individual);

            // save results
            individual->saveResults();
        }

        // TODO: multicriterion selector
        assert(m_functionals.size() == 1);
        QString parameterName = m_functionals[0].name();

        // sort individuals
        currentPopulation.sort(parameterName);

        m_computationSets.append(currentPopulation);

        // check stopping criteria
        if (m_computationSets.size() == 15)
            break;

        // selection (sorted population)
        QList<QSharedPointer<Computation> > selectedPopulation = selectIndividuals(currentPopulation.computations());

        // elitism
        QList<QSharedPointer<Computation> > elitePopulation = selectElite(selectedPopulation);

        // crossover and mutation
        QList<QSharedPointer<Computation> > crossBreeds = crossoverAndMutate(selectedPopulation);

        // add new population
        QList<QSharedPointer<Computation> > finalPopulation;
        for (int i = 0; i < elitePopulation.size(); i++)
            finalPopulation.append(elitePopulation[i]);
        for (int i = 0; i < crossBreeds.size(); i++)
            finalPopulation.append(crossBreeds[i]);

        currentPopulation = ComputationSet(finalPopulation);
    }
}

QList<QSharedPointer<Computation> > StudyGenetic::selectIndividuals(const QList<QSharedPointer<Computation> > &individuals)
{
    int numberOfSelected = qMin((int) (m_selectionRatio * individuals.size()), individuals.size());

    QList<QSharedPointer<Computation> > selectedPopulation;
    for (int i = 0; i < numberOfSelected; i++)
        selectedPopulation.append(individuals.at(i));

    return selectedPopulation;
}

QList<QSharedPointer<Computation> > StudyGenetic::selectElite(const QList<QSharedPointer<Computation> > &individuals)
{
    int numberOfElite = qMax((int) (m_elitismRatio * individuals.size()), 1);
    QList<QSharedPointer<Computation> > elitePopulation;
    for (int i = 0; i < numberOfElite; i++)
        elitePopulation.append(individuals[i]);

    return elitePopulation;
}

QList<QSharedPointer<Computation> > StudyGenetic::crossoverAndMutate(const QList<QSharedPointer<Computation> > &individuals)
{
    int numberOfCrossBreeds = qMax((int) (m_crossoverRatio * individuals.size()),
                                   (int) (individuals.size() / 2.0));

    // crossover
    QList<QSharedPointer<Computation> > crossBreeds;
    while (crossBreeds.size() < numberOfCrossBreeds)
    {
        int motherIndex = ((double) qrand() / RAND_MAX * individuals.size());
        int fatherIndex = motherIndex;
        while (fatherIndex == motherIndex)
            fatherIndex = ((double) qrand() / RAND_MAX * individuals.size());

        QSharedPointer<Computation> motherIndividial = individuals[motherIndex];
        QSharedPointer<Computation> fatherIndividial = individuals[fatherIndex];
        QSharedPointer<Computation> sonIndividual = Agros2D::problem()->createComputation(true);

        // random list
        QList<Parameter> parameters = m_parameters;
        std::random_shuffle(parameters.begin(), parameters.end());
        int index = 0;
        foreach (Parameter parameter, parameters)
        {
            if (index % 2 == 0)
                sonIndividual->config()->setParameter(parameter.name(), motherIndividial->config()->parameter(parameter.name()));
            else
                sonIndividual->config()->setParameter(parameter.name(), fatherIndividial->config()->parameter(parameter.name()));

            index++;
        }

        crossBreeds.append(sonIndividual);
    }

    // mutation of crossbreeds
    for (int i = 0; i < numberOfCrossBreeds; i++)
    {
        QSharedPointer<Computation> individual = crossBreeds[i];
        assert(!individual.isNull());

        foreach (Parameter parameter, m_parameters)
        {
            if ((double) qrand() / RAND_MAX * 100 < m_mutationProbability)
            {
                // mutate
                double value = individual->config()->parameter(parameter.name());

                double pert = (double) qrand() / RAND_MAX * (parameter.upperBound() - parameter.lowerBound()) * m_mutationRatio;

                // 50 %
                if (qrand() > RAND_MAX / 2)
                    value += pert;
                else
                    value -= pert;

                // check bounds
                if (value < parameter.lowerBound())
                    value = parameter.lowerBound();
                if (value > parameter.upperBound())
                    value = parameter.upperBound();

                // set new value
                individual->config()->setParameter(parameter.name(), value);
            }
        }
    }

    return crossBreeds;
}
