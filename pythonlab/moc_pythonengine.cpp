/****************************************************************************
** Meta object code from reading C++ file 'pythonengine.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "pythonengine.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pythonengine.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_PythonEngine_t {
    QByteArrayData data[9];
    char stringdata0[116];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PythonEngine_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PythonEngine_t qt_meta_stringdata_PythonEngine = {
    {
QT_MOC_LITERAL(0, 0, 12), // "PythonEngine"
QT_MOC_LITERAL(1, 13, 11), // "pythonClear"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 17), // "pythonShowMessage"
QT_MOC_LITERAL(4, 44, 14), // "pythonShowHtml"
QT_MOC_LITERAL(5, 59, 15), // "pythonShowImage"
QT_MOC_LITERAL(6, 75, 14), // "executedScript"
QT_MOC_LITERAL(7, 90, 13), // "startedScript"
QT_MOC_LITERAL(8, 104, 11) // "abortScript"

    },
    "PythonEngine\0pythonClear\0\0pythonShowMessage\0"
    "pythonShowHtml\0pythonShowImage\0"
    "executedScript\0startedScript\0abortScript"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PythonEngine[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x06 /* Public */,
       3,    1,   50,    2, 0x06 /* Public */,
       4,    1,   53,    2, 0x06 /* Public */,
       5,    3,   56,    2, 0x06 /* Public */,
       6,    0,   63,    2, 0x06 /* Public */,
       7,    0,   64,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    0,   65,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString, QMetaType::Int, QMetaType::Int,    2,    2,    2,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void PythonEngine::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        PythonEngine *_t = static_cast<PythonEngine *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->pythonClear(); break;
        case 1: _t->pythonShowMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->pythonShowHtml((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->pythonShowImage((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 4: _t->executedScript(); break;
        case 5: _t->startedScript(); break;
        case 6: _t->abortScript(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (PythonEngine::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&PythonEngine::pythonClear)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (PythonEngine::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&PythonEngine::pythonShowMessage)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (PythonEngine::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&PythonEngine::pythonShowHtml)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (PythonEngine::*_t)(const QString & , int , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&PythonEngine::pythonShowImage)) {
                *result = 3;
                return;
            }
        }
        {
            typedef void (PythonEngine::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&PythonEngine::executedScript)) {
                *result = 4;
                return;
            }
        }
        {
            typedef void (PythonEngine::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&PythonEngine::startedScript)) {
                *result = 5;
                return;
            }
        }
    }
}

const QMetaObject PythonEngine::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_PythonEngine.data,
      qt_meta_data_PythonEngine,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *PythonEngine::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PythonEngine::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_PythonEngine.stringdata0))
        return static_cast<void*>(const_cast< PythonEngine*>(this));
    if (!strcmp(_clname, "PythonEngineProfiler"))
        return static_cast< PythonEngineProfiler*>(const_cast< PythonEngine*>(this));
    return QObject::qt_metacast(_clname);
}

int PythonEngine::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void PythonEngine::pythonClear()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void PythonEngine::pythonShowMessage(const QString & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PythonEngine::pythonShowHtml(const QString & _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PythonEngine::pythonShowImage(const QString & _t1, int _t2, int _t3)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PythonEngine::executedScript()
{
    QMetaObject::activate(this, &staticMetaObject, 4, Q_NULLPTR);
}

// SIGNAL 5
void PythonEngine::startedScript()
{
    QMetaObject::activate(this, &staticMetaObject, 5, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
