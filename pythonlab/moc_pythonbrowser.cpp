/****************************************************************************
** Meta object code from reading C++ file 'pythonbrowser.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "pythonbrowser.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pythonbrowser.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_PythonBrowser_t {
    QByteArrayData data[12];
    char stringdata0[120];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PythonBrowser_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PythonBrowser_t qt_meta_stringdata_PythonBrowser = {
    {
QT_MOC_LITERAL(0, 0, 13), // "PythonBrowser"
QT_MOC_LITERAL(1, 14, 8), // "executed"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 14), // "executeCommand"
QT_MOC_LITERAL(4, 39, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(5, 56, 4), // "item"
QT_MOC_LITERAL(6, 61, 4), // "role"
QT_MOC_LITERAL(7, 66, 13), // "doContextMenu"
QT_MOC_LITERAL(8, 80, 5), // "point"
QT_MOC_LITERAL(9, 86, 8), // "copyName"
QT_MOC_LITERAL(10, 95, 9), // "copyValue"
QT_MOC_LITERAL(11, 105, 14) // "deleteVariable"

    },
    "PythonBrowser\0executed\0\0executeCommand\0"
    "QTreeWidgetItem*\0item\0role\0doContextMenu\0"
    "point\0copyName\0copyValue\0deleteVariable"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PythonBrowser[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x08 /* Private */,
       3,    2,   45,    2, 0x08 /* Private */,
       7,    1,   50,    2, 0x08 /* Private */,
       9,    0,   53,    2, 0x08 /* Private */,
      10,    0,   54,    2, 0x08 /* Private */,
      11,    0,   55,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4, QMetaType::Int,    5,    6,
    QMetaType::Void, QMetaType::QPoint,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void PythonBrowser::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        PythonBrowser *_t = static_cast<PythonBrowser *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->executed(); break;
        case 1: _t->executeCommand((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 2: _t->doContextMenu((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 3: _t->copyName(); break;
        case 4: _t->copyValue(); break;
        case 5: _t->deleteVariable(); break;
        default: ;
        }
    }
}

const QMetaObject PythonBrowser::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_PythonBrowser.data,
      qt_meta_data_PythonBrowser,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *PythonBrowser::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PythonBrowser::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_PythonBrowser.stringdata0))
        return static_cast<void*>(const_cast< PythonBrowser*>(this));
    return QWidget::qt_metacast(_clname);
}

int PythonBrowser::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
