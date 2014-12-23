/****************************************************************************
** Meta object code from reading C++ file 'sshconnection_p.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.3.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sshconnection_p.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sshconnection_p.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.3.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QSsh__Internal__SshConnectionPrivate_t {
    QByteArrayData data[14];
    char stringdata[222];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QSsh__Internal__SshConnectionPrivate_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QSsh__Internal__SshConnectionPrivate_t qt_meta_stringdata_QSsh__Internal__SshConnectionPrivate = {
    {
QT_MOC_LITERAL(0, 0, 36),
QT_MOC_LITERAL(1, 37, 9),
QT_MOC_LITERAL(2, 47, 0),
QT_MOC_LITERAL(3, 48, 12),
QT_MOC_LITERAL(4, 61, 13),
QT_MOC_LITERAL(5, 75, 7),
QT_MOC_LITERAL(6, 83, 5),
QT_MOC_LITERAL(7, 89, 14),
QT_MOC_LITERAL(8, 104, 21),
QT_MOC_LITERAL(9, 126, 18),
QT_MOC_LITERAL(10, 145, 17),
QT_MOC_LITERAL(11, 163, 24),
QT_MOC_LITERAL(12, 188, 13),
QT_MOC_LITERAL(13, 202, 19)
    },
    "QSsh::Internal::SshConnectionPrivate\0"
    "connected\0\0disconnected\0dataAvailable\0"
    "message\0error\0QSsh::SshError\0"
    "handleSocketConnected\0handleIncomingData\0"
    "handleSocketError\0handleSocketDisconnected\0"
    "handleTimeout\0sendKeepAlivePacket"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QSsh__Internal__SshConnectionPrivate[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x06 /* Public */,
       3,    0,   65,    2, 0x06 /* Public */,
       4,    1,   66,    2, 0x06 /* Public */,
       6,    1,   69,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    0,   72,    2, 0x08 /* Private */,
       9,    0,   73,    2, 0x08 /* Private */,
      10,    0,   74,    2, 0x08 /* Private */,
      11,    0,   75,    2, 0x08 /* Private */,
      12,    0,   76,    2, 0x08 /* Private */,
      13,    0,   77,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, 0x80000000 | 7,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void QSsh::Internal::SshConnectionPrivate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SshConnectionPrivate *_t = static_cast<SshConnectionPrivate *>(_o);
        switch (_id) {
        case 0: _t->connected(); break;
        case 1: _t->disconnected(); break;
        case 2: _t->dataAvailable((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->error((*reinterpret_cast< QSsh::SshError(*)>(_a[1]))); break;
        case 4: _t->handleSocketConnected(); break;
        case 5: _t->handleIncomingData(); break;
        case 6: _t->handleSocketError(); break;
        case 7: _t->handleSocketDisconnected(); break;
        case 8: _t->handleTimeout(); break;
        case 9: _t->sendKeepAlivePacket(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (SshConnectionPrivate::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshConnectionPrivate::connected)) {
                *result = 0;
            }
        }
        {
            typedef void (SshConnectionPrivate::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshConnectionPrivate::disconnected)) {
                *result = 1;
            }
        }
        {
            typedef void (SshConnectionPrivate::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshConnectionPrivate::dataAvailable)) {
                *result = 2;
            }
        }
        {
            typedef void (SshConnectionPrivate::*_t)(QSsh::SshError );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshConnectionPrivate::error)) {
                *result = 3;
            }
        }
    }
}

const QMetaObject QSsh::Internal::SshConnectionPrivate::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_QSsh__Internal__SshConnectionPrivate.data,
      qt_meta_data_QSsh__Internal__SshConnectionPrivate,  qt_static_metacall, 0, 0}
};


const QMetaObject *QSsh::Internal::SshConnectionPrivate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QSsh::Internal::SshConnectionPrivate::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QSsh__Internal__SshConnectionPrivate.stringdata))
        return static_cast<void*>(const_cast< SshConnectionPrivate*>(this));
    return QObject::qt_metacast(_clname);
}

int QSsh::Internal::SshConnectionPrivate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void QSsh::Internal::SshConnectionPrivate::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void QSsh::Internal::SshConnectionPrivate::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void QSsh::Internal::SshConnectionPrivate::dataAvailable(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QSsh::Internal::SshConnectionPrivate::error(QSsh::SshError _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE
