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

#include "solutionstore.h"

#include "util/global.h"
#include "util/constants.h"

#include "logview.h"
#include "field.h"
#include "scene.h"
#include "problem.h"
#include "problem_config.h"

#include <deal.II/fe/fe_q.h>

#include <boost/config.hpp>
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

// consts
const QString SOLUTIONS = "solutions";
const QString FIELDID = "fieldid";
const QString TIMESTEP = "timestep";
const QString ADAPTIVITYSTEP = "adaptivitystep";
const QString RUNTIME = "runtime";

SolutionStore::SolutionRunTimeDetails::SolutionRunTimeDetails()
{
    setStringKeys();
    setDefaultValues();

    clear();
}

bool SolutionStore::SolutionRunTimeDetails::load(QJsonObject &results)
{
    // default
    m_runtime = m_runtimeDefault;

    // results
    foreach(QString keyRes, results.keys())
    {
        Type key = stringKeyToType(keyRes);

        if (m_runtimeDefault.keys().contains(key))
        {
            if (m_runtimeDefault[key].type() == QVariant::Double)
                m_runtime[key] = results[keyRes].toDouble();
            else if (m_runtimeDefault[key].type() == QVariant::Int)
                m_runtime[key] = results[keyRes].toInt();
            else if (m_runtimeDefault[key].type() == QVariant::Bool)
                m_runtime[key] = results[keyRes].toBool();
            else if (m_runtimeDefault[key].type() == QVariant::String)
                m_runtime[key] = results[keyRes].toString();
            else
            {
                qDebug() << "Key not found" << keyRes << results[keyRes].toString();
                return false;
            }
        }
    }

    return true;
}

bool SolutionStore::SolutionRunTimeDetails::save(QJsonObject &results)
{
    foreach(Type key, m_runtime.keys())
    {
        if (m_runtimeDefault[key].type() == QVariant::Bool)
            results[typeToStringKey(key)] = m_runtime[key].toBool();
        else if (m_runtimeDefault[key].type() == QVariant::String)
            results[typeToStringKey(key)] = m_runtime[key].toString();
        else if (m_runtimeDefault[key].type() == QVariant::Int)
            results[typeToStringKey(key)] = m_runtime[key].toInt();
        else if (m_runtimeDefault[key].type() == QVariant::Double)
            results[typeToStringKey(key)] = m_runtime[key].toDouble();
        else
        {
            assert(0);
            return false;
        }
    }

    return true;
}

void SolutionStore::SolutionRunTimeDetails::clear()
{
    setDefaultValues();

    m_runtime = m_runtimeDefault;
}

void SolutionStore::SolutionRunTimeDetails::setStringKeys()
{
    m_runtimeKey[TimeStepLength] = "TimeStepLength";
    m_runtimeKey[AdaptivityError] = "AdaptivityError";
    m_runtimeKey[DOFs] = "DOFs";
}

void SolutionStore::SolutionRunTimeDetails::setDefaultValues()
{
    m_runtimeDefault.clear();

    m_runtimeDefault[TimeStepLength] = 0.0;
    m_runtimeDefault[AdaptivityError] = 0.0;
    m_runtimeDefault[DOFs] = 0;
}

// *******************************************************************************************************************************

SolutionStore::SolutionStore(Computation *parentProblem) : m_computation(parentProblem)
{
}

SolutionStore::~SolutionStore()
{
    clear();
}

QString SolutionStore::baseStoreFileName(FieldSolutionID solutionID) const
{
    QString fn = QString("%1/%2/%3").
            arg(cacheProblemDir()).
            arg(m_computation->problemDir()).
            arg(solutionID.toString());

    return fn;
}

