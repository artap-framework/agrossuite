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

#include "util/util_expr.h"

#include <tbb/tbb.h>
tbb::mutex compileExpressionMutex;

static exprtk::parser<double> *m_exprtkParser = nullptr;

bool compileExpression(const QString &exprString, exprtk::expression<double> &expr, QString *error)
{
    {
        tbb::mutex::scoped_lock lock(compileExpressionMutex);

        if (!m_exprtkParser)
            m_exprtkParser = new exprtk::parser<double>();

        if (error)
            error->clear();

        // replace "**" with "^"
        QString str = exprString;
        str = str.replace("**", "^");
        // compile expression
        if (m_exprtkParser->compile(str.toStdString(), expr))
        {
            return true;
        }
        else
        {
            QString str = QObject::tr("exprtk error: %1, expression: %2: ").
                    arg(QString::fromStdString(m_exprtkParser->error())).
                    arg(exprString);

            for (int i = 0; i < m_exprtkParser->error_count(); ++i)
            {
                exprtk::parser_error::type error = m_exprtkParser->get_error(i);

                str += QObject::tr("error: %1, position: %2, type: [%3], message: %4, expression: %5; ").
                        arg(i).
                        arg(error.token.position).
                        arg(QString::fromStdString(exprtk::parser_error::to_str(error.mode))).
                        arg(QString::fromStdString(error.diagnostic)).
                        arg(exprString);
            }

            // qDebug() << str;
            if (error)
                *error = str;

            return false;
        }
    }
}

