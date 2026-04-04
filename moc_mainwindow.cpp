/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "src/mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[18];
    char stringdata0[503];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 23), // "on_actionOpen_triggered"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 25), // "on_actionSaveAs_triggered"
QT_MOC_LITERAL(4, 62, 23), // "on_actionExit_triggered"
QT_MOC_LITERAL(5, 86, 23), // "on_actionUndo_triggered"
QT_MOC_LITERAL(6, 110, 32), // "on_actionCatmull_Clark_triggered"
QT_MOC_LITERAL(7, 143, 28), // "on_actionDoo_Sabin_triggered"
QT_MOC_LITERAL(8, 172, 23), // "on_actionLoop_triggered"
QT_MOC_LITERAL(9, 196, 31), // "on_actionToggle_Faces_triggered"
QT_MOC_LITERAL(10, 228, 35), // "on_actionToggle_Wireframe_tri..."
QT_MOC_LITERAL(11, 264, 34), // "on_actionToggle_Vertices_trig..."
QT_MOC_LITERAL(12, 299, 34), // "on_actionToggle_Lighting_trig..."
QT_MOC_LITERAL(13, 334, 40), // "on_actionToggle_Smooth_Shadin..."
QT_MOC_LITERAL(14, 375, 29), // "on_actionMesh_Color_triggered"
QT_MOC_LITERAL(15, 405, 35), // "on_actionBackground_Color_tri..."
QT_MOC_LITERAL(16, 441, 24), // "on_actionAbout_triggered"
QT_MOC_LITERAL(17, 466, 36) // "on_actionChange_Wire_Color_tr..."

    },
    "MainWindow\0on_actionOpen_triggered\0\0"
    "on_actionSaveAs_triggered\0"
    "on_actionExit_triggered\0on_actionUndo_triggered\0"
    "on_actionCatmull_Clark_triggered\0"
    "on_actionDoo_Sabin_triggered\0"
    "on_actionLoop_triggered\0"
    "on_actionToggle_Faces_triggered\0"
    "on_actionToggle_Wireframe_triggered\0"
    "on_actionToggle_Vertices_triggered\0"
    "on_actionToggle_Lighting_triggered\0"
    "on_actionToggle_Smooth_Shading_triggered\0"
    "on_actionMesh_Color_triggered\0"
    "on_actionBackground_Color_triggered\0"
    "on_actionAbout_triggered\0"
    "on_actionChange_Wire_Color_triggered"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   94,    2, 0x08 /* Private */,
       3,    0,   95,    2, 0x08 /* Private */,
       4,    0,   96,    2, 0x08 /* Private */,
       5,    0,   97,    2, 0x08 /* Private */,
       6,    0,   98,    2, 0x08 /* Private */,
       7,    0,   99,    2, 0x08 /* Private */,
       8,    0,  100,    2, 0x08 /* Private */,
       9,    0,  101,    2, 0x08 /* Private */,
      10,    0,  102,    2, 0x08 /* Private */,
      11,    0,  103,    2, 0x08 /* Private */,
      12,    0,  104,    2, 0x08 /* Private */,
      13,    0,  105,    2, 0x08 /* Private */,
      14,    0,  106,    2, 0x08 /* Private */,
      15,    0,  107,    2, 0x08 /* Private */,
      16,    0,  108,    2, 0x08 /* Private */,
      17,    0,  109,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->on_actionOpen_triggered(); break;
        case 1: _t->on_actionSaveAs_triggered(); break;
        case 2: _t->on_actionExit_triggered(); break;
        case 3: _t->on_actionUndo_triggered(); break;
        case 4: _t->on_actionCatmull_Clark_triggered(); break;
        case 5: _t->on_actionDoo_Sabin_triggered(); break;
        case 6: _t->on_actionLoop_triggered(); break;
        case 7: _t->on_actionToggle_Faces_triggered(); break;
        case 8: _t->on_actionToggle_Wireframe_triggered(); break;
        case 9: _t->on_actionToggle_Vertices_triggered(); break;
        case 10: _t->on_actionToggle_Lighting_triggered(); break;
        case 11: _t->on_actionToggle_Smooth_Shading_triggered(); break;
        case 12: _t->on_actionMesh_Color_triggered(); break;
        case 13: _t->on_actionBackground_Color_triggered(); break;
        case 14: _t->on_actionAbout_triggered(); break;
        case 15: _t->on_actionChange_Wire_Color_triggered(); break;
        default: ;
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 16;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
