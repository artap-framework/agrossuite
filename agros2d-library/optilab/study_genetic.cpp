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

const QString INITIALPOPULATIONSIZE = "initialpopulationsize";
const QString SELECTIONRATIO = "selectionratio";
const QString ELITISMRATIO = "elitismratio";
const QString MUTATIONPROBABILITY = "mutationprobability";
const QString CROSSOVERRATIO = "crossoverratio";
const QString MUTATIONRATIO = "mutationratio";

const QString POPULATIONS = "populations";
const QString INDIVIDUALS = "individuals";
const QString COMPUTATION = "computation";

class GeneticIndividualCompare
{
public:
    GeneticIndividualCompare(const QString &parameterName) : m_parameterName(parameterName) {}

    inline bool operator() (const GeneticIndividual &i, const GeneticIndividual &j)
    {
        return (i.computation()->result()->results()[m_parameterName] < j.computation()->result()->results()[m_parameterName]);
    }

protected:
    QString m_parameterName;
};

void GeneticIndividual::load(QJsonObject &object)
{
    QMap<QString, QSharedPointer<Computation> > computations = Agros2D::computations();
    m_computation = computations[object[COMPUTATION].toString()];
}

void GeneticIndividual::save(QJsonObject &object)
{
    object[COMPUTATION] = m_computation->problemDir();
}

void GeneticPopulation::load(QJsonObject &object)
{
    // individuals
    QJsonArray individualsJson = object[INDIVIDUALS].toArray();
    for (int i = 0; i < individualsJson.size(); i++)
    {
        QJsonObject individualJson = individualsJson[i].toObject();

        GeneticIndividual individual;
        individual.load(individualJson);

        m_individuals.append(individual);
    }
}

void GeneticPopulation::save(QJsonObject &object)
{
    // individuals
    QJsonArray individualsJson;
    foreach (GeneticIndividual individual, m_individuals)
    {
        QJsonObject individualJson;
        individual.save(individualJson);

        individualsJson.append(individualJson);
    }
    object[INDIVIDUALS] = individualsJson;
}

StudyGenetic::StudyGenetic() : Study()
{
    m_initialpopulationSize = 20;

    m_selectionRatio = 0.8;
    m_elitismRatio = 0.1;
    m_crossoverRatio = 1.2;
    m_mutationProbability = 20;
    m_mutationRatio = 0.1;
}

void StudyGenetic::load(QJsonObject &object)
{
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
    object[INITIALPOPULATIONSIZE] = m_initialpopulationSize;
    object[SELECTIONRATIO] = m_selectionRatio;
    object[ELITISMRATIO] = m_elitismRatio;
    object[MUTATIONPROBABILITY] = m_mutationProbability;
    object[CROSSOVERRATIO] = m_crossoverRatio;
    object[MUTATIONRATIO] = m_mutationRatio;

    // populations
    QJsonArray populationsJson;
    foreach (GeneticPopulation population, m_populations)
    {
        QJsonObject populationJson;
        population.save(populationJson);

        populationsJson.append(populationJson);
    }
    object[POPULATIONS] = populationsJson;

    Study::save(object);
}

void StudyGenetic::solve()
{
    // create initial population
    GeneticPopulationRandom initialPopulation(m_parameters, m_initialpopulationSize);
    GeneticPopulation currentPopulation = initialPopulation;

    while (true)
    {
        for (int index = 0; index < currentPopulation.individuals().size(); index++)
        {
            GeneticIndividual individual = currentPopulation.individuals().at(index);

            // set parameters
            foreach (QString key, individual.values().keys())
            {
                Agros2D::problem()->config()->setParameter(key, individual.values()[key]);
            }

            // create computation
            QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(true, false);

            // store computation in individual
            currentPopulation.individuals()[index].setComputation(computation);

            // solve
            // computation->solve();

            // evaluate expressions
            foreach (Functional functional, m_functionals)
            {
                bool successfulRun = functional.evaluateExpression(computation);
                // qDebug() << successfulRun;
            }

            // global dict
            currentPythonEngine()->useGlobalDict();

            // save results
            computation->saveResults();
        }

        // TODO: multicriterion selector
        assert(m_functionals.size() == 1);
        QString parameterName = m_functionals[0].name();

        // sort individuals
        std::sort(currentPopulation.individuals().begin(), currentPopulation.individuals().end(), GeneticIndividualCompare(parameterName));

        m_populations.append(currentPopulation);

        // check stopping criteria
        if (m_populations.size() == 15)
            break;

        // create new population

        // selection (sorted population)
        QList<GeneticIndividual> selectedPopulation = selectIndividuals(currentPopulation.individuals());

        // elitism
        QList<GeneticIndividual> elitePopulation = selectElite(selectedPopulation);

        // crossover and mutation
        QList<GeneticIndividual> crossBreeds = crossoverAndMutate(selectedPopulation);

        // add new population
        QList<GeneticIndividual> finalPopulation;
        for (int i = 0; i < elitePopulation.size(); i++)
            finalPopulation.append(elitePopulation[i]);
        for (int i = 0; i < crossBreeds.size(); i++)
            finalPopulation.append(crossBreeds[i]);

        currentPopulation = GeneticPopulation(finalPopulation);
    }
}

