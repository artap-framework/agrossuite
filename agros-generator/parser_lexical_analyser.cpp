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

#include "util/constants.h"
#include "solver/weak_form.h"
#include "solver/module.h"
#include "solver/coupling.h"
#include "parser/lex.h"
#include "generator.h"
#include "parser.h"
#include "parser_instance.h"

//********************************************************************************************************
// In this file are all methods of Parser and ParserInstance, that handle creation of lexical analyser and dictionary of symbols
// defining symbols and creation of lexical analyser is done in static methods of Parser
// creation of dictionary is done in ParserInstance
// This is probably not ideal state
//********************************************************************************************************

void ParserInstance::addBasicWeakformTokens(ParserModuleInfo pmi)
{
    // coordinates
    if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
    {
        m_dict["x"] = "p[0]"; // "e->x[i]";
        m_dict["y"] = "p[1]"; // "e->y[i]";
        m_dict["tx"] = "e->tx[i]";
        m_dict["ty"] = "e->ty[i]";
        m_dict["nx"] = "e->nx[i]";
        m_dict["ny"] = "e->ny[i]";
    }
    else
    {
        m_dict["r"] = "p[0]"; // "e->x[i]";
        m_dict["z"] = "p[1]"; // "e->y[i]";
        m_dict["tr"] = "e->tx[i]";
        m_dict["tz"] = "e->ty[i]";
        m_dict["nr"] = "e->nx[i]";
        m_dict["nz"] = "e->ny[i]";
    }

    // constants
    m_dict["PI"] = "M_PI";
    m_dict["f"] = "frequency";

    // area of a label
    // assumes, that this->getAreas has allways only one component (it is true at the moment, since in Agros we create one form for each label)
    m_dict["area"] = "this->markerVolume()";

    // functions
    // scalar field
    m_dict["uval"] = pmi.isSurface ? "cache.shape_face_value[face][j][q_point]" : "cache.shape_value[j][q_point]"; // "u->val[i]";
    m_dict["vval"] = pmi.isSurface ? "cache.shape_face_value[face][i][q_point]" : "cache.shape_value[i][q_point]"; // "v->val[i]";

    // vector field
    m_dict["uval0"] = "u->val0[i]";
    m_dict["uval1"] = "u->val1[i]";
    m_dict["ucurl"] = "u->curl[i]";
    m_dict["vval0"] = "v->val0[i]";
    m_dict["vval1"] = "v->val1[i]";
    m_dict["vcurl"] = "v->curl[i]";
    m_dict["upcurl"] = "0"; // "u_ext[this->j]->curl[i]";

    if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
    {
        // scalar field
        m_dict["udx"] = "cache.shape_grad[j][q_point][0]"; // u->dx[i]";
        m_dict["vdx"] = "cache.shape_grad[i][q_point][0]"; // "v->dx[i]";
        m_dict["udy"] = "cache.shape_grad[j][q_point][1]"; // "u->dy[i]";
        m_dict["vdy"] = "cache.shape_grad[i][q_point][1]"; // "v->dy[i]";
    }
    else
    {
        // scalar field
        m_dict["udr"] = "cache.shape_grad[j][q_point][0]"; // u->dx[i]";
        m_dict["vdr"] = "cache.shape_grad[i][q_point][0]"; // "v->dx[i]";
        m_dict["udz"] = "cache.shape_grad[j][q_point][1]"; // "u->dy[i]";
        m_dict["vdz"] = "cache.shape_grad[i][q_point][1]"; // "v->dy[i]";
    }

    // previous solution (residual form in Newton's method)
    for (int i = 1; i < pmi.numSolutions + 1; i++)
    {
        m_dict[QString("upval%1").arg(i)] = pmi.isSurface ? QString("cache.solution_value_previous_face[face][q_point][%1]").arg(i-1)
                                                  : QString("cache.solution_value_previous[q_point][%1]").arg(i-1); // "u_ext[this->j]->val[i]";

        if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
        {
            // scalar field
            m_dict[QString("updx%1").arg(i)] = QString("cache.solution_grad_previous[q_point][%1][0]").arg(i-1); // "u_ext[this->j]->dx[i]";
            m_dict[QString("updy%1").arg(i)] = QString("cache.solution_grad_previous[q_point][%1][1]").arg(i-1); // "u_ext[this->j]->dy[i]";
        }
        else
        {
            // scalar field
            m_dict[QString("updr%1").arg(i)] = QString("cache.solution_grad_previous[q_point][%1][0]").arg(i-1); // "u_ext[this->j]->dx[i]";
            m_dict[QString("updz%1").arg(i)] = QString("cache.solution_grad_previous[q_point][%1][1]").arg(i-1); // "u_ext[this->j]->dy[i]";
        }
    }
}

