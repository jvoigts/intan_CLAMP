/****************************************************************************
** Meta object code from reading C++ file 'SignalOutputWidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CLAMP_UI/Control/SignalOutputWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SignalOutputWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SignalOutputWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      26,   20,   19,   19, 0x0a,
      62,   56,   19,   19, 0x08,
      92,   88,   19,   19, 0x08,
     118,   20,   19,   19, 0x08,
     139,   20,   19,   19, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SignalOutputWidget[] = {
    "SignalOutputWidget\0\0index\0"
    "updateFeedbackResistance(int)\0state\0"
    "enableOutputChanged(bool)\0dac\0"
    "setOutputDestination(int)\0"
    "setVoltageScale(int)\0setCurrentScale(int)\0"
};

void SignalOutputWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SignalOutputWidget *_t = static_cast<SignalOutputWidget *>(_o);
        switch (_id) {
        case 0: _t->updateFeedbackResistance((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->enableOutputChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->setOutputDestination((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->setVoltageScale((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->setCurrentScale((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData SignalOutputWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SignalOutputWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_SignalOutputWidget,
      qt_meta_data_SignalOutputWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SignalOutputWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SignalOutputWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SignalOutputWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SignalOutputWidget))
        return static_cast<void*>(const_cast< SignalOutputWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int SignalOutputWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
static const uint qt_meta_data_DacInUseDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_DacInUseDialog[] = {
    "DacInUseDialog\0"
};

void DacInUseDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData DacInUseDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DacInUseDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DacInUseDialog,
      qt_meta_data_DacInUseDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DacInUseDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DacInUseDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DacInUseDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DacInUseDialog))
        return static_cast<void*>(const_cast< DacInUseDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int DacInUseDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
