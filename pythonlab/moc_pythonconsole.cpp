/****************************************************************************
** Meta object code from reading C++ file 'pythonconsole.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "pythonconsole.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pythonconsole.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_PythonScriptingConsole_t {
    QByteArrayData data[30];
    char stringdata0[315];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PythonScriptingConsole_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PythonScriptingConsole_t qt_meta_stringdata_PythonScriptingConsole = {
    {
QT_MOC_LITERAL(0, 0, 22), // "PythonScriptingConsole"
QT_MOC_LITERAL(1, 23, 14), // "historyChanged"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 4), // "code"
QT_MOC_LITERAL(4, 44, 11), // "executeLine"
QT_MOC_LITERAL(5, 56, 9), // "storeOnly"
QT_MOC_LITERAL(6, 66, 3), // "str"
QT_MOC_LITERAL(7, 70, 13), // "keyPressEvent"
QT_MOC_LITERAL(8, 84, 10), // "QKeyEvent*"
QT_MOC_LITERAL(9, 95, 1), // "e"
QT_MOC_LITERAL(10, 97, 14), // "consoleMessage"
QT_MOC_LITERAL(11, 112, 7), // "message"
QT_MOC_LITERAL(12, 120, 5), // "color"
QT_MOC_LITERAL(13, 126, 16), // "clearCommandLine"
QT_MOC_LITERAL(14, 143, 14), // "welcomeMessage"
QT_MOC_LITERAL(15, 158, 3), // "cut"
QT_MOC_LITERAL(16, 162, 8), // "stdClear"
QT_MOC_LITERAL(17, 171, 6), // "stdOut"
QT_MOC_LITERAL(18, 178, 6), // "stdErr"
QT_MOC_LITERAL(19, 185, 7), // "stdHtml"
QT_MOC_LITERAL(20, 193, 8), // "stdImage"
QT_MOC_LITERAL(21, 202, 8), // "fileName"
QT_MOC_LITERAL(22, 211, 5), // "width"
QT_MOC_LITERAL(23, 217, 6), // "height"
QT_MOC_LITERAL(24, 224, 16), // "insertCompletion"
QT_MOC_LITERAL(25, 241, 10), // "completion"
QT_MOC_LITERAL(26, 252, 19), // "appendCommandPrompt"
QT_MOC_LITERAL(27, 272, 13), // "connectStdOut"
QT_MOC_LITERAL(28, 286, 11), // "currentPath"
QT_MOC_LITERAL(29, 298, 16) // "disconnectStdOut"

    },
    "PythonScriptingConsole\0historyChanged\0"
    "\0code\0executeLine\0storeOnly\0str\0"
    "keyPressEvent\0QKeyEvent*\0e\0consoleMessage\0"
    "message\0color\0clearCommandLine\0"
    "welcomeMessage\0cut\0stdClear\0stdOut\0"
    "stdErr\0stdHtml\0stdImage\0fileName\0width\0"
    "height\0insertCompletion\0completion\0"
    "appendCommandPrompt\0connectStdOut\0"
    "currentPath\0disconnectStdOut"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PythonScriptingConsole[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      22,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  124,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,  127,    2, 0x0a /* Public */,
       4,    2,  130,    2, 0x0a /* Public */,
       7,    1,  135,    2, 0x0a /* Public */,
      10,    2,  138,    2, 0x0a /* Public */,
      10,    1,  143,    2, 0x2a /* Public | MethodCloned */,
      13,    0,  146,    2, 0x0a /* Public */,
      14,    0,  147,    2, 0x0a /* Public */,
      15,    0,  148,    2, 0x0a /* Public */,
      16,    0,  149,    2, 0x0a /* Public */,
      17,    1,  150,    2, 0x0a /* Public */,
      18,    1,  153,    2, 0x0a /* Public */,
      19,    1,  156,    2, 0x0a /* Public */,
      20,    3,  159,    2, 0x0a /* Public */,
      20,    2,  166,    2, 0x2a /* Public | MethodCloned */,
      20,    1,  171,    2, 0x2a /* Public | MethodCloned */,
      24,    1,  174,    2, 0x0a /* Public */,
      26,    1,  177,    2, 0x0a /* Public */,
      26,    0,  180,    2, 0x2a /* Public | MethodCloned */,
      27,    1,  181,    2, 0x0a /* Public */,
      27,    0,  184,    2, 0x2a /* Public | MethodCloned */,
      29,    0,  185,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    6,    5,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, QMetaType::QString, QMetaType::QColor,   11,   12,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::QString, QMetaType::Int, QMetaType::Int,   21,   22,   23,
    QMetaType::Void, QMetaType::QString, QMetaType::Int,   21,   22,
    QMetaType::Void, QMetaType::QString,   21,
    QMetaType::Void, QMetaType::QString,   25,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   28,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void PythonScriptingConsole::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        PythonScriptingConsole *_t = static_cast<PythonScriptingConsole *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->historyChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->executeLine((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->executeLine((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 3: _t->keyPressEvent((*reinterpret_cast< QKeyEvent*(*)>(_a[1]))); break;
        case 4: _t->consoleMessage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QColor(*)>(_a[2]))); break;
        case 5: _t->consoleMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->clearCommandLine(); break;
        case 7: _t->welcomeMessage(); break;
        case 8: _t->cut(); break;
        case 9: _t->stdClear(); break;
        case 10: _t->stdOut((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 11: _t->stdErr((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: _t->stdHtml((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 13: _t->stdImage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 14: _t->stdImage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 15: _t->stdImage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 16: _t->insertCompletion((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 17: _t->appendCommandPrompt((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 18: _t->appendCommandPrompt(); break;
        case 19: _t->connectStdOut((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 20: _t->connectStdOut(); break;
        case 21: _t->disconnectStdOut(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (PythonScriptingConsole::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&PythonScriptingConsole::historyChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject PythonScriptingConsole::staticMetaObject = {
    { &QTextEdit::staticMetaObject, qt_meta_stringdata_PythonScriptingConsole.data,
      qt_meta_data_PythonScriptingConsole,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *PythonScriptingConsole::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PythonScriptingConsole::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_PythonScriptingConsole.stringdata0))
        return static_cast<void*>(const_cast< PythonScriptingConsole*>(this));
    return QTextEdit::qt_metacast(_clname);
}

int PythonScriptingConsole::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTextEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 22)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 22;
    }
    return _id;
}

// SIGNAL 0
void PythonScriptingConsole::historyChanged(const QString & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_PythonScriptingHistory_t {
    QByteArrayData data[8];
    char stringdata0[86];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PythonScriptingHistory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PythonScriptingHistory_t qt_meta_stringdata_PythonScriptingHistory = {
    {
QT_MOC_LITERAL(0, 0, 22), // "PythonScriptingHistory"
QT_MOC_LITERAL(1, 23, 14), // "historyChanged"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 4), // "code"
QT_MOC_LITERAL(4, 44, 14), // "executeCommand"
QT_MOC_LITERAL(5, 59, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(6, 76, 4), // "item"
QT_MOC_LITERAL(7, 81, 4) // "role"

    },
    "PythonScriptingHistory\0historyChanged\0"
    "\0code\0executeCommand\0QTreeWidgetItem*\0"
    "item\0role"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PythonScriptingHistory[] = {

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
       1,    1,   24,    2, 0x08 /* Private */,
       4,    2,   27,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, 0x80000000 | 5, QMetaType::Int,    6,    7,

       0        // eod
};

void PythonScriptingHistory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        PythonScriptingHistory *_t = static_cast<PythonScriptingHistory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->historyChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->executeCommand((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject PythonScriptingHistory::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_PythonScriptingHistory.data,
      qt_meta_data_PythonScriptingHistory,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *PythonScriptingHistory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PythonScriptingHistory::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_PythonScriptingHistory.stringdata0))
        return static_cast<void*>(const_cast< PythonScriptingHistory*>(this));
    return QWidget::qt_metacast(_clname);
}

int PythonScriptingHistory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
