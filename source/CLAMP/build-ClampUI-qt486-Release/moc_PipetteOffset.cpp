/****************************************************************************
** Meta object code from reading C++ file 'PipetteOffset.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CLAMP_UI/Control/PipetteOffset.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PipetteOffset.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_PipetteOffsetWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x08,
      43,   37,   20,   20, 0x08,
      69,   37,   20,   20, 0x08,
     110,   98,   20,   20, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_PipetteOffsetWidget[] = {
    "PipetteOffsetWidget\0\0autoCalibrate()\0"
    "value\0setAutoButtonEnable(bool)\0"
    "setPipetteOffsetHere(double)\0unit_,value\0"
    "setValueHere(int,double)\0"
};

void PipetteOffsetWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PipetteOffsetWidget *_t = static_cast<PipetteOffsetWidget *>(_o);
        switch (_id) {
        case 0: _t->autoCalibrate(); break;
        case 1: _t->setAutoButtonEnable((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->setPipetteOffsetHere((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 3: _t->setValueHere((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData PipetteOffsetWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PipetteOffsetWidget::staticMetaObject = {
    { &QGroupBox::staticMetaObject, qt_meta_stringdata_PipetteOffsetWidget,
      qt_meta_data_PipetteOffsetWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PipetteOffsetWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PipetteOffsetWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PipetteOffsetWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PipetteOffsetWidget))
        return static_cast<void*>(const_cast< PipetteOffsetWidget*>(this));
    return QGroupBox::qt_metacast(_clname);
}

int PipetteOffsetWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGroupBox::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