void SolutionStore::clear()
{
    // fast remove of all files
    foreach(FieldSolutionID sid, m_multiSolutions)
        removeSolution(sid, false);

    // remove runtime
    QString fn = QString("%1/%2/runtime.json").
            arg(cacheProblemDir()).
            arg(m_computation->problemDir());
    if (QFile::exists(fn))
        QFile::remove(fn);

    assert(m_multiSolutions.isEmpty());
    assert(m_multiSolutionRunTimeDetails.isEmpty());
    assert(m_multiSolutionDealCache.isEmpty());
}

MultiArray &SolutionStore::multiArray(FieldSolutionID solutionID)
{
    assert(m_multiSolutions.contains(solutionID));

    if (!m_multiSolutionDealCache.contains(solutionID))
    {
        // load from stream
        QString baseFN = baseStoreFileName(solutionID);

        // triangulation
        std::shared_ptr<dealii::Triangulation<2> > triangulation = std::shared_ptr<dealii::Triangulation<2> >(new dealii::Triangulation<2>());
        QString fnMesh = QString("%1.msh").arg(baseFN);
        if (!QFile::exists(fnMesh))
            assert(0); // return MultiArray();
        std::ifstream ifsMesh(fnMesh.toStdString());
        boost::archive::binary_iarchive sbiMesh(ifsMesh);
        triangulation->load(sbiMesh, 0);

        // dof handler
        std::shared_ptr<dealii::hp::DoFHandler<2> > doFHandler = std::shared_ptr<dealii::hp::DoFHandler<2> >(new dealii::hp::DoFHandler<2>(*triangulation));
        doFHandler->distribute_dofs(*m_computation->problemSolver()->feCollection(m_computation->fieldInfo(solutionID.fieldId)));
        QString fnDoF = QString("%1.dof").arg(baseFN);
        if (!QFile::exists(fnDoF))
            assert(0); // return MultiArray();
        std::ifstream ifsDoF(fnDoF.toStdString());
        boost::archive::binary_iarchive sbiDoF(ifsDoF);
        doFHandler->load(sbiDoF, 0);

        // solution vector
        dealii::Vector<double> solution;
        QString fnSol = QString("%1.sol").arg(baseFN);
        if (!QFile::exists(fnSol))
            assert(0); // return MultiArray();
        std::ifstream ifsSol(fnSol.toStdString());
        boost::archive::binary_iarchive sbiSol(ifsSol);
        solution.load(sbiSol, 0);

        // insert to the cache
        insertMultiSolutionToCache(solutionID, *doFHandler, solution);
    }

    return m_multiSolutionDealCache[solutionID];
}

bool SolutionStore::contains(FieldSolutionID solutionID) const
{
    return m_multiSolutions.contains(solutionID);
}

void SolutionStore::addSolution(FieldSolutionID solutionID,
                                dealii::hp::DoFHandler<2> &doFHandler,
                                dealii::Vector<double> &solution,
                                SolutionRunTimeDetails runTime)
{
    assert(solutionID.timeStep >= 0);
    assert(solutionID.adaptivityStep >= 0);

    // save soloution
    QString baseFN = baseStoreFileName(solutionID);

    bool forceSave = true;

    // speedup for transient problems with same mesh
    {
        if (solutionID.timeStep > 0)
        {
            // decrease timestep
            FieldSolutionID previousSolutionID = solutionID;
            previousSolutionID.timeStep--;

            if (contains(previousSolutionID))
            {
                MultiArray previousMA = m_multiSolutionDealCache[previousSolutionID];
                if (doFHandler.get_triangulation().n_cells() == previousMA.doFHandler().get_triangulation().n_cells())
                {
                    // copy file
                    forceSave = false;
                    QString previousFN = baseStoreFileName(previousSolutionID);

                    QString fnMesh = QString("%1.msh").arg(baseFN);
                    QString previousMesh = QString("%1.msh").arg(previousFN);
                    QFile::copy(previousMesh, fnMesh);
                }
            }
        }

        if (forceSave)
        {
            QString fnMesh = QString("%1.msh").arg(baseFN);
            std::ofstream ofsMesh(fnMesh.toStdString());
            boost::archive::binary_oarchive sbMesh(ofsMesh);
            doFHandler.get_triangulation().save(sbMesh, 0);
        }
    }

    QString fnDoF = QString("%1.dof").arg(baseFN);
    std::ofstream ofsDoF(fnDoF.toStdString());
    boost::archive::binary_oarchive sbDoF(ofsDoF);
    doFHandler.save(sbDoF, 0);

    QString fnSol = QString("%1.sol").arg(baseFN);
    std::ofstream ofsSol(fnSol.toStdString());
    boost::archive::binary_oarchive sbSol(ofsSol);
    solution.save(sbSol, 0);

    // append multisolution
    m_multiSolutions.append(solutionID);

    // append properties
    m_multiSolutionRunTimeDetails.insert(solutionID, runTime);

    // insert to the cache
    insertMultiSolutionToCache(solutionID, doFHandler, solution);

    // save run time details to the file
    saveRunTimeDetails();
}

