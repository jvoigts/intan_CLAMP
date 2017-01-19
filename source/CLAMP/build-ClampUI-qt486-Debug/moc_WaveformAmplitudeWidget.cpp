/****************************************************************************
** Meta object code from reading C++ file 'WaveformAmplitudeWidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CLAMP_UI/Control/WaveformAmplitudeWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'WaveformAmplitudeWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_WaveformAmplitudeParams[] = {

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
      25,   24,   24,   24, 0x05,

 // slots: signature, parameters, type, tag, flags
      41,   24,   24,   24, 0x08,
      58,   24,   24,   24, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_WaveformAmplitudeParams[] = {
    "WaveformAmplitudeParams\0\0valuesChanged()\0"
    "notifyObserver()\0adjustStepSize()\0"
};

void WaveformAmplitudeParams::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        WaveformAmplitudeParams *_t = static_cast<WaveformAmplitudeParams *>(_o);
        switch (_id) {
        case 0: _t->valuesChanged(); break;
        case 1: _t->notifyObserver(); break;
        case 2: _t->adjustStepSize(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData WaveformAmplitudeParams::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject WaveformAmplitudeParams::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_WaveformAmplitudeParams,
      qt_meta_data_WaveformAmplitudeParams, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &WaveformAmplitudeParams::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *WaveformAmplitudeParams::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *WaveformAmplitudeParams::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_WaveformAmplitudeParams))
        return static_cast<void*>(const_cast< WaveformAmplitudeParams*>(this));
    return QObject::qt_metacast(_clname);
}

int WaveformAmplitudeParams::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void WaveformAmplitudeParams::valuesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_WaveformAmplitudeWidget[] = {

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
      33,   25,   24,   24, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_WaveformAmplitudeWidget[] = {
    "WaveformAmplitudeWidget\0\0checked\0"
    "setMultiStep(bool)\0"
};

void WaveformAmplitudeWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        WaveformAmplitudeWidget *_t = static_cast<WaveformAmplitudeWidget *>(_o);
        switch (_id) {
        case 0: _t->setMultiStep((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData WaveformAmplitudeWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject WaveformAmplitudeWidget::staticMetaObject = {
    { &QGroupBox::staticMetaObject, qt_meta_stringdata_WaveformAmplitudeWidget,
      qt_meta_data_WaveformAmplitudeWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &WaveformAmplitudeWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *WaveformAmplitudeWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *WaveformAmplitudeWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_WaveformAmplitudeWidget))
        return static_cast<void*>(const_cast< WaveformAmplitudeWidget*>(this));
    return QGroupBox::qt_metacast(_clname);
}

int WaveformAmplitudeWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
QT_END_MOC_NAMESPACE
