/****************************************************************************
** Meta object code from reading C++ file 'DisplayWindow.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CLAMP_UI/Display/DisplayWindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DisplayWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DisplayWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x08,
      45,   39,   14,   14, 0x08,
      70,   14,   14,   14, 0x08,
      85,   77,   14,   14, 0x08,
     121,  107,   14,   14, 0x08,
     159,   14,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_DisplayWindow[] = {
    "DisplayWindow\0\0viewCalibrationReport()\0"
    "index\0changeLowPassFilter(int)\0save()\0"
    "running\0setThreadStatus(bool)\0"
    "title,message\0errorMessage(const char*,const char*)\0"
    "adjustTAxis()\0"
};

void DisplayWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DisplayWindow *_t = static_cast<DisplayWindow *>(_o);
        switch (_id) {
        case 0: _t->viewCalibrationReport(); break;
        case 1: _t->changeLowPassFilter((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->save(); break;
        case 3: _t->setThreadStatus((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->errorMessage((*reinterpret_cast< const char*(*)>(_a[1])),(*reinterpret_cast< const char*(*)>(_a[2]))); break;
        case 5: _t->adjustTAxis(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DisplayWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DisplayWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_DisplayWindow,
      qt_meta_data_DisplayWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DisplayWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DisplayWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DisplayWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DisplayWindow))
        return static_cast<void*>(const_cast< DisplayWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int DisplayWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
