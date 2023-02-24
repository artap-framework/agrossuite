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

#ifndef SCRIPTGENERATORUTILS_H
#define SCRIPTGENERATORUTILS_H

#include "qplaintextedit.h"
#include <QtCore/QVector>
#include <QtCore/QSet>
#include <QtGui/QSyntaxHighlighter>

#include <QRegExp>
#include <QHash>
#include <QObject>
#include <QTextCharFormat>

class QTextDocument;
class ScriptEditor;

struct ParenthesisInfo
{
    char character;
    int position;
};

class TextBlockData : public QTextBlockUserData
{
public:
    TextBlockData();

    QVector<ParenthesisInfo *> parentheses();
    void insert(ParenthesisInfo *info);

private:
    QVector<ParenthesisInfo *> m_parentheses;
};

class PlainTextEditParenthesis : public QPlainTextEdit
{
    Q_OBJECT

public:
   PlainTextEditParenthesis(QWidget *parent = 0);

protected slots:
   void matchParentheses(char left = '(', char right = ')');

protected:
    bool matchLeftParenthesis(char left, char right, QTextBlock currentBlock, int index, int numRightParentheses);
    bool matchRightParenthesis(char left, char right, QTextBlock currentBlock, int index, int numLeftParentheses);
    void createParenthesisSelection(int pos);
};

class PythonHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    PythonHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);
    void highlightBlockParenthesis(const QString &text, char left, char right);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat operatorFormat;
    QTextCharFormat numberFormat;
};

class ScriptEditor : public PlainTextEditParenthesis
{
    Q_OBJECT

public:
    QMap<int, QString> errorMessagesPyFlakes;

    ScriptEditor(QWidget *parent = 0);
    ~ScriptEditor();

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void lineNumberAreaMouseMoveEvent(QMouseEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void updateLineNumberAreaWidth();
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);

    void indentSelection();
    void unindentSelection();
    void indentAndUnindentSelection(bool doIndent);
    void commentAndUncommentSelection();

private:
    QWidget *lineNumberArea;
};

class ScriptEditorLineNumberArea : public QWidget
{
public:
    ScriptEditorLineNumberArea(ScriptEditor *editor) : QWidget(editor)
    {
        setMouseTracking(true);
        codeEditor = editor;
    }

    QSize sizeHint() const { return QSize(codeEditor->lineNumberAreaWidth(), 0); }

protected:
    void paintEvent(QPaintEvent *event) { codeEditor->lineNumberAreaPaintEvent(event); }

    virtual void mouseMoveEvent(QMouseEvent *event) { codeEditor->lineNumberAreaMouseMoveEvent(event); }

private:
    ScriptEditor *codeEditor;
};


#endif // SCRIPTGENERATORUTILS_H

