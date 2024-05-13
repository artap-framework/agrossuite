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

#include "scriptgeneratordialog.h"

#include "util/global.h"
#include "gui/valuelineedit.h"

#include <QSyntaxHighlighter>

#include <QTextBlock>
#include <QTextBlockUserData>

const int ParenthesisMatcherPropertyId = QTextFormat::UserProperty;
const int ErrorMarkerPropertyId = QTextFormat::UserProperty + 1;

// ************************************************************************************************************

TextBlockData::TextBlockData()
{

}

QVector<ParenthesisInfo *> TextBlockData::parentheses()
{
    return m_parentheses;
}

void TextBlockData::insert(ParenthesisInfo *info)
{
    int i = 0;
    while (i < m_parentheses.size() && info->position > m_parentheses.at(i)->position)
        ++i;

    m_parentheses.insert(i, info);
}

// ************************************************************************************************************

PlainTextEditParenthesis::PlainTextEditParenthesis(QWidget *parent)
    : QPlainTextEdit(parent)
{
}

void PlainTextEditParenthesis::matchParentheses(char left, char right)
{
    QList<QTextEdit::ExtraSelection> selections;
    setExtraSelections(selections);

    TextBlockData *data = static_cast<TextBlockData *>(textCursor().block().userData());

    if (data)
    {
        QVector<ParenthesisInfo *> infos = data->parentheses();

        int pos = textCursor().block().position();
        for (int i = 0; i < infos.size(); ++i)
        {
            ParenthesisInfo *info = infos.at(i);

            int curPos = textCursor().position() - textCursor().block().position();
            if (info->position == curPos - 1 && info->character == left)
            {
                if (matchLeftParenthesis(left, right, textCursor().block(), i + 1, 0))
                    createParenthesisSelection(pos + info->position);
            }
            else if (info->position == curPos - 1 && info->character == right)
            {
                if (matchRightParenthesis(left, right, textCursor().block(), i - 1, 0))
                    createParenthesisSelection(pos + info->position);
            }
        }
    }
}

bool PlainTextEditParenthesis::matchLeftParenthesis(char left, char right, QTextBlock currentBlock, int i, int numLeftParentheses)
{
    TextBlockData *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> infos = data->parentheses();

    int docPos = currentBlock.position();
    for (; i < infos.size(); ++i)
    {
        ParenthesisInfo *info = infos.at(i);

        if (info->character == left)
        {
            ++numLeftParentheses;
            continue;
        }

        if (info->character == right && numLeftParentheses == 0)
        {
            createParenthesisSelection(docPos + info->position);
            return true;
        }
        else
        {
            --numLeftParentheses;
        }
    }

    currentBlock = currentBlock.next();
    if (currentBlock.isValid())
        return matchLeftParenthesis(left, right, currentBlock, 0, numLeftParentheses);

    return false;
}

bool PlainTextEditParenthesis::matchRightParenthesis(char left, char right, QTextBlock currentBlock, int i, int numRightParentheses)
{
    TextBlockData *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> parentheses = data->parentheses();

    int docPos = currentBlock.position();
    for (; i > -1 && parentheses.size() > 0; --i)
    {
        ParenthesisInfo *info = parentheses.at(i);
        if (info->character == right)
        {
            ++numRightParentheses;
            continue;
        }
        if (info->character == left && numRightParentheses == 0)
        {
            createParenthesisSelection(docPos + info->position);
            return true;
        }
        else
        {
            --numRightParentheses;
        }
    }

    currentBlock = currentBlock.previous();
    if (currentBlock.isValid())
        return matchRightParenthesis(left, right, currentBlock, 0, numRightParentheses);

    return false;
}

void PlainTextEditParenthesis::createParenthesisSelection(int pos)
{
    QList<QTextEdit::ExtraSelection> selections = extraSelections();

    QTextEdit::ExtraSelection selection;
    QTextCharFormat format = selection.format;
    format.setForeground(Qt::red);
    format.setFontWeight(QFont::Bold);
    // format.setBackground(Qt::lightGray);
    selection.format = format;

    QTextCursor cursor = textCursor();
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = cursor;

    selections.append(selection);

    setExtraSelections(selections);
}

