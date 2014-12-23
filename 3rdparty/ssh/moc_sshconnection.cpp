/****************************************************************************
** Meta object code from reading C++ file 'sshconnection.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.3.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sshconnection.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sshconnection.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.3.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QSsh__SshConnection_t {
    QByteArrayData data[8];
    char stringdata[87];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QSsh__SshConnection_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QSsh__SshConnection_t qt_meta_stringdata_QSsh__SshConnection = {
    {
QT_MOC_LITERAL(0, 0, 19),
QT_MOC_LITERAL(1, 20, 9),
QT_MOC_LITERAL(2, 30, 0),
QT_MOC_LITERAL(3, 31, 12),
QT_MOC_LITERAL(4, 44, 13),
QT_MOC_LITERAL(5, 58, 7),
QT_MOC_LITERAL(6, 66, 5),
QT_MOC_LITERAL(7, 72, 14)
    },
    "QSsh::SshConnection\0connected\0\0"
    "disconnected\0dataAvailable\0message\0"
    "error\0QSsh::SshError"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QSsh__SshConnection[] = {

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
       4,    1,   36,    2, 0x06 /* Public */,
       6,    1,   39,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, 0x80000000 | 7,    2,

       0        // eod
};

void QSsh::SshConnection::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SshConnection *_t = static_cast<SshConnection *>(_o);
        switch (_id) {
        case 0: _t->connected(); break;
        case 1: _t->disconnected(); break;
        case 2: _t->dataAvailable((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->error((*reinterpret_cast< QSsh::SshError(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (SshConnection::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshConnection::connected)) {
                *result = 0;
            }
        }
        {
            typedef void (SshConnection::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshConnection::disconnected)) {
                *result = 1;
            }
        }
        {
            typedef void (SshConnection::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshConnection::dataAvailable)) {
                *result = 2;
            }
        }
        {
            typedef void (SshConnection::*_t)(QSsh::SshError );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshConnection::error)) {
                *result = 3;
            }
        }
    }
}

const QMetaObject QSsh::SshConnection::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QSsh__SshConnection.data,
      qt_meta_data_QSsh__SshConnection,  qt_static_metacall, 0, 0}
};


const QMetaObject *QSsh::SshConnection::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QSsh::SshConnection::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QSsh__SshConnection.stringdata))
        return static_cast<void*>(const_cast< SshConnection*>(this));
    return QObject::qt_metacast(_clname);
}

int QSsh::SshConnection::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void QSsh::SshConnection::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void QSsh::SshConnection::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void QSsh::SshConnection::dataAvailable(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QSsh::SshConnection::error(QSsh::SshError _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
