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

#include "problem_result.h"

// consts
const QString RESULTS = "results";
const QString INFO = "info";

ProblemResult::ProblemResult()
{
    clear();
}

void ProblemResult::clear()
{
    // clear results
    m_results.clear();
    m_info.clear();
}

bool ProblemResult::load(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't open result file.");
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    QJsonObject storeJson = doc.object();

    // results
    QJsonObject resultsJson = storeJson[RESULTS].toObject();
    foreach (QString key, resultsJson.keys())
    {
        m_results[key] = resultsJson[key].toDouble();
    }

    return true;
}

bool ProblemResult::save(const QString &fileName)
{
    if (m_results.isEmpty() && m_info.isEmpty())
    {
        if (QFile::exists(fileName))
            QFile::remove(fileName);

        return true;
    }

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning("Couldn't open result file.");
        return false;
    }

    // root object
    QJsonObject storeJson;

    // results
    QJsonObject resultsJson;
    for (StringToDoubleMap::const_iterator i = m_results.constBegin(); i != m_results.constEnd(); ++i)
    {
        resultsJson[i.key()] = i.value();
    }
    storeJson[RESULTS] = resultsJson;

    // save to file
    QJsonDocument doc(storeJson);
    file.write(doc.toJson());

    return true;
}