void ParserInstance::addConstants(ParserModuleInfo pmiField)
{
    foreach (XMLModule::constant cnst, pmiField.constants.constant())
    {
        m_dict[QString::fromStdString(cnst.id())] = QString::number(cnst.value());
    }
}

void ParserInstance::addCouplingWeakformTokens(int numSourceSolutions)
{
    for (int i = 1; i < numSourceSolutions + 1; i++)
    {
        foreach (QString moduleId, Agros2DGenerator::availableModules())
        {
            m_dict[moduleId + QString("%1").arg(i)] = moduleId + QString("_value[q_point][%1]").arg(i-1);
            if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
            {
                m_dict[moduleId + QString("%1dx").arg(i)] = moduleId + QString("_grad[q_point][%1][0]").arg(i-1);
                m_dict[moduleId + QString("%1dy").arg(i)] = moduleId + QString("_grad[q_point][%1][1]").arg(i-1);
            }
            else
            {
                m_dict[moduleId + QString("%1dr").arg(i)] = moduleId + QString("_grad[q_point][%1][0]").arg(i-1);
                m_dict[moduleId + QString("%1dz").arg(i)] = moduleId + QString("_grad[q_point][%1][1]").arg(i-1);
            }
        }
    }
}


void ParserInstance::addPreviousSolWeakform(int numSolutions)
{
    for (int i = 1; i < numSolutions + 1; i++)
    {
        m_dict[QString("value%1").arg(i)] = QString("cache.solution_value_previous[q_point][%1]").arg(i-1); //QString("u_ext[%1 + offset.forms]->val[i]").arg(i-1);

        if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
        {
            m_dict[QString("dx%1").arg(i)] = QString("cache.solution_grad_previous[q_point][%1][0]").arg(i-1); //QString("u_ext[%1 + offset.forms]->dx[i]").arg(i-1);
            m_dict[QString("dy%1").arg(i)] = QString("cache.solution_grad_previous[q_point][%1][1]").arg(i-1); //QString("u_ext[%1 + offset.forms]->dy[i]").arg(i-1);
        }
        else
        {
            m_dict[QString("dr%1").arg(i)] = QString("cache.solution_grad_previous[q_point][%1][0]").arg(i-1); //QString("u_ext[%1 + offset.forms]->dx[i]").arg(i-1);
            m_dict[QString("dz%1").arg(i)] = QString("cache.solution_grad_previous[q_point][%1][1]").arg(i-1); //QString("u_ext[%1 + offset.forms]->dy[i]").arg(i-1);
        }
    }
}


void ParserInstance::addPreviousSolErrorCalculation()
{
    for (int i = 1; i < m_parserModuleInfo.numSolutions + 1; i++)
    {
        m_dict[QString("value%1").arg(i)] = QString("cache.solution_value_previous[q_point][%1]").arg(i-1); // u->val[i]

        if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
        {
            m_dict[QString("dx%1").arg(i)] = QString("cache.solution_grad_previous[q_point][%1][0]").arg(i-1); // u->dx[i]
            m_dict[QString("dy%1").arg(i)] = QString("cache.solution_grad_previous[q_point][%1][1]").arg(i-1); // u->dy[i]
        }
        else
        {
            m_dict[QString("dr%1").arg(i)] = QString("cache.solution_grad_previous[q_point][%1][0]").arg(i-1); // u->dx[i]
            m_dict[QString("dz%1").arg(i)] = QString("cache.solution_grad_previous[q_point][%1][1]").arg(i-1); // u->dy[i]
        }
    }
}

// for linearized variant adding ext with index of allready solved field
// used for nonlinear source term in the case of weak coupling
void ParserInstance::addPreviousSolLinearizeDependence()
{
    for (int i = 1; i < m_parserModuleInfo.numSolutions + 1; i++)
    {
        m_dict[QString("value%1").arg(i)] = QString("ext[%1 + offset.prevSol]->val[i]").arg(i-1);

        if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
        {
            m_dict[QString("dx%1").arg(i)] = QString("ext[%1 + offset.prevSol]->dx[i]").arg(i-1);
            m_dict[QString("dy%1").arg(i)] = QString("ext[%1 + offset.prevSol]->dy[i]").arg(i-1);
        }
        else
        {
            m_dict[QString("dr%1").arg(i)] = QString("ext[%1 + offset.prevSol]->dx[i]").arg(i-1);
            m_dict[QString("dz%1").arg(i)] = QString("ext[%1 + offset.prevSol]->dy[i]").arg(i-1);
        }
    }
}

void ParserInstance::addVolumeVariablesErrorCalculation()
{
    foreach (XMLModule::quantity quantity, m_parserModuleInfo.volume.quantity())
    {
        if (quantity.shortname().present())
        {
            QString dep = m_parserModuleInfo.dependenceVolume(QString::fromStdString(quantity.id()));
            QString nonlinearExpr = m_parserModuleInfo.nonlinearExpressionVolume(QString::fromStdString(quantity.id()));

            if (m_parserModuleInfo.linearityType == LinearityType_Linear || nonlinearExpr.isEmpty())
            {
                if (dep.isEmpty())
                {
                    // linear material
                    m_dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->number()").
                            arg(QString::fromStdString(quantity.shortname().get()));
                }
                else if (dep == "time")
                {
                    // linear boundary condition
                    m_dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->number()").
                            arg(QString::fromStdString(quantity.shortname().get()));
                }
                else if (dep == "space")
                {
                    // spacedep boundary condition
                    m_dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberAtPoint(Point(p[0], p[1]))").
                            arg(QString::fromStdString(quantity.shortname().get()));
                }
                else if (dep == "time-space")
                {
                    // spacedep boundary condition
                    m_dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberAtTimeAndPoint(actualTime, Point(x, y))").
                            arg(QString::fromStdString(quantity.shortname().get()));
                }
            }
            else
            {
                // nonlinear material
                m_dict[QString::fromStdString(quantity.shortname().get())] = QString("%1->numberFromTable(%2)").
                        arg(QString::fromStdString(quantity.shortname().get())).
                        arg(Parser::parseErrorExpression(m_parserModuleInfo, nonlinearExpr, false));

                if (m_parserModuleInfo.linearityType == LinearityType_Newton)
                    m_dict["d" + QString::fromStdString(quantity.shortname().get())] = QString("%1->derivativeFromTable(%2)").
                            arg(QString::fromStdString(quantity.shortname().get())).
                            arg(Parser::parseErrorExpression(m_parserModuleInfo, nonlinearExpr, false));
            }
        }
    }

    //todo: XMLModule::function
}

void ParserInstance::addVolumeVariablesWeakform(ParserModuleInfo pmiField, bool isSource)
{
    /*
    QString offsetQuant("offset.quant");
    if(isSource)
        offsetQuant = "offset.sourceQuant";
    */

    foreach (XMLModule::quantity quantity, pmiField.volume.quantity())
    {
        if (quantity.shortname().present())
        {
            m_dict[QString::fromStdString(quantity.shortname().get())] = QString("%1_val").
                    arg(QString::fromStdString(quantity.shortname().get()));

            m_dict["d" + QString::fromStdString(quantity.shortname().get())] = QString("%1_der").
                    arg(QString::fromStdString(quantity.shortname().get()));
        }
    }

    /*
    foreach (XMLModule::function function, pmiField.volume.function())
    {
        // in weak forms, functions replaced by ext functions
        m_dict[QString::fromStdString(function.shortname())] = QString("ext[%1 + %2]->val[i]").
                arg(pmiField.functionOrdering[QString::fromStdString(function.id())]).
                arg(offsetQuant);
    }
    */
}

void ParserInstance::addSurfaceVariables()
{
    // surface quantities still done the old way
    foreach (XMLModule::quantity quantity, m_parserModuleInfo.surface.quantity())
    {
        if (quantity.shortname().present())
        {
            m_dict[QString::fromStdString(quantity.shortname().get())] = QString("%1_val").
                    arg(QString::fromStdString(quantity.shortname().get()));
        }
    }
}

