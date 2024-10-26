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

#include "other.h"
#include "util/global.h"
#include "logview.h"

static QUndoStack *m_undoStack = nullptr;
static QMap<QString, QIcon> *m_iconCache = nullptr;

// undo framework
QUndoStack *undoStack()
{
    if (!m_undoStack)
        m_undoStack = new QUndoStack();
    return  m_undoStack;
}

QIcon icon(const QString &name, const QString &defaultName)
{
    if (!m_iconCache)
       m_iconCache = new QMap<QString, QIcon>();

    if (m_iconCache->contains(name))
    {
        return m_iconCache->value(name);
    }
    else
    {
        if (QFile::exists(":/" + name + ".png"))
            m_iconCache->insert(name, QIcon(":/" + name + ".png"));
        else if (QFile::exists(":/" + defaultName + ".png"))
            m_iconCache->insert(name, QIcon(":/" + defaultName + ".png"));
        else
            m_iconCache->insert(name, QIcon());

        return m_iconCache->value(name);
    }
}

QIcon iconAlphabet(const QChar &letter, AlphabetColor color)
{
    QString directory = "";

    switch (color)
    {
    case AlphabetColor_Blue:
        directory = "blue";
        break;
    case AlphabetColor_Bluegray:
        directory = "bluegray";
        break;
    case AlphabetColor_Brown:
        directory = "brown";
        break;
    case AlphabetColor_Green:
        directory = "green";
        break;
    case AlphabetColor_Lightgray:
        directory = "lightgray";
        break;
    case AlphabetColor_Purple:
        directory = "purple";
        break;
    case AlphabetColor_Red:
        directory = "red";
        break;
    case AlphabetColor_Yellow:
        directory = "yellow";
        break;
    default:
        assert(0);
    }

    static QString alphabet = "abcdefghijklmnopqrstuvwxyz";

    if (alphabet.contains(letter.toLower()))
        return icon(QString("alphabet/%1/%2").arg(directory).arg(letter.toLower()));
    else
        return icon(QString("alphabet/%1/imageback").arg(directory));
}

QFont defaultFixedFont(int size)
{
    static int loadedFontID = QFontDatabase::addApplicationFont(":fixed-font.ttf");
    static QFont font = QFontDatabase::applicationFontFamilies(loadedFontID).at(0);

    font.setPointSize(size);
    return font;
}

// ************************************************************************************************************************

SolveThread::SolveThread(const QSharedPointer<Computation> computation) : QThread(), m_computation(computation)
{
    Agros::problem()->setCurrentComputation(computation);
    connect(this, SIGNAL(finished()), this, SLOT(finished()));
}

void SolveThread::run()
{
    Agros::log()->printHeading(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));

    dealii::deal_II_exceptions::disable_abort_on_exception();

    m_computation->solve();
}

void SolveThread::finished()
{
    deleteLater();
}

// ************************************************************************************************************************

QString createToolTip(const QString &title, const QMap<QString, QString> &items)
{
    QString html = "<body style=\"font-size: 11px;\">";
    if (!title.isEmpty())
        html += QString("<h4>%1</h4>").arg(title);
    html += "<table width=\"100%\">";

    foreach (const QString &key, items.keys())
        html += QString("<tr><td><b>%1:</b></td><td>%2</td></tr>").arg(key).arg(items[key]);

    html += "</table>";
    html += "</body>";

    return html;
}

void createTooltipOperate(QMap<QString, QString> &items)
{
    items[QObject::tr("Shift + Left mouse or Middle mouse")] = QObject::tr("Pan over the workspace (you can also use arrows keys)");
    items[QObject::tr("Mouse wheel")] = QObject::tr("Zoom the workspace");
    items[QObject::tr("Middle button double click or Shift + Left mouse double click")] = QObject::tr("Zoom to fit");
}

QString createTooltipOperateOnNodes()
{
    QMap<QString, QString> items;
    items[QObject::tr("Alt + N")] = QObject::tr("Add node by coordinates (open dialog)");
    items[QObject::tr("Ctrl + Left mouse")] = QObject::tr("Add node directly by mouse");
    items[QObject::tr("Control + Shift + Left mouse")] = QObject::tr("Move nodes by mouse");
    items[QObject::tr("Space")] = QObject::tr("Open dialog for setting of selected node");
    createTooltipOperate(items);

    return createToolTip("", items);
}

QString createTooltipOperateOnEdges()
{
    QMap<QString, QString> items;
    items[QObject::tr("Alt + E")] = QObject::tr("Add edge using coordinates (open dialog)");
    items[QObject::tr("Ctrl + Left mouse")] = QObject::tr("Add edge directly by mouse");
    items[QObject::tr("Control + Shift + Left mouse")] = QObject::tr("Move edges by mouse");
    items[QObject::tr("Space")] = QObject::tr("Open dialog for setting of selected edge");
    createTooltipOperate(items);

    return createToolTip("", items);
}

QString createTooltipOperateOnLabels()
{
    QMap<QString, QString> items;
    items[QObject::tr("Alt + L")] = QObject::tr("Add label using coordinates (open dialog)");
    items[QObject::tr("Ctrl + left mouse")] = QObject::tr("Add label directly by mouse");
    items[QObject::tr("Control + Shift + Left mouse")] = QObject::tr("Move labels by mouse");
    items[QObject::tr("Space")] = QObject::tr("Open dialog for setting of selected label");
    createTooltipOperate(items);

    return createToolTip("", items);
}