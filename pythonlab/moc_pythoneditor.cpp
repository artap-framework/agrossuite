/****************************************************************************
** Meta object code from reading C++ file 'pythoneditor.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "pythoneditor.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pythoneditor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_PythonEditorTextEdit_t {
    QByteArrayData data[9];
    char stringdata0[116];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PythonEditorTextEdit_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PythonEditorTextEdit_t qt_meta_stringdata_PythonEditorTextEdit = {
    {
QT_MOC_LITERAL(0, 0, 20), // "PythonEditorTextEdit"
QT_MOC_LITERAL(1, 21, 13), // "pyLintAnalyse"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 15), // "pyFlakesAnalyse"
QT_MOC_LITERAL(4, 52, 20), // "pyLintAnalyseStopped"
QT_MOC_LITERAL(5, 73, 15), // "doHighlightLine"
QT_MOC_LITERAL(6, 89, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(7, 106, 4), // "item"
QT_MOC_LITERAL(8, 111, 4) // "role"

    },
    "PythonEditorTextEdit\0pyLintAnalyse\0\0"
    "pyFlakesAnalyse\0pyLintAnalyseStopped\0"
    "doHighlightLine\0QTreeWidgetItem*\0item\0"
    "role"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PythonEditorTextEdit[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x0a /* Public */,
       3,    0,   35,    2, 0x0a /* Public */,
       4,    0,   36,    2, 0x08 /* Private */,
       5,    2,   37,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6, QMetaType::Int,    7,    8,

       0        // eod
};

void PythonEditorTextEdit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        PythonEditorTextEdit *_t = static_cast<PythonEditorTextEdit *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->pyLintAnalyse(); break;
        case 1: _t->pyFlakesAnalyse(); break;
        case 2: _t->pyLintAnalyseStopped(); break;
        case 3: _t->doHighlightLine((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject PythonEditorTextEdit::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_PythonEditorTextEdit.data,
      qt_meta_data_PythonEditorTextEdit,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *PythonEditorTextEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PythonEditorTextEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_PythonEditorTextEdit.stringdata0))
        return static_cast<void*>(const_cast< PythonEditorTextEdit*>(this));
    return QWidget::qt_metacast(_clname);
}

int PythonEditorTextEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
struct qt_meta_stringdata_PythonEditorWidget_t {
    QByteArrayData data[1];
    char stringdata0[19];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PythonEditorWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PythonEditorWidget_t qt_meta_stringdata_PythonEditorWidget = {
    {
QT_MOC_LITERAL(0, 0, 18) // "PythonEditorWidget"

    },
    "PythonEditorWidget"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PythonEditorWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void PythonEditorWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObject PythonEditorWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_PythonEditorWidget.data,
      qt_meta_data_PythonEditorWidget,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *PythonEditorWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PythonEditorWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_PythonEditorWidget.stringdata0))
        return static_cast<void*>(const_cast< PythonEditorWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int PythonEditorWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
struct qt_meta_stringdata_PythonEditorDialog_t {
    QByteArrayData data[49];
    char stringdata0[651];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PythonEditorDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PythonEditorDialog_t qt_meta_stringdata_PythonEditorDialog = {
    {
QT_MOC_LITERAL(0, 0, 18), // "PythonEditorDialog"
QT_MOC_LITERAL(1, 19, 9), // "doFileNew"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 10), // "doFileOpen"
QT_MOC_LITERAL(4, 41, 4), // "file"
QT_MOC_LITERAL(5, 46, 10), // "doFileSave"
QT_MOC_LITERAL(6, 57, 12), // "doFileSaveAs"
QT_MOC_LITERAL(7, 70, 11), // "doFileClose"
QT_MOC_LITERAL(8, 82, 16), // "doFileOpenRecent"
QT_MOC_LITERAL(9, 99, 8), // "QAction*"
QT_MOC_LITERAL(10, 108, 6), // "action"
QT_MOC_LITERAL(11, 115, 11), // "doFilePrint"
QT_MOC_LITERAL(12, 127, 6), // "doFind"
QT_MOC_LITERAL(13, 134, 10), // "doFindNext"
QT_MOC_LITERAL(14, 145, 9), // "doReplace"
QT_MOC_LITERAL(15, 155, 13), // "doDataChanged"
QT_MOC_LITERAL(16, 169, 12), // "doHelpOnWord"
QT_MOC_LITERAL(17, 182, 16), // "doGotoDefinition"
QT_MOC_LITERAL(18, 199, 16), // "doPrintSelection"
QT_MOC_LITERAL(19, 216, 10), // "doCloseTab"
QT_MOC_LITERAL(20, 227, 5), // "index"
QT_MOC_LITERAL(21, 233, 11), // "doRunPython"
QT_MOC_LITERAL(22, 245, 23), // "doCreatePythonFromModel"
QT_MOC_LITERAL(23, 269, 17), // "doFileOpenAndFind"
QT_MOC_LITERAL(24, 287, 4), // "find"
QT_MOC_LITERAL(25, 292, 22), // "onOtherInstanceMessage"
QT_MOC_LITERAL(26, 315, 3), // "msg"
QT_MOC_LITERAL(27, 319, 12), // "doStopScript"
QT_MOC_LITERAL(28, 332, 23), // "doReplaceTabsWithSpaces"
QT_MOC_LITERAL(29, 356, 14), // "doPyLintPython"
QT_MOC_LITERAL(30, 371, 19), // "doGotoFileDirectory"
QT_MOC_LITERAL(31, 391, 21), // "doFileItemDoubleClick"
QT_MOC_LITERAL(32, 413, 4), // "path"
QT_MOC_LITERAL(33, 418, 15), // "doPathChangeDir"
QT_MOC_LITERAL(34, 434, 24), // "doCurrentDocumentChanged"
QT_MOC_LITERAL(35, 459, 7), // "changed"
QT_MOC_LITERAL(36, 467, 20), // "doCurrentPageChanged"
QT_MOC_LITERAL(37, 488, 23), // "doCursorPositionChanged"
QT_MOC_LITERAL(38, 512, 15), // "doStartedScript"
QT_MOC_LITERAL(39, 528, 16), // "doExecutedScript"
QT_MOC_LITERAL(40, 545, 13), // "doUseProfiler"
QT_MOC_LITERAL(41, 559, 15), // "doConsoleOutput"
QT_MOC_LITERAL(42, 575, 12), // "printHeading"
QT_MOC_LITERAL(43, 588, 7), // "message"
QT_MOC_LITERAL(44, 596, 12), // "printMessage"
QT_MOC_LITERAL(45, 609, 6), // "module"
QT_MOC_LITERAL(46, 616, 10), // "printError"
QT_MOC_LITERAL(47, 627, 12), // "printWarning"
QT_MOC_LITERAL(48, 640, 10) // "printDebug"

    },
    "PythonEditorDialog\0doFileNew\0\0doFileOpen\0"
    "file\0doFileSave\0doFileSaveAs\0doFileClose\0"
    "doFileOpenRecent\0QAction*\0action\0"
    "doFilePrint\0doFind\0doFindNext\0doReplace\0"
    "doDataChanged\0doHelpOnWord\0doGotoDefinition\0"
    "doPrintSelection\0doCloseTab\0index\0"
    "doRunPython\0doCreatePythonFromModel\0"
    "doFileOpenAndFind\0find\0onOtherInstanceMessage\0"
    "msg\0doStopScript\0doReplaceTabsWithSpaces\0"
    "doPyLintPython\0doGotoFileDirectory\0"
    "doFileItemDoubleClick\0path\0doPathChangeDir\0"
    "doCurrentDocumentChanged\0changed\0"
    "doCurrentPageChanged\0doCursorPositionChanged\0"
    "doStartedScript\0doExecutedScript\0"
    "doUseProfiler\0doConsoleOutput\0"
    "printHeading\0message\0printMessage\0"
    "module\0printError\0printWarning\0"
    "printDebug"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PythonEditorDialog[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      38,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  204,    2, 0x0a /* Public */,
       3,    1,  205,    2, 0x0a /* Public */,
       3,    0,  208,    2, 0x2a /* Public | MethodCloned */,
       5,    0,  209,    2, 0x0a /* Public */,
       6,    0,  210,    2, 0x0a /* Public */,
       7,    0,  211,    2, 0x0a /* Public */,
       8,    1,  212,    2, 0x0a /* Public */,
      11,    0,  215,    2, 0x0a /* Public */,
      12,    0,  216,    2, 0x0a /* Public */,
      13,    0,  217,    2, 0x0a /* Public */,
      14,    0,  218,    2, 0x0a /* Public */,
      15,    0,  219,    2, 0x0a /* Public */,
      16,    0,  220,    2, 0x0a /* Public */,
      17,    0,  221,    2, 0x0a /* Public */,
      18,    0,  222,    2, 0x0a /* Public */,
      19,    1,  223,    2, 0x0a /* Public */,
      21,    0,  226,    2, 0x0a /* Public */,
      22,    0,  227,    2, 0x0a /* Public */,
      23,    2,  228,    2, 0x0a /* Public */,
      25,    1,  233,    2, 0x0a /* Public */,
      27,    0,  236,    2, 0x08 /* Private */,
      28,    0,  237,    2, 0x08 /* Private */,
      29,    0,  238,    2, 0x08 /* Private */,
      30,    0,  239,    2, 0x08 /* Private */,
      31,    1,  240,    2, 0x08 /* Private */,
      33,    0,  243,    2, 0x08 /* Private */,
      34,    1,  244,    2, 0x08 /* Private */,
      36,    1,  247,    2, 0x08 /* Private */,
      37,    0,  250,    2, 0x08 /* Private */,
      38,    0,  251,    2, 0x08 /* Private */,
      39,    0,  252,    2, 0x08 /* Private */,
      40,    0,  253,    2, 0x08 /* Private */,
      41,    0,  254,    2, 0x08 /* Private */,
      42,    1,  255,    2, 0x08 /* Private */,
      44,    2,  258,    2, 0x08 /* Private */,
      46,    2,  263,    2, 0x08 /* Private */,
      47,    2,  268,    2, 0x08 /* Private */,
      48,    2,  273,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   20,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    4,   24,
    QMetaType::Void, QMetaType::QString,   26,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   32,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   35,
    QMetaType::Void, QMetaType::Int,   20,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   43,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   45,   43,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   45,   43,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   45,   43,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   45,   43,

       0        // eod
};

void PythonEditorDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        PythonEditorDialog *_t = static_cast<PythonEditorDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->doFileNew(); break;
        case 1: _t->doFileOpen((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->doFileOpen(); break;
        case 3: _t->doFileSave(); break;
        case 4: _t->doFileSaveAs(); break;
        case 5: _t->doFileClose(); break;
        case 6: _t->doFileOpenRecent((*reinterpret_cast< QAction*(*)>(_a[1]))); break;
        case 7: _t->doFilePrint(); break;
        case 8: _t->doFind(); break;
        case 9: _t->doFindNext(); break;
        case 10: _t->doReplace(); break;
        case 11: _t->doDataChanged(); break;
        case 12: _t->doHelpOnWord(); break;
        case 13: _t->doGotoDefinition(); break;
        case 14: _t->doPrintSelection(); break;
        case 15: _t->doCloseTab((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->doRunPython(); break;
        case 17: _t->doCreatePythonFromModel(); break;
        case 18: _t->doFileOpenAndFind((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 19: _t->onOtherInstanceMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 20: _t->doStopScript(); break;
        case 21: _t->doReplaceTabsWithSpaces(); break;
        case 22: _t->doPyLintPython(); break;
        case 23: _t->doGotoFileDirectory(); break;
        case 24: _t->doFileItemDoubleClick((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 25: _t->doPathChangeDir(); break;
        case 26: _t->doCurrentDocumentChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 27: _t->doCurrentPageChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 28: _t->doCursorPositionChanged(); break;
        case 29: _t->doStartedScript(); break;
        case 30: _t->doExecutedScript(); break;
        case 31: _t->doUseProfiler(); break;
        case 32: _t->doConsoleOutput(); break;
        case 33: _t->printHeading((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 34: _t->printMessage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 35: _t->printError((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 36: _t->printWarning((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 37: _t->printDebug((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject PythonEditorDialog::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_PythonEditorDialog.data,
      qt_meta_data_PythonEditorDialog,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *PythonEditorDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PythonEditorDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_PythonEditorDialog.stringdata0))
        return static_cast<void*>(const_cast< PythonEditorDialog*>(this));
    return QWidget::qt_metacast(_clname);
}

int PythonEditorDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 38)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 38;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 38)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 38;
    }
    return _id;
}
struct qt_meta_stringdata_ScriptEditor_t {
    QByteArrayData data[15];
    char stringdata0[231];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ScriptEditor_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ScriptEditor_t qt_meta_stringdata_ScriptEditor = {
    {
QT_MOC_LITERAL(0, 0, 12), // "ScriptEditor"
QT_MOC_LITERAL(1, 13, 8), // "gotoLine"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 4), // "line"
QT_MOC_LITERAL(4, 28, 7), // "isError"
QT_MOC_LITERAL(5, 36, 25), // "updateLineNumberAreaWidth"
QT_MOC_LITERAL(6, 62, 20), // "highlightCurrentLine"
QT_MOC_LITERAL(7, 83, 20), // "updateLineNumberArea"
QT_MOC_LITERAL(8, 104, 15), // "indentSelection"
QT_MOC_LITERAL(9, 120, 17), // "unindentSelection"
QT_MOC_LITERAL(10, 138, 26), // "indentAndUnindentSelection"
QT_MOC_LITERAL(11, 165, 8), // "doIndent"
QT_MOC_LITERAL(12, 174, 28), // "commentAndUncommentSelection"
QT_MOC_LITERAL(13, 203, 16), // "insertCompletion"
QT_MOC_LITERAL(14, 220, 10) // "completion"

    },
    "ScriptEditor\0gotoLine\0\0line\0isError\0"
    "updateLineNumberAreaWidth\0"
    "highlightCurrentLine\0updateLineNumberArea\0"
    "indentSelection\0unindentSelection\0"
    "indentAndUnindentSelection\0doIndent\0"
    "commentAndUncommentSelection\0"
    "insertCompletion\0completion"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ScriptEditor[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   74,    2, 0x0a /* Public */,
       1,    1,   79,    2, 0x2a /* Public | MethodCloned */,
       1,    0,   82,    2, 0x2a /* Public | MethodCloned */,
       5,    0,   83,    2, 0x08 /* Private */,
       6,    1,   84,    2, 0x08 /* Private */,
       6,    0,   87,    2, 0x28 /* Private | MethodCloned */,
       7,    2,   88,    2, 0x08 /* Private */,
       8,    0,   93,    2, 0x08 /* Private */,
       9,    0,   94,    2, 0x08 /* Private */,
      10,    1,   95,    2, 0x08 /* Private */,
      12,    0,   98,    2, 0x08 /* Private */,
      13,    1,   99,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    3,    4,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QRect, QMetaType::Int,    2,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   11,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   14,

       0        // eod
};

void ScriptEditor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ScriptEditor *_t = static_cast<ScriptEditor *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->gotoLine((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->gotoLine((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->gotoLine(); break;
        case 3: _t->updateLineNumberAreaWidth(); break;
        case 4: _t->highlightCurrentLine((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->highlightCurrentLine(); break;
        case 6: _t->updateLineNumberArea((*reinterpret_cast< const QRect(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 7: _t->indentSelection(); break;
        case 8: _t->unindentSelection(); break;
        case 9: _t->indentAndUnindentSelection((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->commentAndUncommentSelection(); break;
        case 11: _t->insertCompletion((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject ScriptEditor::staticMetaObject = {
    { &PlainTextEditParenthesis::staticMetaObject, qt_meta_stringdata_ScriptEditor.data,
      qt_meta_data_ScriptEditor,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ScriptEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ScriptEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ScriptEditor.stringdata0))
        return static_cast<void*>(const_cast< ScriptEditor*>(this));
    return PlainTextEditParenthesis::qt_metacast(_clname);
}

int ScriptEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = PlainTextEditParenthesis::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}
struct qt_meta_stringdata_SearchWidget_t {
    QByteArrayData data[5];
    char stringdata0[45];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SearchWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SearchWidget_t qt_meta_stringdata_SearchWidget = {
    {
QT_MOC_LITERAL(0, 0, 12), // "SearchWidget"
QT_MOC_LITERAL(1, 13, 8), // "findNext"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 10), // "replaceAll"
QT_MOC_LITERAL(4, 34, 10) // "hideWidget"

    },
    "SearchWidget\0findNext\0\0replaceAll\0"
    "hideWidget"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SearchWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x0a /* Public */,
       3,    0,   30,    2, 0x0a /* Public */,
       4,    0,   31,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void SearchWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SearchWidget *_t = static_cast<SearchWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->findNext(); break;
        case 1: _t->replaceAll(); break;
        case 2: _t->hideWidget(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject SearchWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_SearchWidget.data,
      qt_meta_data_SearchWidget,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *SearchWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SearchWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_SearchWidget.stringdata0))
        return static_cast<void*>(const_cast< SearchWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int SearchWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
struct qt_meta_stringdata_ErrorWidget_t {
    QByteArrayData data[9];
    char stringdata0[90];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ErrorWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ErrorWidget_t qt_meta_stringdata_ErrorWidget = {
    {
QT_MOC_LITERAL(0, 0, 11), // "ErrorWidget"
QT_MOC_LITERAL(1, 12, 20), // "doHighlightLineError"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(4, 51, 4), // "item"
QT_MOC_LITERAL(5, 56, 4), // "role"
QT_MOC_LITERAL(6, 61, 9), // "showError"
QT_MOC_LITERAL(7, 71, 11), // "ErrorResult"
QT_MOC_LITERAL(8, 83, 6) // "result"

    },
    "ErrorWidget\0doHighlightLineError\0\0"
    "QTreeWidgetItem*\0item\0role\0showError\0"
    "ErrorResult\0result"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ErrorWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    2,   24,    2, 0x0a /* Public */,
       6,    1,   29,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void, 0x80000000 | 7,    8,

       0        // eod
};

void ErrorWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ErrorWidget *_t = static_cast<ErrorWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->doHighlightLineError((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 1: _t->showError((*reinterpret_cast< ErrorResult(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject ErrorWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ErrorWidget.data,
      qt_meta_data_ErrorWidget,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ErrorWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ErrorWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ErrorWidget.stringdata0))
        return static_cast<void*>(const_cast< ErrorWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int ErrorWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
