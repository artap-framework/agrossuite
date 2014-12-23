/****************************************************************************
** Meta object code from reading C++ file 'sshremoteprocess_p.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.3.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sshremoteprocess_p.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sshremoteprocess_p.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.3.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QSsh__Internal__SshRemoteProcessPrivate_t {
    QByteArrayData data[8];
    char stringdata[124];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QSsh__Internal__SshRemoteProcessPrivate_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QSsh__Internal__SshRemoteProcessPrivate_t qt_meta_stringdata_QSsh__Internal__SshRemoteProcessPrivate = {
    {
QT_MOC_LITERAL(0, 0, 39),
QT_MOC_LITERAL(1, 40, 7),
QT_MOC_LITERAL(2, 48, 0),
QT_MOC_LITERAL(3, 49, 9),
QT_MOC_LITERAL(4, 59, 23),
QT_MOC_LITERAL(5, 83, 22),
QT_MOC_LITERAL(6, 106, 6),
QT_MOC_LITERAL(7, 113, 10)
    },
    "QSsh::Internal::SshRemoteProcessPrivate\0"
    "started\0\0readyRead\0readyReadStandardOutput\0"
    "readyReadStandardError\0closed\0exitStatus"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QSsh__Internal__SshRemoteProcessPrivate[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   39,    2, 0x06 /* Public */,
       3,    0,   40,    2, 0x06 /* Public */,
       4,    0,   41,    2, 0x06 /* Public */,
       5,    0,   42,    2, 0x06 /* Public */,
       6,    1,   43,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    7,

       0        // eod
};

void QSsh::Internal::SshRemoteProcessPrivate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SshRemoteProcessPrivate *_t = static_cast<SshRemoteProcessPrivate *>(_o);
        switch (_id) {
        case 0: _t->started(); break;
        case 1: _t->readyRead(); break;
        case 2: _t->readyReadStandardOutput(); break;
        case 3: _t->readyReadStandardError(); break;
        case 4: _t->closed((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (SshRemoteProcessPrivate::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshRemoteProcessPrivate::started)) {
                *result = 0;
            }
        }
        {
            typedef void (SshRemoteProcessPrivate::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshRemoteProcessPrivate::readyRead)) {
                *result = 1;
            }
        }
        {
            typedef void (SshRemoteProcessPrivate::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshRemoteProcessPrivate::readyReadStandardOutput)) {
                *result = 2;
            }
        }
        {
            typedef void (SshRemoteProcessPrivate::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshRemoteProcessPrivate::readyReadStandardError)) {
                *result = 3;
            }
        }
        {
            typedef void (SshRemoteProcessPrivate::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshRemoteProcessPrivate::closed)) {
                *result = 4;
            }
        }
    }
}

const QMetaObject QSsh::Internal::SshRemoteProcessPrivate::staticMetaObject = {
    { &AbstractSshChannel::staticMetaObject, qt_meta_stringdata_QSsh__Internal__SshRemoteProcessPrivate.data,
      qt_meta_data_QSsh__Internal__SshRemoteProcessPrivate,  qt_static_metacall, 0, 0}
};


const QMetaObject *QSsh::Internal::SshRemoteProcessPrivate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QSsh::Internal::SshRemoteProcessPrivate::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QSsh__Internal__SshRemoteProcessPrivate.stringdata))
        return static_cast<void*>(const_cast< SshRemoteProcessPrivate*>(this));
    return AbstractSshChannel::qt_metacast(_clname);
}

int QSsh::Internal::SshRemoteProcessPrivate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = AbstractSshChannel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void QSsh::Internal::SshRemoteProcessPrivate::started()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void QSsh::Internal::SshRemoteProcessPrivate::readyRead()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void QSsh::Internal::SshRemoteProcessPrivate::readyReadStandardOutput()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void QSsh::Internal::SshRemoteProcessPrivate::readyReadStandardError()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}

// SIGNAL 4
void QSsh::Internal::SshRemoteProcessPrivate::closed(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_END_MOC_NAMESPACE