void SolutionStore::removeSolution(FieldSolutionID solutionID, bool saveRunTime)
{
    assert(m_multiSolutions.contains(solutionID));

    // remove from list
    m_multiSolutions.removeOne(solutionID);
    // remove properties
    m_multiSolutionRunTimeDetails.remove(solutionID);
    // remove from cache
    if (m_multiSolutionDealCache.contains(solutionID))
    {
        // free ma
        m_multiSolutionDealCache[solutionID].clear();
        m_multiSolutionDealCache.remove(solutionID);
        m_multiSolutionCacheIDOrder.removeOne(solutionID);
    }

    // remove old files
    QString baseFN = baseStoreFileName(solutionID);

    QString fnMesh = QString("%1.msh").arg(baseFN);
    if (QFile::exists(fnMesh))
        QFile::remove(fnMesh);

    QString fnSpace = QString("%1.dof").arg(baseFN);
    if (QFile::exists(fnSpace))
        QFile::remove(fnSpace);

    QString fnSolution = QString("%1.sol").arg(baseFN);
    if (QFile::exists(fnSolution))
        QFile::remove(fnSolution);

    // save structure to the file
    if (saveRunTime)
        saveRunTimeDetails();
}

int SolutionStore::lastTimeStep(const FieldInfo *fieldInfo) const
{
    int timeStep = NOT_FOUND_SO_FAR;
    foreach(FieldSolutionID sid, m_multiSolutions)
    {
        if ((sid.fieldId == fieldInfo->fieldId()) && (sid.timeStep > timeStep))
            timeStep = sid.timeStep;
    }

    return timeStep;
}

int SolutionStore::lastAdaptiveStep(const FieldInfo *fieldInfo, int timeStep) const
{
    if (timeStep == -1)
        timeStep = lastTimeStep(fieldInfo);

    int adaptiveStep = NOT_FOUND_SO_FAR;
    foreach(FieldSolutionID sid, m_multiSolutions)
    {
        if ((sid.fieldId == fieldInfo->fieldId()) && (sid.timeStep == timeStep) && (sid.adaptivityStep > adaptiveStep))
            adaptiveStep = sid.adaptivityStep;
    }

    return adaptiveStep;
}

