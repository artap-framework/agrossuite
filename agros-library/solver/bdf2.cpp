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


#include "bdf2.h"

void BDF2Table::setOrderAndPreviousSteps(int order, QList<double> previousStepsLengths)
{
    this->m_order = order;

    int numSteps = previousStepsLengths.length();
    assert(numSteps >= m_order - 1);   

    for (int i = 0; i < order - 1; i++)
    {
        th[i] = previousStepsLengths[numSteps - 1 - i] / previousStepsLengths[numSteps - 2 - i];
        // cout << i << ": " << " order " << order << " th " << th[i] << " " << numSteps << endl;
    }

    recalculate();
}

void BDF2ATable::recalculate()
{
    if (m_order == 1)
    {
        // matrix coef
        m_alpha[0] = 1.;
        // vector coefs
        m_alpha[1] = -1.;
    }
    else if (m_order == 2)
    {
        double t0 = th[0];

        // matrix coef
        m_alpha[0] = ((2*t0 + 1) / (t0 + 1));
        // vector coefs
        m_alpha[1] = (-t0 - 1) ;
        m_alpha[2] = (t0 * t0 / (t0 + 1));
    }
    else if (m_order == 3)
    {
        double t0 = th[0];
        double t1 = th[1];

        // matrix coef
        m_alpha[0] = ((4*t0*t1 + 3*t0*t0*t1 + t1 + 1 + 2*t0) / (t0 + 2*t0*t1 + 1 + t1 + t0*t0*t1));
        // vector coefs
        m_alpha[1] = (-(t0 + 2*t0*t1 + 1 + t1 + t0*t0*t1) / (1+t1));
        m_alpha[2] = ((t1 + t0*t1 + 1) * t0*t0 / (1+t0));
        m_alpha[3] = ( -(1+t0) * t0*t0 * t1*t1*t1 / (t0*t1*t1 + t0*t1 + 2*t1 + 1 + t1*t1));
    }
    else
        assert(0);
}


const double a = 500;

double f(double x)
{
    return (exp(a*x) - 1) / (exp(a) - 1);
}

double df(double x)
{
    return a * exp(a*x) / (exp(a) - 1);
}

double BDF2Table::testCalcValue(double step, QList<double> values, double fVal)
{
    double result = fVal;

    for(int i = 1; i <= m_order; i++)
    {
        result -= m_alpha[i] * values[values.size() - i];
    }

    result /= m_alpha[0];

    return result;
}

void BDF2Table::test(bool varyLength)
{
    BDF2ATable tableA;

    double results[3][4];

    int numStepsArray[] = {100, 1000, 10000, 100000};

    for(int order = 1; order <=3; order++)
    {
        for(int numStepsIdx = 0; numStepsIdx < 4; numStepsIdx++)
        {
            int numSteps = numStepsArray[numStepsIdx];
            double constantStepLen = 1./double(numSteps);

            if(varyLength)
                numSteps = 3*numSteps/2;

            QList<double> previousSteps;

            QList<double>  valsA;
            valsA.push_back(f(0));
            double actTime = 0;
            int realOrder = -1;
            for(int s = 0; s < numSteps; s++)
            {
                double actualStepLen = constantStepLen;
                if(varyLength && (s % 3))
                    actualStepLen = constantStepLen/2.;

                previousSteps.push_back(actualStepLen);

                if(s == 0)
                    realOrder = 1;

                if((s == 1) && (order >= 2))
                    realOrder = 2;

                if((s == 2) && (order >= 3))
                    realOrder = 3;

                tableA.setOrderAndPreviousSteps(realOrder, previousSteps);
                actTime += actualStepLen;

                double valA = tableA.testCalcValue(actualStepLen, valsA, df(actTime));
                valsA.push_back(valA);
            }

            cout << "actTime " << actTime << ", step " << numSteps << endl;
            assert(fabs(actTime-1.) < 0.000000001);

            double errorA = fabs(valsA.last() - f(1));
            cout << "order " << order << ", step " << 1./double(numSteps) << (varyLength ? " approx(alternate)" : " exact") << ", error " << errorA << endl;
            results[order-1][numStepsIdx] = errorA;
        }
    }
    cout << "errors = [";
    for(int ord = 0; ord < 3; ord++)
    {
        cout << "[";
        for(int st = 0; st < 4; st++)
        {
            cout << results[ord][st];
            if(st < 3)
                cout << ",";
        }
        cout << "]";
        if(ord < 2)
            cout << ",";
    }
    cout << "]"<< endl << endl;
}
