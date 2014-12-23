/****************************************************************************
** Meta object code from reading C++ file 'sshremoteprocess.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.3.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sshremoteprocess.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sshremoteprocess.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.3.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QSsh__SshRemoteProcess_t {
    QByteArrayData data[7];
    char stringdata[97];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QSsh__SshRemoteProcess_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QSsh__SshRemoteProcess_t qt_meta_stringdata_QSsh__SshRemoteProcess = {
    {
QT_MOC_LITERAL(0, 0, 22),
QT_MOC_LITERAL(1, 23, 7),
QT_MOC_LITERAL(2, 31, 0),
QT_MOC_LITERAL(3, 32, 23),
QT_MOC_LITERAL(4, 56, 22),
QT_MOC_LITERAL(5, 79, 6),
QT_MOC_LITERAL(6, 86, 10)
    },
    "QSsh::SshRemoteProcess\0started\0\0"
    "readyReadStandardOutput\0readyReadStandardError\0"
    "closed\0exitStatus"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QSsh__SshRemoteProcess[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x06 /* Public */,
       3,    0,   35,    2, 0x06 /* Public */,
       4,    0,   36,    2, 0x06 /* Public */,
       5,    1,   37,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    6,

       0        // eod
};

void QSsh::SshRemoteProcess::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SshRemoteProcess *_t = static_cast<SshRemoteProcess *>(_o);
        switch (_id) {
        case 0: _t->started(); break;
        case 1: _t->readyReadStandardOutput(); break;
        case 2: _t->readyReadStandardError(); break;
        case 3: _t->closed((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (SshRemoteProcess::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshRemoteProcess::started)) {
                *result = 0;
            }
        }
        {
            typedef void (SshRemoteProcess::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshRemoteProcess::readyReadStandardOutput)) {
                *result = 1;
            }
        }
        {
            typedef void (SshRemoteProcess::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshRemoteProcess::readyReadStandardError)) {
                *result = 2;
            }
        }
        {
            typedef void (SshRemoteProcess::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshRemoteProcess::closed)) {
                *result = 3;
            }
        }
    }
}

const QMetaObject QSsh::SshRemoteProcess::staticMetaObject = {
    { &QIODevice::staticMetaObject, qt_meta_stringdata_QSsh__SshRemoteProcess.data,
      qt_meta_data_QSsh__SshRemoteProcess,  qt_static_metacall, 0, 0}
};


const QMetaObject *QSsh::SshRemoteProcess::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QSsh::SshRemoteProcess::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QSsh__SshRemoteProcess.stringdata))
        return static_cast<void*>(const_cast< SshRemoteProcess*>(this));
    return QIODevice::qt_metacast(_clname);
}

int QSsh::SshRemoteProcess::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QIODevice::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void QSsh::SshRemoteProcess::started()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void QSsh::SshRemoteProcess::readyReadStandardOutput()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void QSsh::SshRemoteProcess::readyReadStandardError()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void QSsh::SshRemoteProcess::closed(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
