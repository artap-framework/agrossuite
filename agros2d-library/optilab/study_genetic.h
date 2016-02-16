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

#ifndef STUDY_GENETIC_H
#define STUDY_GENETIC_H

#include <QWidget>

#include "util.h"
#include "util/enums.h"
#include "study.h"
#include "parameter.h"
/*
class ComputationResults;

class GeneticPopulationRandom : public ComputationSet
{
public:
    GeneticPopulationRandom(QList<Parameter> parameters, int count);
};

class StudyGenetic : public Study
{
public:
    StudyGenetic();

    virtual inline StudyType type() { return StudyType_Genetic; }

    void solve();

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);

protected:
    int m_numberOfPopulation;
    int m_initialpopulationSize;
    double m_selectionRatio;
    double m_elitismRatio;
    double m_crossoverRatio;
    double m_mutationProbability;
    double m_mutationRatio;

    QList<QSharedPointer<Computation> > selectIndividuals(const QList<QSharedPointer<Computation> > &individuals);
    QList<QSharedPointer<Computation> > selectElite(const QList<QSharedPointer<Computation> > &individuals);
    QList<QSharedPointer<Computation> > crossoverAndMutate(const QList<QSharedPointer<Computation> > &individuals);

};
*/
#endif // STUDY_GENETIC_H
