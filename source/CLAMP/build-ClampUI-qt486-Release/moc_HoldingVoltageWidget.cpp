/****************************************************************************
** Meta object code from reading C++ file 'HoldingVoltageWidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CLAMP_UI/Control/HoldingVoltageWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'HoldingVoltageWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_HoldingVoltageWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      32,   22,   21,   21, 0x08,
      66,   21,   21,   21, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_HoldingVoltageWidget[] = {
    "HoldingVoltageWidget\0\0valueInmV\0"
    "setHoldingVoltageInternal(double)\0"
    "setSliderValue(int)\0"
};

void HoldingVoltageWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        HoldingVoltageWidget *_t = static_cast<HoldingVoltageWidget *>(_o);
        switch (_id) {
        case 0: _t->setHoldingVoltageInternal((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 1: _t->setSliderValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData HoldingVoltageWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject HoldingVoltageWidget::staticMetaObject = {
    { &QGroupBox::staticMetaObject, qt_meta_stringdata_HoldingVoltageWidget,
      qt_meta_data_HoldingVoltageWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &HoldingVoltageWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *HoldingVoltageWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *HoldingVoltageWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HoldingVoltageWidget))
        return static_cast<void*>(const_cast< HoldingVoltageWidget*>(this));
    return QGroupBox::qt_metacast(_clname);
}

int HoldingVoltageWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGroupBox::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