QList<GeneticIndividual> StudyGenetic::selectIndividuals(const QList<GeneticIndividual> &individuals)
{
    int numberOfSelected = qMin((int) (m_selectionRatio * individuals.size()), individuals.size());

    QList<GeneticIndividual> selectedPopulation;
    for (int i = 0; i < numberOfSelected; i++)
        selectedPopulation.append(individuals.at(i));

    return selectedPopulation;
}

QList<GeneticIndividual> StudyGenetic::selectElite(const QList<GeneticIndividual> &individuals)
{
    int numberOfElite = qMax((int) (m_elitismRatio * individuals.size()), 1);
    QList<GeneticIndividual> elitePopulation;
    for (int i = 0; i < numberOfElite; i++)
        elitePopulation.append(individuals[i]);

    return elitePopulation;
}

QList<GeneticIndividual> StudyGenetic::crossoverAndMutate(const QList<GeneticIndividual> &individuals)
{
    int numberOfCrossBreeds = qMax((int) (m_crossoverRatio * individuals.size()),
                                   (int) (individuals.size() / 2.0));

    // crossover
    QList<GeneticIndividual> crossBreeds;
    while (crossBreeds.size() < numberOfCrossBreeds)
    {
        int motherIndex = ((double) qrand() / RAND_MAX * individuals.size());
        int fatherIndex = motherIndex;
        while (fatherIndex == motherIndex)
            fatherIndex = ((double) qrand() / RAND_MAX * individuals.size());

        GeneticIndividual motherIndividial = individuals[motherIndex];
        GeneticIndividual fatherIndividial = individuals[fatherIndex];
        GeneticIndividual sonIndividual;

        // random list
        QList<Parameter> parameters = m_parameters;
        std::random_shuffle(parameters.begin(), parameters.end());
        int index = 0;
        foreach (Parameter parameter, parameters)
        {
            if (index % 2 == 0)
                sonIndividual.values()[parameter.name()] = motherIndividial.values()[parameter.name()];
            else
                sonIndividual.values()[parameter.name()] = fatherIndividial.values()[parameter.name()];

            index++;
        }

        crossBreeds.append(sonIndividual);
    }

    // mutation of crossbreeds
    for (int i = 0; i < numberOfCrossBreeds; i++)
        crossBreeds[i].mutate(m_parameters, m_mutationProbability, m_mutationRatio);

    return crossBreeds;
}

// gui
void StudyGenetic::fillTreeView(QTreeWidget *trvComputations)
{
    for (int i = 0; i < m_populations.size(); i++)
    {
        QTreeWidgetItem *itemPopulation = new QTreeWidgetItem(trvComputations);
        itemPopulation->setText(0, tr("Population %1 (%2 items)").arg(i).arg(m_populations[i].individuals().size()));
        itemPopulation->setExpanded(true);

        foreach (GeneticIndividual individual, m_populations[i].individuals())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(itemPopulation);
            item->setText(0, individual.computation()->problemDir());
            item->setText(1, QString("%1 / %2").arg(individual.computation()->isSolved() ? tr("solved") : tr("not solved")).arg(individual.computation()->result()->hasResults() ? tr("results") : tr("no results")));
            item->setData(0, Qt::UserRole, individual.computation()->problemDir());
        }
    }
}