void SolutionStore::insertMultiSolutionToCache(FieldSolutionID solutionID, dealii::hp::DoFHandler<2> &doFHandler, dealii::Vector<double> &solution)
{
    // triangulation
    dealii::Triangulation<2> *newTriangulation = new dealii::Triangulation<2>();
    newTriangulation->copy_triangulation(doFHandler.get_triangulation());

    // dof handler
    std::stringstream fsDoF(std::ios::out | std::ios::in | std::ios::binary);
    boost::archive::binary_oarchive sboDoF(fsDoF);
    doFHandler.save(sboDoF, 0);
    // new handler
    dealii::hp::DoFHandler<2> *newDoFHandler = new dealii::hp::DoFHandler<2>(*newTriangulation);
    newDoFHandler->distribute_dofs(*m_computation->problemSolver()->feCollection(m_computation->fieldInfo(solutionID.fieldId)));
    // load
    boost::archive::binary_iarchive sbiDoF(fsDoF);
    newDoFHandler->load(sbiDoF, 0);

    assert(!m_multiSolutionDealCache.contains(solutionID));

    // flush cache
    if (m_multiSolutionDealCache.count() > Agros::configComputer()->value(Config::Config_CacheSize).toInt())
    {
        assert(!m_multiSolutionCacheIDOrder.empty());
        // find first item of current field (coupled fields cannot be removed)
        foreach (FieldSolutionID solutionIDRemove, m_multiSolutionCacheIDOrder)
        {
            if (solutionID.fieldId == solutionIDRemove.fieldId)
            {
                // free ma
                m_multiSolutionDealCache[solutionIDRemove].clear();
                m_multiSolutionDealCache.remove(solutionIDRemove);
                m_multiSolutionCacheIDOrder.removeOne(solutionIDRemove);
            }
        }
    }

    // add solution
    m_multiSolutionDealCache.insert(solutionID, MultiArray(newTriangulation, newDoFHandler, solution));
    m_multiSolutionCacheIDOrder.append(solutionID);
}

bool SolutionStore::loadRunTimeDetails()
{
    QString fnJSON = QString("%1/%2/runtime.json").arg(cacheProblemDir()).arg(m_computation->problemDir());
    QFile file(fnJSON);

    if (!file.open(QIODevice::ReadOnly))
    {
        // qWarning("Couldn't open runtime file.");
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    QJsonObject rootJson = doc.object();

    // solutions
    int time_step = 0;
    QJsonArray solutionsJson = rootJson[SOLUTIONS].toArray();
    for (int i = 0; i < solutionsJson.size(); i++)
    {
        QJsonObject solutionJson = solutionsJson[i].toObject();
        QJsonObject runtimeJson = solutionJson[RUNTIME].toObject();
        SolutionRunTimeDetails runTime;
        runTime.load(runtimeJson);

        FieldSolutionID solutionID(solutionJson[FIELDID].toString(),
                                   solutionJson[TIMESTEP].toInt(),
                                   solutionJson[ADAPTIVITYSTEP].toInt());
        // append multisolution
        m_multiSolutions.append(solutionID);

        // define transient time step
        if (solutionJson[TIMESTEP].toInt() > time_step)
        {
            // new time step
            time_step = solutionJson[TIMESTEP].toInt();
            m_computation->setActualTimeStepLength(runTime.value(SolutionStore::SolutionRunTimeDetails::TimeStepLength).toDouble());
        }

        // append run time details
        m_multiSolutionRunTimeDetails.insert(solutionID, runTime);
    }

    return true;
}

bool SolutionStore::saveRunTimeDetails()
{
    QString fnJSON = QString("%1/%2/runtime.json").arg(cacheProblemDir()).arg(m_computation->problemDir());
    QFile file(fnJSON);

    if (!file.open(QIODevice::WriteOnly))
    {
        // qWarning("Couldn't open result file.");
        return false;
    }

    // root object
    QJsonObject rootJson;

    // solution
    QJsonArray solutionsJson;
    foreach(FieldSolutionID solutionID, m_multiSolutions)
    {
        QJsonObject runtimeJson;
        SolutionRunTimeDetails runTime = m_multiSolutionRunTimeDetails[solutionID];
        runTime.save(runtimeJson);

        QJsonObject solutionJson;
        solutionJson[FIELDID] = solutionID.fieldId;
        solutionJson[TIMESTEP] = solutionID.timeStep;
        solutionJson[ADAPTIVITYSTEP] = solutionID.adaptivityStep;
        solutionJson[RUNTIME] = runtimeJson;

        solutionsJson.append(solutionJson);
    }
    rootJson[SOLUTIONS] = solutionsJson;

    // save to file
    QJsonDocument doc(rootJson);
    file.write(doc.toJson());

    return true;
}