void ParserInstance::addWeakformCheckTokens()
{
    foreach (XMLModule::quantity quantity, m_parserModuleInfo.volume.quantity())
    {
        if (quantity.shortname().present())
        {
            QString dep = m_parserModuleInfo.dependenceVolume(QString::fromStdString(quantity.id()));
            QString nonlinearExpr = m_parserModuleInfo.nonlinearExpressionVolume(QString::fromStdString(quantity.id()));

            if (m_parserModuleInfo.linearityType == LinearityType_Linear || nonlinearExpr.isEmpty())
            {
                if (dep.isEmpty())
                {
                    // linear material
                    m_dict[QString::fromStdString(quantity.shortname().get())] = QString("material->value(\"%1\")->number()").
                            arg(QString::fromStdString(quantity.id()));
                }
                else
                {
                    m_dict[QString::fromStdString(quantity.shortname().get())] = "1.";
                }
            }
            else
            {
                // nonlinear material
                m_dict[QString::fromStdString(quantity.shortname().get())] = "1.";
                m_dict["d" + QString::fromStdString(quantity.shortname().get())] = "1.";
            }
        }
    }

    foreach (XMLModule::quantity quantity, m_parserModuleInfo.surface.quantity())
    {
        if (quantity.shortname().present())
        {
            QString dep = m_parserModuleInfo.dependenceSurface(QString::fromStdString(quantity.id()));

            if (dep.isEmpty())
            {
                // linear boundary condition
                m_dict[QString::fromStdString(quantity.shortname().get())] = QString("boundary->value(\"%1\").number()").
                        arg(QString::fromStdString(quantity.id()));
            }
            else
            {
                m_dict[QString::fromStdString(quantity.shortname().get())] = "1.";
            }
        }
    }

}

void ParserInstance::addPostprocessorBasic()
{
    // coordinates
    if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
    {
        m_dict["x"] = "p[0]"; // "x[i]";
        m_dict["y"] = "p[1]"; // "y[i]";
        // surface integral
        m_dict["tanx"] = "(-normal[1])"; // "e->tx[i]";
        m_dict["tany"] = "(normal[0])"; // "e->ty[i]";
        // velocity (force calculation)
        m_dict["velx"] = "velocity.x";
        m_dict["vely"] = "velocity.y";
        m_dict["velz"] = "velocity.z";
    }
    else
    {
        m_dict["r"] = "p[0]"; // "x[i]";
        m_dict["z"] = "p[1]"; // "y[i]";
        // surface integral
        m_dict["tanr"] = "(-normal[1])"; // "e->tx[i]";
        m_dict["tanz"] = "(normal[0])"; // "e->ty[i]";
        // velocity (force calculation)
        m_dict["velr"] = "velocity.x";
        m_dict["velz"] = "velocity.y";
        m_dict["velphi"] = "velocity.z";
    }

    // constants
    m_dict["PI"] = "M_PI";
    m_dict["f"] = "frequency";
    foreach (XMLModule::constant cnst, m_parserModuleInfo.constants.constant())
        m_dict[QString::fromStdString(cnst.id())] = QString::number(cnst.value());

    // functions
    for (int i = 1; i < m_parserModuleInfo.numSolutions + 1; i++)
    {
        m_dict[QString("value%1").arg(i)] = QString("solution_values[k][%1]").arg(i-1); // QString("value[%1][i]").arg(i-1);
        if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
        {
            m_dict[QString("dx%1").arg(i)] = QString("solution_gradients[k][%1][0]").arg(i-1); // QString("dudx[%1][i]").arg(i-1);
            m_dict[QString("dy%1").arg(i)] = QString("solution_gradients[k][%1][1]").arg(i-1); // QString("dudy[%1][i]").arg(i-1);
            m_dict[QString("ddxx%1").arg(i)] = QString("solution_hessians[k][%1][0][0]").arg(i-1);
            m_dict[QString("ddyy%1").arg(i)] = QString("solution_hessians[k][%1][1][1]").arg(i-1);
        }
        else
        {
            m_dict[QString("dr%1").arg(i)] = QString("solution_gradients[k][%1][0]").arg(i-1); // QString("dudx[%1][i]").arg(i-1);
            m_dict[QString("dz%1").arg(i)] = QString("solution_gradients[k][%1][1]").arg(i-1); // QString("dudy[%1][i]").arg(i-1);
            m_dict[QString("ddrr%1").arg(i)] = QString("solution_hessians[k][%1][0][0]").arg(i-1);
            m_dict[QString("ddzz%1").arg(i)] = QString("solution_hessians[k][%1][1][1]").arg(i-1);
        }
    }
    // eggshell
    if (m_parserModuleInfo.coordinateType == CoordinateType_Planar)
    {
        m_dict["dxegg"] = "normal[0]";
        m_dict["dyegg"] = "normal[1]";
    }
    else
    {
        m_dict["dregg"] = "normal[0]";
        m_dict["dzegg"] = "normal[1]";
    }

}

