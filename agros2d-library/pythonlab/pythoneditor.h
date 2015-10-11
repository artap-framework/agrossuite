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

#ifndef SCRIPTEDITORDIALOG_H
#define SCRIPTEDITORDIALOG_H

#include <QtGui>
#include <QtCore>

#include "util.h"
#include "gui/textedit.h"

#include "pythonengine.h"

class PythonEngine;
class PythonScriptingConsole;
class PythonScriptingConsoleView;
class PythonScriptingHistory;
class PythonBrowser;
class PythonEditorDialog;

class SceneView;
class FileBrowser;

class Scene;
class SceneView;
class ScriptEditor;
class SearchDialog;
class SearchWidget;
class SelectionWidget;
class ErrorWidget;

#ifdef Q_WS_X11
    const QFont FONT = QFont("Monospace", 9);
#endif
#ifdef Q_WS_WIN
    const QFont FONT = QFont("Courier New", 9);
#endif
#ifdef Q_WS_MAC
    const QFont FONT = QFont("Monaco", 12);
#endif

class AGROS_LIBRARY_API PythonEditorTextEdit : public QWidget
{
    Q_OBJECT
public:    
    ScriptEditor *txtEditor;
    QTreeWidget *trvPyLint;
    SearchWidget *searchWidget;
    QSplitter *splitter;

    PythonEditorTextEdit(PythonEngine *m_pythonEngine, QWidget *parent);
    ~PythonEditorTextEdit();

    inline QString fileName() { return m_fileName; }
    inline void setFileName(const QString &fileName) { m_fileName = QFileInfo(fileName).absoluteFilePath(); }

public slots:
    void pyLintAnalyse();
    void pyFlakesAnalyse();

private:
    QString m_fileName;
    PythonEngine *m_pythonEngine;
    PythonScriptingConsole *m_console;

private slots:
    void pyLintAnalyseStopped(int exitCode);
    void doHighlightLine(QTreeWidgetItem *item, int role);       
};

class AGROS_LIBRARY_API PythonEditorWidget : public QWidget
{
    Q_OBJECT
public:
    PythonEditorWidget(PythonEditorDialog *parent);
    ~PythonEditorWidget();

    FileBrowser *fileBrowser;
    PythonScriptingHistory *consoleHistory;
    PythonBrowser *variableBrowser;
    QWidget *config;

    QToolBar *toolBar;

private:
    PythonEditorDialog *pythonEditor;

    QSplitter *splitter;

    void createControls();  
};

class AGROS_LIBRARY_API PythonEditorDialog : public QWidget
{
    Q_OBJECT
public:
    PythonEditorDialog(PythonScriptingConsole *console, QWidget *parent = 0);
    ~PythonEditorDialog();

    void showDialog();
    void closeTabs();
    bool isScriptModified();

    QStringList *recentFiles() { return &m_recentFiles; }

    QAction *actSceneModePythonEditor;
    inline PythonEditorWidget *pythonEditorWidget() { return m_pythonEditorWidget; }

    QMenu *mnuFile;
    QMenu *mnuRecentFiles;
    QMenu *mnuEdit;
    QMenu *mnuTools;

public slots:
    void doFileNew();
    void doFileOpen(const QString &file = QString());
    void doFileSave();
    void doFileSaveAs();
    void doFileClose();
    void doFileOpenRecent(QAction *action);
    void doFilePrint();

    void doFind();
    void doFindNext();
    void doReplace();

    void doDataChanged();

    void doHelpOnWord();
    void doGotoDefinition();
    void doPrintSelection();

    void doCloseTab(int index);

    void doRunPython();
    void doCreatePythonFromModel();

    // message from another app
    void doFileOpenAndFind(const QString &file, const QString &find);
    void onOtherInstanceMessage(const QString &msg);

protected:
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);

    virtual void scriptPrepare();
    virtual void scriptFinish();

    PythonEngine *pythonEngine;

    QStringList m_recentFiles;

    // gui    
    PythonEditorWidget *m_pythonEditorWidget;
    ScriptEditor *txtEditor;
    ErrorWidget *errorWidget;

    QLabel *lblCurrentPosition;
    PythonScriptingConsole *m_console;

    QAction *actFileNew;
    QAction *actFileOpen;
    QAction *actFileSave;
    QAction *actFileSaveAs;
    QAction *actFileClose;
    QAction *actFilePrint;
    QAction *actExit;
    QActionGroup *actFileOpenRecentGroup;

    QAction *actUndo;
    QAction *actRedo;
    QAction *actCut;
    QAction *actCopy;
    QAction *actPaste;

    QAction *actFind;
    QAction *actFindNext;
    QAction *actReplace;

    QAction *actIndentSelection;
    QAction *actUnindentSelection;
    QAction *actCommentAndUncommentSelection;
    QAction *actGotoLine;
    QAction *actGotoToFileDirectory;

    QAction *actRunPython;
    QAction *actStopPython;
    QAction *actReplaceTabsWithSpaces;
    QAction *actCheckPyLint;    

    QAction *actHelpOnWord;
    QAction *actGotoDefinition;
    QAction *actPrintSelection;

    QAction *actUseProfiler;
    QAction *actPrintStacktrace;
    QAction *actConsoleOutput;

    QAction *actCreateFromModel;

    QTabWidget *tabWidget;

    void createActions();
    void createControls();
    void createStatusBar();

    void setRecentFiles();
    void setEnabledControls(bool state);

    inline PythonEditorTextEdit *scriptEditorWidget() { return dynamic_cast<PythonEditorTextEdit *>(tabWidget->currentWidget()); }

