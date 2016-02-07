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

#include "study_bayesopt.h"

#include "study.h"
#include "parameter.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

StudyBayesOptAnalysis::StudyBayesOptAnalysis() : Study()
{
}

void StudyBayesOptAnalysis::solve()
{
    // parameter space
    ParameterSpace space = ParameterSpace(m_parameters);
    space.random(10);

    foreach (StringToDoubleMap set, space.sets())
    {
        // computation
        QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(true);
        addComputation(computation);

        foreach (QString parameter, set.keys())
            computation->config()->setParameter(parameter, set[parameter]);

        // solve and evaluate
        computation->solve();
        evaluateFunctionals(computation);

        computation->saveResults();
        //qDebug() << computation->config()->parameters();
        //qDebug() << computation->results()->results();
    }
}