void ParserInstance::addPostprocessorVariables()
{
    foreach (XMLModule::quantity quantity, m_parserModuleInfo.volume.quantity())
    {
        if (quantity.shortname().present())
        {
            QString nonlinearExpr = m_parserModuleInfo.nonlinearExpressionVolume(QString::fromStdString(quantity.id()));

            if (nonlinearExpr.isEmpty())
                // linear material
                m_dict[QString::fromStdString(quantity.shortname().get())] = QString("material_%1->number()").arg(QString::fromStdString(quantity.id()));
            else
                // nonlinear material
                m_dict[QString::fromStdString(quantity.shortname().get())] = QString("material_%1->numberFromTable(%2)").
                        arg(QString::fromStdString(quantity.id())).
                        arg(Parser::parsePostprocessorExpression(m_parserModuleInfo, nonlinearExpr, false));
        }
    }

    foreach (XMLModule::quantity quantity, m_parserModuleInfo.surface.quantity())
    {
        if (quantity.shortname().present())
        {
            m_dict[QString::fromStdString(quantity.shortname().get())] = QString("boundary_%1->number()").arg(QString::fromStdString(quantity.id()));
        }
    }

    foreach (XMLModule::function function, m_parserModuleInfo.volume.function())
    {
        QString parameter("0");
        // todo: so far used only in Richards, where is OK
        if(QString::fromStdString(function.type()) == "function_1d")
            parameter = "value[0]";

        m_dict[QString::fromStdString(function.shortname())] = QString("%1->getValue(elementMarker, %2)").
                arg(QString::fromStdString(function.shortname())).arg(parameter);
    }

}

void ParserInstance::addFilterVariables()
{
    foreach (XMLModule::quantity quantity, m_parserModuleInfo.volume.quantity())
    {
        if (quantity.shortname().present())
        {
            QString nonlinearExpr = m_parserModuleInfo.nonlinearExpressionVolume(QString::fromStdString(quantity.id()));

            if (nonlinearExpr.isEmpty())
                // linear material
                m_dict[QString::fromStdString(quantity.shortname().get())] = QString("material_%1->number()").arg(QString::fromStdString(quantity.id()));
            else
                // nonlinear material
                m_dict[QString::fromStdString(quantity.shortname().get())] = QString("material_%1->numberFromTable(%2)").
                        arg(QString::fromStdString(quantity.id())).
                        arg(Parser::parseFilterExpression(m_parserModuleInfo, nonlinearExpr, false));
        }
    }

    foreach (XMLModule::function function, m_parserModuleInfo.volume.function())
    {
        QString parameter("0");
        // todo: so far used only in Richards, where is OK
        if(QString::fromStdString(function.type()) == "function_1d")
            parameter = "value[0][i]";

        m_dict[QString::fromStdString(function.shortname())] = QString("%1->getValue(elementMarker, %2)").
                arg(QString::fromStdString(function.shortname())).arg(parameter);
    }

}


//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------


void Parser::addPreviousSolutionsLATokens(QSharedPointer<LexicalAnalyser> lex, CoordinateType coordinateType, int numSolutions)
{
    for (int i = 1; i < numSolutions + 1; i++)
    {
        lex->addVariable(QString("value%1").arg(i));
        if (coordinateType == CoordinateType_Planar)
        {
            lex->addVariable(QString("dx%1").arg(i));
            lex->addVariable(QString("dy%1").arg(i));
        }
        else
        {
            lex->addVariable(QString("dr%1").arg(i));
            lex->addVariable(QString("dz%1").arg(i));
        }
    }
}

