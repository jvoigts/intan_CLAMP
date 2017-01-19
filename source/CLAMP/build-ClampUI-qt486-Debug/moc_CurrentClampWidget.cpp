/****************************************************************************
** Meta object code from reading C++ file 'CurrentClampWidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CLAMP_UI/Control/CurrentClampWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CurrentClampWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_AppliedCurrentWaveformWidget[] = {

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

static const char qt_meta_stringdata_AppliedCurrentWaveformWidget[] = {
    "AppliedCurrentWaveformWidget\0\0"
    "enableArbWaveformButton()\0"
};

void AppliedCurrentWaveformWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        AppliedCurrentWaveformWidget *_t = static_cast<AppliedCurrentWaveformWidget *>(_o);
        switch (_id) {
        case 0: _t->enableArbWaveformButton(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData AppliedCurrentWaveformWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject AppliedCurrentWaveformWidget::staticMetaObject = {
    { &QGroupBox::staticMetaObject, qt_meta_stringdata_AppliedCurrentWaveformWidget,
      qt_meta_data_AppliedCurrentWaveformWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AppliedCurrentWaveformWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AppliedCurrentWaveformWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AppliedCurrentWaveformWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AppliedCurrentWaveformWidget))
        return static_cast<void*>(const_cast< AppliedCurrentWaveformWidget*>(this));
    return QGroupBox::qt_metacast(_clname);
}

int AppliedCurrentWaveformWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
static const uint qt_meta_data_CurrentClampWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      20,   19,   19,   19, 0x05,

 // slots: signature, parameters, type, tag, flags
      42,   36,   19,   19, 0x08,
      66,   19,   19,   19, 0x08,
      83,   19,   19,   19, 0x08,
     109,  100,   19,   19, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_CurrentClampWidget[] = {
    "CurrentClampWidget\0\0changeDisplay()\0"
    "index\0setCurrentStepSize(int)\0"
    "setZeroCurrent()\0optionsChanged()\0"
    "Ra,Rm,Cm\0setWholeCell(double,double,double)\0"
};

void CurrentClampWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CurrentClampWidget *_t = static_cast<CurrentClampWidget *>(_o);
        switch (_id) {
        case 0: _t->changeDisplay(); break;
        case 1: _t->setCurrentStepSize((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->setZeroCurrent(); break;
        case 3: _t->optionsChanged(); break;
        case 4: _t->setWholeCell((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData CurrentClampWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CurrentClampWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_CurrentClampWidget,
      qt_meta_data_CurrentClampWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CurrentClampWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CurrentClampWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CurrentClampWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CurrentClampWidget))
        return static_cast<void*>(const_cast< CurrentClampWidget*>(this));
    if (!strcmp(_clname, "Controller"))
        return static_cast< Controller*>(const_cast< CurrentClampWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int CurrentClampWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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

// SIGNAL 0
void CurrentClampWidget::changeDisplay()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
