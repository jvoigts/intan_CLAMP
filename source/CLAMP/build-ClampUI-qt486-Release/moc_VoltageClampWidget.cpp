/****************************************************************************
** Meta object code from reading C++ file 'VoltageClampWidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CLAMP_UI/Control/VoltageClampWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VoltageClampWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AppliedVoltageWaveformWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      30,   29,   29,   29, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_AppliedVoltageWaveformWidget[] = {
    "AppliedVoltageWaveformWidget\0\0"
    "enableArbWaveformButton()\0"
};

void AppliedVoltageWaveformWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        AppliedVoltageWaveformWidget *_t = static_cast<AppliedVoltageWaveformWidget *>(_o);
        switch (_id) {
        case 0: _t->enableArbWaveformButton(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData AppliedVoltageWaveformWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject AppliedVoltageWaveformWidget::staticMetaObject = {
    { &QGroupBox::staticMetaObject, qt_meta_stringdata_AppliedVoltageWaveformWidget,
      qt_meta_data_AppliedVoltageWaveformWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AppliedVoltageWaveformWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AppliedVoltageWaveformWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AppliedVoltageWaveformWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AppliedVoltageWaveformWidget))
        return static_cast<void*>(const_cast< AppliedVoltageWaveformWidget*>(this));
    return QGroupBox::qt_metacast(_clname);
}

int AppliedVoltageWaveformWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGroupBox::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_VoltageClampWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      20,   19,   19,   19, 0x05,

 // slots: signature, parameters, type, tag, flags
      45,   36,   19,   19, 0x0a,
      80,   19,   19,   19, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_VoltageClampWidget[] = {
    "VoltageClampWidget\0\0changeDisplay()\0"
    "Ra,Rm,Cm\0setWholeCell(double,double,double)\0"
    "optionsChanged()\0"
};

void VoltageClampWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        VoltageClampWidget *_t = static_cast<VoltageClampWidget *>(_o);
        switch (_id) {
        case 0: _t->changeDisplay(); break;
        case 1: _t->setWholeCell((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 2: _t->optionsChanged(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData VoltageClampWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject VoltageClampWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_VoltageClampWidget,
      qt_meta_data_VoltageClampWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &VoltageClampWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *VoltageClampWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *VoltageClampWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_VoltageClampWidget))
        return static_cast<void*>(const_cast< VoltageClampWidget*>(this));
    if (!strcmp(_clname, "Controller"))
        return static_cast< Controller*>(const_cast< VoltageClampWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int VoltageClampWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void VoltageClampWidget::changeDisplay()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
