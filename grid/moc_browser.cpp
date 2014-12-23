/****************************************************************************
** Meta object code from reading C++ file 'browser.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.3.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "browser.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'browser.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.3.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_BrowserWindow_t {
    QByteArrayData data[22];
    char stringdata[314];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_BrowserWindow_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_BrowserWindow_t qt_meta_stringdata_BrowserWindow = {
    {
QT_MOC_LITERAL(0, 0, 13),
QT_MOC_LITERAL(1, 14, 7),
QT_MOC_LITERAL(2, 22, 0),
QT_MOC_LITERAL(3, 23, 12),
QT_MOC_LITERAL(4, 36, 13),
QT_MOC_LITERAL(5, 50, 11),
QT_MOC_LITERAL(6, 62, 8),
QT_MOC_LITERAL(7, 71, 12),
QT_MOC_LITERAL(8, 84, 21),
QT_MOC_LITERAL(9, 106, 14),
QT_MOC_LITERAL(10, 121, 5),
QT_MOC_LITERAL(11, 127, 15),
QT_MOC_LITERAL(12, 143, 18),
QT_MOC_LITERAL(13, 162, 18),
QT_MOC_LITERAL(14, 181, 7),
QT_MOC_LITERAL(15, 189, 18),
QT_MOC_LITERAL(16, 208, 18),
QT_MOC_LITERAL(17, 227, 18),
QT_MOC_LITERAL(18, 246, 19),
QT_MOC_LITERAL(19, 266, 10),
QT_MOC_LITERAL(20, 277, 28),
QT_MOC_LITERAL(21, 306, 7)
    },
    "BrowserWindow\0doAbout\0\0showSettings\0"
    "connectToHost\0parseStatus\0fileName\0"
    "parseHistory\0handleConnectionError\0"
    "QSsh::SshError\0error\0handleConnected\0"
    "handleDisconnected\0handleShellMessage\0"
    "message\0handleShellStarted\0"
    "handleRemoteStdout\0handleRemoteStderr\0"
    "handleChannelClosed\0exitStatus\0"
    "handleSFTPChannelInitialized\0refresh"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_BrowserWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   89,    2, 0x08 /* Private */,
       3,    0,   90,    2, 0x08 /* Private */,
       4,    0,   91,    2, 0x08 /* Private */,
       5,    1,   92,    2, 0x08 /* Private */,
       7,    1,   95,    2, 0x08 /* Private */,
       8,    1,   98,    2, 0x08 /* Private */,
      11,    0,  101,    2, 0x08 /* Private */,
      12,    0,  102,    2, 0x08 /* Private */,
      13,    1,  103,    2, 0x08 /* Private */,
      15,    0,  106,    2, 0x08 /* Private */,
      16,    0,  107,    2, 0x08 /* Private */,
      17,    0,  108,    2, 0x08 /* Private */,
      18,    1,  109,    2, 0x08 /* Private */,
      20,    0,  112,    2, 0x08 /* Private */,
      21,    0,  113,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   14,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   19,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void BrowserWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        BrowserWindow *_t = static_cast<BrowserWindow *>(_o);
        switch (_id) {
        case 0: _t->doAbout(); break;
        case 1: _t->showSettings(); break;
        case 2: _t->connectToHost(); break;
        case 3: _t->parseStatus((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->parseHistory((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->handleConnectionError((*reinterpret_cast< QSsh::SshError(*)>(_a[1]))); break;
        case 6: _t->handleConnected(); break;
        case 7: _t->handleDisconnected(); break;
        case 8: _t->handleShellMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->handleShellStarted(); break;
        case 10: _t->handleRemoteStdout(); break;
        case 11: _t->handleRemoteStderr(); break;
        case 12: _t->handleChannelClosed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->handleSFTPChannelInitialized(); break;
        case 14: _t->refresh(); break;
        default: ;
        }
    }
}

const QMetaObject BrowserWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_BrowserWindow.data,
      qt_meta_data_BrowserWindow,  qt_static_metacall, 0, 0}
};


const QMetaObject *BrowserWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BrowserWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BrowserWindow.stringdata))
        return static_cast<void*>(const_cast< BrowserWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int BrowserWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 15;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
