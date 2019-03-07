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

#include "problem_parameter.h"

#include "util/global.h"
#include "util/constants.h"

#include "problem.h"
#include "problem_config.h"

const QString PARAMETERS = "Parameters";
const QString NAME = "name";
const QString VALUE = "value";

template <typename T>
inline T from_csv(T column, T val)
{
    static QList<double> xd;
    static QList<QList<double> > yd;

    // cache works only with first ags file !!!
    if (xd.length() == 0)
    {
        QFileInfo info = QFileInfo(Agros::problem()->archiveFileName());
        QString fn =  info.absolutePath() + "/" + info.baseName() + ".csv";

        QFile file(fn);
        if (!file.exists())
            return std::numeric_limits<T>::quiet_NaN();

        if (file.open(QIODevice::ReadOnly))
        {
            while (!file.atEnd())
            {
                QString line = file.readLine();
                // replace ',' -> '.'
                line = line.replace(",", ".");
                QStringList ar = line.split(';');
                xd.append(ar[0].toDouble());

                // values
                QList<double> values;
                for (int j = 1; j < ar.length(); j++)
                {
                    values.append(ar[j].toDouble());
                }

                yd.append(values);
            }
        }
    }

    int col = int(column - 1);
    if (xd.length() > 0)
    {
        if (val <= xd[0])
            return T(yd[0][col]);
        else if (val >= xd[xd.length() - 1])
            return T(yd[yd.length() - 1][col]);
    }

    for (int i = 0; i < xd.length() - 1; i++)
    {
        // linear aproximation
        if (val >= xd[i] and val <= xd[i+1])
        {
            double k = (yd[i+1][col] - yd[i][col]) / (xd[i+1] - xd[i]);
            double b = yd[i][col] - k * xd[i];

            return T(k * val + b);
        }
    }

    return std::numeric_limits<T>::quiet_NaN();
}

/*
template <typename T>
class pokus : public exprtk::igeneric_function<T>
{
public:

   typedef typename exprtk::igeneric_function<T> igfun_t;
   typedef typename igfun_t::parameter_list_t parameter_list_t;
   typedef typename igfun_t::generic_type::scalar_view scalar_t;
   typedef typename igfun_t::generic_type::string_view string_t;

   using exprtk::igeneric_function<T>::operator();

   pokus()
   : igfun_t("ST", igfun_t::e_rtrn_scalar)
   // : igfun_t("ST", igfun_t::e_rtrn_string)0
   { exprtk::disable_has_side_effects(*this); }

   inline T operator()(std::string& result,
                       parameter_list_t parameters)
   {
      T val = scalar_t(parameters[0])();
      std::cout << result << ", value " << QString::number(val).toStdString() << std::endl;
      return T(val);
   }

};
*/

ProblemParameters::ProblemParameters(const QList<ProblemParameter> parameters) : m_parameters(QMap<QString, ProblemParameter>())
{
    set(parameters);
}

ProblemParameters::~ProblemParameters()
{
    clear();
}

void ProblemParameters::load(QJsonObject &object)
{
    // parameters
    if (object.contains(PARAMETERS))
    {
        QList<ProblemParameter> parameters;

        if (object[PARAMETERS].isString())
        {
            // TODO: remove - old interface
            QString str = object[PARAMETERS].toString();
            QStringList strKeysAndValues = str.split(":");
            QStringList strKeys = (strKeysAndValues[0].size() > 0) ? strKeysAndValues[0].split("|") : QStringList();
            QStringList strValues = (strKeysAndValues[1].size() > 0) ? strKeysAndValues[1].split("|") : QStringList();
            assert(strKeys.count() == strValues.count());

            for (int i = 0; i < strKeys.count(); i++)
                parameters.append(ProblemParameter(strKeys[i], strValues[i].toDouble()));
        }
        else
        {
            // new interface
            QJsonArray parametersJson = object[PARAMETERS].toArray();
            for (int i = 0; i < parametersJson.size(); i++)
            {
                QJsonObject parameterJson = parametersJson[i].toObject();

                parameters.append(ProblemParameter(parameterJson[NAME].toString(), parameterJson[VALUE].toDouble()));
            }
        }

        set(parameters);
    }
}

void ProblemParameters::save(QJsonObject &object)
{
    QJsonArray parametersJson;
    foreach(ProblemParameter parameter, m_parameters.values())
    {
        QJsonObject parameterJson;
        parameterJson[NAME] = parameter.name();
        parameterJson[VALUE] = parameter.value();

        parametersJson.append(parameterJson);
    }
    object[PARAMETERS] = parametersJson;
}

void ProblemParameters::clear()
{
    m_parametersSymbolTable.clear();
    m_parameters.clear();
}

void ProblemParameters::add(const ProblemParameter parameter)
{
    m_parameters[parameter.name()] = parameter;
}

void ProblemParameters::remove(const QString &name)
{
    m_parameters.remove(name);
}

void ProblemParameters::set(const QString &key, double val)
{
    try
    {
        // existing key with same value
        if (m_parameters.keys().contains(key) && fabs(number(key) - val) < EPS_ZERO)
            return;

        Agros::problem()->config()->checkVariableName(key, key);
        m_parameters[key] = ProblemParameter(key, val);

        // create new table - invalidate
        m_parametersSymbolTable = exprtk::symbol_table<double>();
        m_parametersSymbolTable.add_constants();
        m_parametersSymbolTable.add_function("from_csv", from_csv);

        foreach (QString k, m_parameters.keys())
        {
            if (k == key)
                m_parametersSymbolTable.add_constant(key.toStdString(), val);
            else
                m_parametersSymbolTable.add_constant(k.toStdString(), number(k));
        }
    }
    catch (AgrosException &e)
    {
        // raise exception
        throw e.toString();
    }
}

void ProblemParameters::set(const QList<ProblemParameter> parameters)
{
    foreach (ProblemParameter parameter, parameters)
        m_parameters[parameter.name()] = parameter;

    // create new table
    m_parametersSymbolTable = exprtk::symbol_table<double>();
    m_parametersSymbolTable.add_constants();
    m_parametersSymbolTable.add_function("from_csv", from_csv);
    // FuncApproximationFromCSV *approximation = new FuncApproximationFromCSV();   
    // pokus<double> *pokus = new ::pokus<double>();
    // m_parametersSymbolTable.add_function("pokus", *pokus);
    // pokus2<double> *pokus2 = new ::pokus2<double>();
    // m_parametersSymbolTable.add_function("pokus2", *pokus2);
    // write2<double> *write2 = new ::write2<double>();
    // m_parametersSymbolTable.add_function("write2", *write2);


    foreach (ProblemParameter parameter, parameters)
        m_parametersSymbolTable.add_constant(parameter.name().toStdString(), parameter.value());
}

ProblemParameter ProblemParameters::parameter(const QString &name) const
{
    assert(m_parameters.contains(name));
    return m_parameters[name];
}
