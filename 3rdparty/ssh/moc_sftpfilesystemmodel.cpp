/****************************************************************************
** Meta object code from reading C++ file 'sftpfilesystemmodel.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.3.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sftpfilesystemmodel.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sftpfilesystemmodel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.3.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_QSsh__SftpFileSystemModel_t {
    QByteArrayData data[18];
    char stringdata[319];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QSsh__SftpFileSystemModel_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QSsh__SftpFileSystemModel_t qt_meta_stringdata_QSsh__SftpFileSystemModel = {
    {
QT_MOC_LITERAL(0, 0, 25),
QT_MOC_LITERAL(1, 26, 19),
QT_MOC_LITERAL(2, 46, 0),
QT_MOC_LITERAL(3, 47, 12),
QT_MOC_LITERAL(4, 60, 15),
QT_MOC_LITERAL(5, 76, 21),
QT_MOC_LITERAL(6, 98, 15),
QT_MOC_LITERAL(7, 114, 5),
QT_MOC_LITERAL(8, 120, 30),
QT_MOC_LITERAL(9, 151, 26),
QT_MOC_LITERAL(10, 178, 28),
QT_MOC_LITERAL(11, 207, 22),
QT_MOC_LITERAL(12, 230, 6),
QT_MOC_LITERAL(13, 237, 14),
QT_MOC_LITERAL(14, 252, 5),
QT_MOC_LITERAL(15, 258, 25),
QT_MOC_LITERAL(16, 284, 12),
QT_MOC_LITERAL(17, 297, 21)
    },
    "QSsh::SftpFileSystemModel\0sftpOperationFailed\0"
    "\0errorMessage\0connectionError\0"
    "sftpOperationFinished\0QSsh::SftpJobId\0"
    "error\0handleSshConnectionEstablished\0"
    "handleSshConnectionFailure\0"
    "handleSftpChannelInitialized\0"
    "handleSftpChannelError\0reason\0"
    "handleFileInfo\0jobId\0QList<QSsh::SftpFileInfo>\0"
    "fileInfoList\0handleSftpJobFinished"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QSsh__SftpFileSystemModel[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x06 /* Public */,
       4,    1,   62,    2, 0x06 /* Public */,
       5,    2,   65,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    0,   70,    2, 0x08 /* Private */,
       9,    0,   71,    2, 0x08 /* Private */,
      10,    0,   72,    2, 0x08 /* Private */,
      11,    1,   73,    2, 0x08 /* Private */,
      13,    2,   76,    2, 0x08 /* Private */,
      17,    2,   81,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, 0x80000000 | 6, QMetaType::QString,    2,    7,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 15,   14,   16,
    QMetaType::Void, 0x80000000 | 6, QMetaType::QString,   14,    3,

       0        // eod
};

void QSsh::SftpFileSystemModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SftpFileSystemModel *_t = static_cast<SftpFileSystemModel *>(_o);
        switch (_id) {
        case 0: _t->sftpOperationFailed((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->connectionError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->sftpOperationFinished((*reinterpret_cast< QSsh::SftpJobId(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 3: _t->handleSshConnectionEstablished(); break;
        case 4: _t->handleSshConnectionFailure(); break;
        case 5: _t->handleSftpChannelInitialized(); break;
        case 6: _t->handleSftpChannelError((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: _t->handleFileInfo((*reinterpret_cast< QSsh::SftpJobId(*)>(_a[1])),(*reinterpret_cast< const QList<QSsh::SftpFileInfo>(*)>(_a[2]))); break;
        case 8: _t->handleSftpJobFinished((*reinterpret_cast< QSsh::SftpJobId(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (SftpFileSystemModel::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SftpFileSystemModel::sftpOperationFailed)) {
                *result = 0;
            }
        }
        {
            typedef void (SftpFileSystemModel::*_t)(const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SftpFileSystemModel::connectionError)) {
                *result = 1;
            }
        }
        {
            typedef void (SftpFileSystemModel::*_t)(QSsh::SftpJobId , const QString & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&SftpFileSystemModel::sftpOperationFinished)) {
                *result = 2;
            }
        }
    }
}

const QMetaObject QSsh::SftpFileSystemModel::staticMetaObject = {
    { &QAbstractItemModel::staticMetaObject, qt_meta_stringdata_QSsh__SftpFileSystemModel.data,
      qt_meta_data_QSsh__SftpFileSystemModel,  qt_static_metacall, 0, 0}
};


const QMetaObject *QSsh::SftpFileSystemModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QSsh::SftpFileSystemModel::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QSsh__SftpFileSystemModel.stringdata))
        return static_cast<void*>(const_cast< SftpFileSystemModel*>(this));
    return QAbstractItemModel::qt_metacast(_clname);
}

int QSsh::SftpFileSystemModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractItemModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void QSsh::SftpFileSystemModel::sftpOperationFailed(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QSsh::SftpFileSystemModel::connectionError(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QSsh::SftpFileSystemModel::sftpOperationFinished(QSsh::SftpJobId _t1, const QString & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
