/****************************************************************************
** Meta object code from reading C++ file 'sshdirecttcpiptunnel.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.3.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sshdirecttcpiptunnel.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sshdirecttcpiptunnel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.3.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QSsh__SshDirectTcpIpTunnel_t {
    QByteArrayData data[7];
    char stringdata[78];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QSsh__SshDirectTcpIpTunnel_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QSsh__SshDirectTcpIpTunnel_t qt_meta_stringdata_QSsh__SshDirectTcpIpTunnel = {
    {
QT_MOC_LITERAL(0, 0, 26),
QT_MOC_LITERAL(1, 27, 11),
QT_MOC_LITERAL(2, 39, 0),
QT_MOC_LITERAL(3, 40, 5),
QT_MOC_LITERAL(4, 46, 6),
QT_MOC_LITERAL(5, 53, 12),
QT_MOC_LITERAL(6, 66, 11)
    },
    "QSsh::SshDirectTcpIpTunnel\0initialized\0"
    "\0error\0reason\0tunnelClosed\0handleError"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QSsh__SshDirectTcpIpTunnel[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x06 /* Public */,
       3,    1,   35,    2, 0x06 /* Public */,
       5,    0,   38,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   39,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    4,

       0        // eod
};

void QSsh::SshDirectTcpIpTunnel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SshDirectTcpIpTunnel *_t = static_cast<SshDirectTcpIpTunnel *>(_o);
        switch (_id) {
        case 0: _t->initialized(); break;
        case 1: _t->error((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->tunnelClosed(); break;
        case 3: _t->handleError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (SshDirectTcpIpTunnel::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshDirectTcpIpTunnel::initialized)) {
                *result = 0;
            }
        }
        {
            typedef void (SshDirectTcpIpTunnel::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshDirectTcpIpTunnel::error)) {
                *result = 1;
            }
        }
        {
            typedef void (SshDirectTcpIpTunnel::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SshDirectTcpIpTunnel::tunnelClosed)) {
                *result = 2;
            }
        }
    }
}

const QMetaObject QSsh::SshDirectTcpIpTunnel::staticMetaObject = {
    { &QIODevice::staticMetaObject, qt_meta_stringdata_QSsh__SshDirectTcpIpTunnel.data,
      qt_meta_data_QSsh__SshDirectTcpIpTunnel,  qt_static_metacall, 0, 0}
};


const QMetaObject *QSsh::SshDirectTcpIpTunnel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QSsh::SshDirectTcpIpTunnel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QSsh__SshDirectTcpIpTunnel.stringdata))
        return static_cast<void*>(const_cast< SshDirectTcpIpTunnel*>(this));
    return QIODevice::qt_metacast(_clname);
}

int QSsh::SshDirectTcpIpTunnel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void QSsh::SshDirectTcpIpTunnel::initialized()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void QSsh::SshDirectTcpIpTunnel::error(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QSsh::SshDirectTcpIpTunnel::tunnelClosed()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}
QT_END_MOC_NAMESPACE