PythonHighlighter::PythonHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);

    QStringList keywordPatterns;
    keywordPatterns << "and" << "assert" << "break" << "class" << "continue" << "def"
            << "del" << "elif" << "else" << "except" << "exec" << "finally"
            << "for" << "from" << "global" << "if" << "import" << "in"
            << "is" << "lambda" << "not" << "or" << "pass" << "print" << "raise"
            << "return" << "try" << "while" << "yield"
            << "None" << "True"<< "False";

    foreach (const QString &pattern, keywordPatterns)
    {
        rule.pattern = QRegExp("\\b" + pattern + "\\b", Qt::CaseInsensitive);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    functionFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    operatorFormat.setForeground(Qt::black);
    rule.pattern = QRegExp("[\\\\|\\<|\\>|\\=|\\!|\\+|\\-|\\*|\\/|\\%]+");
    rule.pattern.setMinimal(true);
    rule.format = operatorFormat;
    highlightingRules.append(rule);

    numberFormat.setForeground(Qt::blue);
    rule.format = numberFormat;
    rule.pattern = QRegExp("\\b[+-]?[0-9]+[lL]?\\b");
    highlightingRules.append(rule);
    rule.pattern = QRegExp("\\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\\b");
    highlightingRules.append(rule);
    rule.pattern = QRegExp("\\b[+-]?[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\\b");
    highlightingRules.append(rule);

    quotationFormat.setForeground(Qt::darkGreen);
    rule.format = quotationFormat;
    rule.pattern = QRegExp("\"[^\"\\\\]*(\\\\.[^\"\\\\]*)*\"");
    highlightingRules.append(rule);
    rule.pattern = QRegExp("'[^'\\\\]*(\\\\.[^'\\\\]*)*'");
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    commentStartExpression = QRegExp("\"\"\"");
    commentEndExpression = QRegExp("\"\"\"");
    multiLineCommentFormat.setForeground(Qt::red);
}

void PythonHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    int add = 0;
    if (previousBlockState() != 1)
    {
        startIndex = commentStartExpression.indexIn(text);
        add = commentStartExpression.matchedLength();
    }

    while (startIndex >= 0)
    {
        int endIndex = commentEndExpression.indexIn(text, startIndex + add);
        int commentLength;
        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            setCurrentBlockState(0);
            commentLength = endIndex - startIndex + add + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }

    // parenthesis
    highlightBlockParenthesis(text, '(', ')');
    // highlightBlockParenthesis(text, '[', ']');
    // highlightBlockParenthesis(text, '{', '}');
}

void PythonHighlighter::highlightBlockParenthesis(const QString &text, char left, char right)
{
    TextBlockData *data = new TextBlockData();

    int leftPos = text.indexOf(left);
    while (leftPos != -1)
    {
        ParenthesisInfo *info = new ParenthesisInfo();
        info->character = left;
        info->position = leftPos;

        data->insert(info);
        leftPos = text.indexOf(left, leftPos + 1);
    }

    int rightPos = text.indexOf(right);
    while (rightPos != -1)
    {
        ParenthesisInfo *info = new ParenthesisInfo();
        info->character = right;
        info->position = rightPos;

        data->insert(info);
        rightPos = text.indexOf(right, rightPos + 1);
    }

    setCurrentBlockUserData(data);
}

// ***********************************************************************************************************


const QString TABS = "    ";
const int TABS_SIZE = 4;

int firstNonSpace(const QString& text)
{
    int i = 0;
    while (i < text.size())
    {
        if (!text.at(i).isSpace())
            return i;
        ++i;
    }
    return i;
}

int indentedColumn(int column, bool doIndent)
{
    int aligned = (column / TABS_SIZE) * TABS_SIZE;
    if (doIndent)
        return aligned + TABS_SIZE;
    if (aligned < column)
        return aligned;
    return qMax(0, aligned - TABS_SIZE);
}

int columnAt(const QString& text, int position)
{
    int column = 0;
    for (int i = 0; i < position; ++i)
    {
        if (text.at(i) == QLatin1Char('\t'))
            column = column - (column % TABS_SIZE) + TABS_SIZE;
        else
            ++column;
    }
    return column;
}

ScriptEditor::ScriptEditor(QWidget *parent)
    : PlainTextEditParenthesis(parent)
{
    lineNumberArea = new ScriptEditorLineNumberArea(this);

    setFont(defaultFixedFont(9));
    // setTabStopWidth(fontMetrics().width(TABS));
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabChangesFocus(false);

    // highlighter
    new PythonHighlighter(document());

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth()));
    connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect &, int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth();
    highlightCurrentLine();
}

ScriptEditor::~ScriptEditor()
{
}

void ScriptEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void ScriptEditor::updateLineNumberAreaWidth()
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void ScriptEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
    {
        lineNumberArea->scroll(0, dy);
    }
    else
    {
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth();
}

void ScriptEditor::keyPressEvent(QKeyEvent *event)
{
    QTextCursor cursor = textCursor();
    int oldPos = cursor.position();
    int indent = firstNonSpace(cursor.block().text());

    if (event->key() == Qt::Key_Tab && !(event->modifiers() & Qt::ShiftModifier))
    {
        if (!textCursor().hasSelection())
        {
            // insert 4 spaces instead of tab
            textCursor().insertText(QString(4, ' '));
        }
        else
        {
            // indent the selection
            indentSelection();
        }
    }
    else if (event->key() == Qt::Key_Backtab && (event->modifiers() & Qt::ShiftModifier))
    {
        if (!textCursor().hasSelection())
        {
            // moves position backward 4 spaces
            QTextCursor cursor = textCursor();
            cursor.setPosition(cursor.position() - 4, QTextCursor::MoveAnchor);
            setTextCursor(cursor);
        }
        else
        {
            // unindent the selection
            unindentSelection();
        }
    }
    else if ((event->key() == Qt::Key_Backspace) && (document()->characterAt(oldPos - 1) == ' ')
             && (document()->characterAt(oldPos - 2) == ' ')
             && (document()->characterAt(oldPos - 3) == ' ')
             && (document()->characterAt(oldPos - 4) == ' '))
    {
        cursor.beginEditBlock();
        // determine selection to delete
        int newPos = oldPos - 4;
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        int startPosOfLine = cursor.position();
        if (newPos < startPosOfLine)
            newPos = startPosOfLine;
        // make selection
        cursor.setPosition(oldPos, QTextCursor::MoveAnchor);
        cursor.setPosition(newPos, QTextCursor::KeepAnchor);
        cursor.deleteChar();
        cursor.endEditBlock();
        setTextCursor(cursor);
    }
    else if ((event->key() == Qt::Key_Return) && (indent))
    {
        cursor.beginEditBlock();

        // add 1 extra indent if current line begins a code block
        bool inCodeBlock = false;
        if (QRegExp("\\:").indexIn(cursor.block().text()) != -1)
        {
            indent += TABS_SIZE;
            inCodeBlock = true;
        }

        cursor.insertBlock();
        QString spaces(indent, true ? QLatin1Char(' ') : QLatin1Char('\t'));
        cursor.insertText(spaces);

        cursor.endEditBlock();
        setTextCursor(cursor);
    }
    else
    {
        QPlainTextEdit::keyPressEvent(event);
    }
}

void ScriptEditor::indentSelection()
{
    indentAndUnindentSelection(true);
}

void ScriptEditor::unindentSelection()
{
    indentAndUnindentSelection(false);
}

