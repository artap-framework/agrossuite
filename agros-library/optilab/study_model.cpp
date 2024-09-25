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

#include "study_model.h"

#include "study.h"
#include "parameter.h"
#include "logview.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"

StudyModel::StudyModel() : Study()
{
}

void StudyModel::solve()
{
    m_computationSets.clear();
    m_isSolving = true;

    addComputationSet(tr("Model"));

	// computation
	QSharedPointer<Computation> computation = Agros::problem()->createComputation(true);

	// set parameters
	// for (int i = 0; i < this->parameters().count(); i++)
	// {
	// 	Parameter parameter = this->parameters()[i];
	// 	// computation->config()->parameters()->set(parameter.name(), x[i]);
	// }

	// evaluate step
	try
	{
		this->evaluateStep(computation);
		double value = this->evaluateSingleGoal(computation);
		qInfo() << "evaluateSingleGoal" << value;

		if (this->value(Study::General_ClearSolution).toBool())
			computation->clearSolution();

		// add computation
		this->addComputation(computation);
	}
	catch (AgrosSolverException &e)
	{
		qDebug() << e.toString();
	}

	// remove empty computation sets
	this->removeEmptyComputationSets();

	m_isSolving = false;
}

void StudyModel::setDefaultValues()
{
    Study::setDefaultValues();

	// m_settingDefault[OpenGA_popsize] = 10;
}

void StudyModel::setStringKeys()
{
    Study::setStringKeys();

    // m_settingKey[OpenGA_algorithm] = "OpenGA_algorithm";
}