void Parser::addSourceCouplingLATokens(QSharedPointer<LexicalAnalyser> lex, CoordinateType coordinateType, int numSourceSolutions)
{
    for (int i = 1; i < numSourceSolutions + 1; i++)
    {
        lex->addVariable(QString("source%1").arg(i));
        if (coordinateType == CoordinateType_Planar)
        {
            lex->addVariable(QString("source%1dx").arg(i));
            lex->addVariable(QString("source%1dy").arg(i));
        }
        else
        {
            lex->addVariable(QString("source%1dr").arg(i));
            lex->addVariable(QString("source%1dz").arg(i));
        }
    }

}


void Parser::addQuantitiesLATokens(QSharedPointer<LexicalAnalyser> lex, ParserModuleInfo parserModuleInfo)
{
    // constants
    lex->addVariable("PI");
    lex->addVariable("f");

    foreach (XMLModule::constant cnst, parserModuleInfo.constants.constant())
        lex->addVariable(QString::fromStdString(cnst.id()));

    // variables
    foreach (XMLModule::quantity quantity, parserModuleInfo.volume.quantity())
    {
        if (quantity.shortname().present())
        {
            lex->addVariable(QString::fromStdString(quantity.shortname().get()));
            lex->addVariable(QString::fromStdString("d" + quantity.shortname().get()));
        }
    }

    foreach (XMLModule::function function, parserModuleInfo.volume.function())
    {
        lex->addVariable(QString::fromStdString(function.shortname()));
    }

    foreach (XMLModule::quantity quantity, parserModuleInfo.surface.quantity())
    {
        if (quantity.shortname().present())
        {
            lex->addVariable(QString::fromStdString(quantity.shortname().get()));
            lex->addVariable(QString::fromStdString("d" + quantity.shortname().get()));
        }
    }

}


void Parser::addWeakFormLATokens(QSharedPointer<LexicalAnalyser> lex, ParserModuleInfo parserModuleInfo)
{
    // scalar field
    lex->addVariable("uval");
    lex->addVariable("upval");
    lex->addVariable("vval");

    // vector field
    lex->addVariable("val0");
    lex->addVariable("val1");
    lex->addVariable("ucurl");
    lex->addVariable("vcurl");
    lex->addVariable("upcurl");

    // coordinates
    if (parserModuleInfo.coordinateType == CoordinateType_Planar)
    {
        // scalar field
        lex->addVariable("udx");
        lex->addVariable("vdx");
        lex->addVariable("udy");
        lex->addVariable("vdy");
        lex->addVariable("updx");
        lex->addVariable("updy");

        lex->addVariable(QString("x"));
        lex->addVariable(QString("y"));
    }
    else
    {
        // scalar field
        lex->addVariable("udr");
        lex->addVariable("vdr");
        lex->addVariable("udz");
        lex->addVariable("vdz");
        lex->addVariable("updr");
        lex->addVariable("updz");

        lex->addVariable(QString("r"));
        lex->addVariable(QString("z"));
    }

    /*
    if (parserModuleInfo.analysisType == AnalysisType_Transient)
    {
        lex->addVariable("deltat");
        lex->addVariable("timedermat");
        lex->addVariable("timedervec");
    }
    */
}

void Parser::addPostprocessorLATokens(QSharedPointer<LexicalAnalyser> lex, ParserModuleInfo parserModuleInfo)
{
    // coordinates
    if (parserModuleInfo.coordinateType == CoordinateType_Planar)
    {
        lex->addVariable("tanx");
        lex->addVariable("tany");
        lex->addVariable("velx");
        lex->addVariable("vely");
        lex->addVariable("velz");
        lex->addVariable("x");
        lex->addVariable("y");
        lex->addVariable("tx");
        lex->addVariable("ty");
        lex->addVariable("nx");
        lex->addVariable("ny");
    }
    else
    {
        lex->addVariable("tanr");
        lex->addVariable("tanz");
        lex->addVariable("velr");
        lex->addVariable("velz");
        lex->addVariable("velphi");
        lex->addVariable("r");
        lex->addVariable("z");
        lex->addVariable("tr");
        lex->addVariable("tz");
        lex->addVariable("nr");
        lex->addVariable("nz");
    }

    // marker area
    lex->addVariable("area");

}