void ScriptEditor::indentAndUnindentSelection(bool doIndent)
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    // indent or unindent the selected lines
    int pos = cursor.position();
    int anchor = cursor.anchor();
    int start = qMin(anchor, pos);
    int end = qMax(anchor, pos);

    QTextDocument *doc = document();
    QTextBlock startBlock = doc->findBlock(start);
    QTextBlock endBlock = doc->findBlock(end-1).next();

    for (QTextBlock block = startBlock; block != endBlock; block = block.next())
    {
        QString text = block.text();
        if (doIndent)
        {
            int indentPosition = firstNonSpace(text);
            cursor.setPosition(block.position() + indentPosition);
            cursor.insertText(QString(TABS_SIZE, ' '));
        }
        else
        {
            int indentPosition = firstNonSpace(text);
            int targetColumn = indentedColumn(columnAt(text, indentPosition), false);
            cursor.setPosition(block.position() + indentPosition);
            cursor.setPosition(block.position() + targetColumn, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
        }
    }

    // reselect the selected lines
    cursor.setPosition(startBlock.position());
    cursor.setPosition(endBlock.previous().position(), QTextCursor::KeepAnchor);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

    cursor.endEditBlock();
    setTextCursor(cursor);
}

void ScriptEditor::commentAndUncommentSelection()
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    // previous selection state
    int selStart = cursor.selectionStart();
    int selEnd = cursor.selectionEnd();
    cursor.setPosition(selEnd, QTextCursor::MoveAnchor);
    int blockEnd = cursor.blockNumber();

    // extend selStart to first blocks's start-of-block
    // extend selEnd to last block's end-of-block
    cursor.setPosition(selStart, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    selStart = cursor.position();
    cursor.setPosition(selEnd, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    selEnd = cursor.position();

    // process first block
    cursor.setPosition(selStart, QTextCursor::MoveAnchor);
    QRegExp commentPattern("^#");
    if (commentPattern.indexIn(cursor.block().text()) == -1)
    {
        // comment it, if the block does not starts with '#'
        cursor.insertText("#");
        selEnd += 1;
    }
    else
    {
        // else uncomment it
        cursor.setPosition(selStart + commentPattern.matchedLength(), QTextCursor::KeepAnchor);
        cursor.deleteChar();
        selEnd -= 1;
    }

    // loop through all blocks
    while (cursor.blockNumber() < blockEnd)
    {
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        if (commentPattern.indexIn(cursor.block().text()) == -1)
        {
            cursor.insertText("#");
            selEnd += 1;
        }
        else
        {
            cursor.setPosition(cursor.position() + commentPattern.matchedLength(), QTextCursor::KeepAnchor);
            cursor.deleteChar();
            selEnd -= 1;
        }
    }

    // restore selection state
    cursor.setPosition(selStart, QTextCursor::MoveAnchor);
    cursor.setPosition(selEnd, QTextCursor::KeepAnchor);

    // update
    cursor.endEditBlock();
    setTextCursor(cursor);
}

void ScriptEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> selections;

    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(Qt::yellow).lighter(180);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }

    setExtraSelections(selections);

    matchParentheses('(', ')');
    // matchParentheses('[', ']');
    // matchParentheses('{', '}');
}

void ScriptEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    // line numbers
    int timesWidth = 0;
    int callWidth = 0;

    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            // line number
            QString lineNumber = QString::number(blockNumber + 1);

            // draw line number
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                             Qt::AlignRight, lineNumber);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void ScriptEditor::lineNumberAreaMouseMoveEvent(QMouseEvent *event)
{
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();

    int line = blockNumber + event->pos().y() / (int) blockBoundingRect(block).height() + 1;

    if (line <= document()->blockCount())
    {
        if (errorMessagesPyFlakes.contains(line))
            QToolTip::showText(event->globalPos(), errorMessagesPyFlakes[line]);
    }
}

int ScriptEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }


    int space = 15 + fontMetrics().maxWidth() * digits;

    return space;
}