private slots:
    void doStopScript();
    void doReplaceTabsWithSpaces();
    void doPyLintPython();
    void doGotoFileDirectory();
    void doFileItemDoubleClick(const QString &path);
    void doPathChangeDir();
    void doCurrentDocumentChanged(bool changed);
    void doCurrentPageChanged(int index);
    void doCursorPositionChanged();

    void doStartedScript();
    void doExecutedScript();

    void doPrintStacktrace();
    void doUseProfiler();
    void doConsoleOutput();

    void printHeading(const QString &message);
    void printMessage(const QString &module, const QString &message);
    void printError(const QString &module, const QString &message);
    void printWarning(const QString &module, const QString &message);
    void printDebug(const QString &module, const QString &message);

    friend class PythonEditorWidget;
};

class AGROS_LIBRARY_API ScriptEditor : public PlainTextEditParenthesis
{
    Q_OBJECT

public:
    QMap<int, QString> errorMessagesPyFlakes;
    QMap<int, QString> errorMessagesError;

    ScriptEditor(PythonEngine *pythonEngine, QWidget *parent = 0);
    ~ScriptEditor();

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void lineNumberAreaMouseMoveEvent(QMouseEvent *event);
    int lineNumberAreaWidth();

    void replaceTabsWithSpaces();

    inline bool isProfiled() const { return m_isProfiled; }
    inline void setProfiled(bool prof) { m_isProfiled = prof; }

    inline QMap<int, int> profilerAccumulatedLines() const { return m_profilerAccumulatedLines; }
    inline void setProfilerAccumulatedLines(QMap<int, int> lines) { m_profilerAccumulatedLines = lines; }
    inline QMap<int, int> profilerAccumulatedTimes() const { return m_profilerAccumulatedTimes; }
    inline void setProfilerAccumulatedTimes(QMap<int, int> times) { m_profilerAccumulatedTimes = times; }

    inline int profilerMaxAccumulatedLine() const { return m_profilerMaxAccumulatedLine; }
    inline void setProfilerMaxAccumulatedLine(int val) { m_profilerMaxAccumulatedLine = val; }
    inline int profilerMaxAccumulatedTime() const { return m_profilerMaxAccumulatedTime; }
    inline void setProfilerMaxAccumulatedTime(int val) { m_profilerMaxAccumulatedTime = val; }
    inline int profilerMaxAccumulatedCallLine() const { return m_profilerMaxAccumulatedCallLine; }
    inline void setProfilerMaxAccumulatedCallLine(int val) { m_profilerMaxAccumulatedCallLine = val; }
    inline int profilerMaxAccumulatedCall() const { return m_profilerMaxAccumulatedCall; }
    inline void setProfilerMaxAccumulatedCall(int val) { m_profilerMaxAccumulatedCall = val; }

    inline bool isLineNumbersVisible() const { return m_isLineNumbersVisible; }
    void setLineNumbersVisible(bool visible) { m_isLineNumbersVisible = visible; updateLineNumberAreaWidth(); }

public slots:
    void gotoLine(int line = -1, bool isError = false);

protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void updateLineNumberAreaWidth();
    void highlightCurrentLine(bool isError = false);
    void updateLineNumberArea(const QRect &, int);

    void indentSelection();
    void unindentSelection();
    void indentAndUnindentSelection(bool doIndent);
    void commentAndUncommentSelection();

    void insertCompletion(const QString& completion);

private:
    PythonEngine *pythonEngine;
    QCompleter* completer;

    bool m_isLineNumbersVisible;
    QWidget *lineNumberArea;    

    // profiler
    bool m_isProfiled;

    QMap<int, int> m_profilerAccumulatedLines;
    QMap<int, int> m_profilerAccumulatedTimes;

    int m_profilerMaxAccumulatedLine;
    int m_profilerMaxAccumulatedTime;
    int m_profilerMaxAccumulatedCallLine;
    int m_profilerMaxAccumulatedCall;

    friend class PythonEditorDialog;
};

class AGROS_LIBRARY_API ScriptEditorLineNumberArea : public QWidget
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

// ************************************************************************************************************

class AGROS_LIBRARY_API SearchWidget: public QWidget
{
    Q_OBJECT
public:
    SearchWidget(ScriptEditor *txtEditor, QWidget *parent = 0);

    void showFind(const QString &text = "");
    void showReplaceAll(const QString &text = "");

public slots:
    void findNext();
    void replaceAll();
    void hideWidget();

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    ScriptEditor *txtEditor;

    QLabel *lblFind, *lblReplace;
    QLineEdit *txtFind, *txtReplace;
    QPushButton *btnFind, *btnReplace, *btnHide;
};

// ************************************************************************************************************

class AGROS_LIBRARY_API ErrorWidget: public QWidget
{
    Q_OBJECT
public:
    ErrorWidget(QTabWidget *tabWidget, QWidget *parent);

public slots:
    void doHighlightLineError(QTreeWidgetItem *item, int role);
    void showError(ErrorResult result);

private:
    QLabel *errorLabel;
    QTreeWidget *trvErrors;

    PythonEditorDialog *dialog;
    QTabWidget *tabWidget;
};

#endif // SCRIPTEDITORDIALOG_H