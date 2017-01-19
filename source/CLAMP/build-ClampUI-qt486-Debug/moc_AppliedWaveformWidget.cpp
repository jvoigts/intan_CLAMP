/****************************************************************************
** Meta object code from reading C++ file 'AppliedWaveformWidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CLAMP_UI/Control/AppliedWaveformWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AppliedWaveformWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_NumCyclesWidget[] = {

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

static const char qt_meta_stringdata_NumCyclesWidget[] = {
    "NumCyclesWidget\0"
};

void NumCyclesWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData NumCyclesWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject NumCyclesWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_NumCyclesWidget,
      qt_meta_data_NumCyclesWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &NumCyclesWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *NumCyclesWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *NumCyclesWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_NumCyclesWidget))
        return static_cast<void*>(const_cast< NumCyclesWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int NumCyclesWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_MultistepParams[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      17,   16,   16,   16, 0x05,

 // slots: signature, parameters, type, tag, flags
      33,   16,   16,   16, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MultistepParams[] = {
    "MultistepParams\0\0valuesChanged()\0"
    "notifyObserver()\0"
};

void MultistepParams::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MultistepParams *_t = static_cast<MultistepParams *>(_o);
        switch (_id) {
        case 0: _t->valuesChanged(); break;
        case 1: _t->notifyObserver(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData MultistepParams::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MultistepParams::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_MultistepParams,
      qt_meta_data_MultistepParams, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MultistepParams::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MultistepParams::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MultistepParams::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MultistepParams))
        return static_cast<void*>(const_cast< MultistepParams*>(this));
    return QObject::qt_metacast(_clname);
}

int MultistepParams::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void MultistepParams::valuesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_MultistepParamsDisplay[] = {

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
      24,   23,   23,   23, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MultistepParamsDisplay[] = {
    "MultistepParamsDisplay\0\0redoLayout()\0"
};

void MultistepParamsDisplay::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MultistepParamsDisplay *_t = static_cast<MultistepParamsDisplay *>(_o);
        switch (_id) {
        case 0: _t->redoLayout(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData MultistepParamsDisplay::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MultistepParamsDisplay::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_MultistepParamsDisplay,
      qt_meta_data_MultistepParamsDisplay, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MultistepParamsDisplay::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MultistepParamsDisplay::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MultistepParamsDisplay::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MultistepParamsDisplay))
        return static_cast<void*>(const_cast< MultistepParamsDisplay*>(this));
    return QWidget::qt_metacast(_clname);
}

int MultistepParamsDisplay::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_ArbWaveformParamsDisplay[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      26,   25,   25,   25, 0x05,

 // slots: signature, parameters, type, tag, flags
      46,   25,   25,   25, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ArbWaveformParamsDisplay[] = {
    "ArbWaveformParamsDisplay\0\0arbWaveformLoaded()\0"
    "loadArbWaveform()\0"
};

void ArbWaveformParamsDisplay::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ArbWaveformParamsDisplay *_t = static_cast<ArbWaveformParamsDisplay *>(_o);
        switch (_id) {
        case 0: _t->arbWaveformLoaded(); break;
        case 1: _t->loadArbWaveform(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData ArbWaveformParamsDisplay::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ArbWaveformParamsDisplay::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ArbWaveformParamsDisplay,
      qt_meta_data_ArbWaveformParamsDisplay, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ArbWaveformParamsDisplay::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ArbWaveformParamsDisplay::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ArbWaveformParamsDisplay::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ArbWaveformParamsDisplay))
        return static_cast<void*>(const_cast< ArbWaveformParamsDisplay*>(this));
    return QWidget::qt_metacast(_clname);
}

int ArbWaveformParamsDisplay::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void ArbWaveformParamsDisplay::arbWaveformLoaded()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_MultistepDialog[] = {

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

static const char qt_meta_stringdata_MultistepDialog[] = {
    "MultistepDialog\0"
};

void MultistepDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData MultistepDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MultistepDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_MultistepDialog,
      qt_meta_data_MultistepDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MultistepDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MultistepDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MultistepDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MultistepDialog))
        return static_cast<void*>(const_cast< MultistepDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int MultistepDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_PulseTrainParams[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x05,

 // slots: signature, parameters, type, tag, flags
      34,   17,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_PulseTrainParams[] = {
    "PulseTrainParams\0\0valuesChanged()\0"
    "notifyObserver()\0"
};

void PulseTrainParams::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PulseTrainParams *_t = static_cast<PulseTrainParams *>(_o);
        switch (_id) {
        case 0: _t->valuesChanged(); break;
        case 1: _t->notifyObserver(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData PulseTrainParams::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PulseTrainParams::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_PulseTrainParams,
      qt_meta_data_PulseTrainParams, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PulseTrainParams::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PulseTrainParams::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PulseTrainParams::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PulseTrainParams))
        return static_cast<void*>(const_cast< PulseTrainParams*>(this));
    return QObject::qt_metacast(_clname);
}

int PulseTrainParams::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void PulseTrainParams::valuesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_PulseTrainDialog[] = {

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

static const char qt_meta_stringdata_PulseTrainDialog[] = {
    "PulseTrainDialog\0"
};

void PulseTrainDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData PulseTrainDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PulseTrainDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_PulseTrainDialog,
      qt_meta_data_PulseTrainDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PulseTrainDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PulseTrainDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PulseTrainDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PulseTrainDialog))
        return static_cast<void*>(const_cast< PulseTrainDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int PulseTrainDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_PulseTrainParamsDisplay[] = {

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
      25,   24,   24,   24, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_PulseTrainParamsDisplay[] = {
    "PulseTrainParamsDisplay\0\0redoLayout()\0"
};

void PulseTrainParamsDisplay::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PulseTrainParamsDisplay *_t = static_cast<PulseTrainParamsDisplay *>(_o);
        switch (_id) {
        case 0: _t->redoLayout(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData PulseTrainParamsDisplay::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PulseTrainParamsDisplay::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_PulseTrainParamsDisplay,
      qt_meta_data_PulseTrainParamsDisplay, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PulseTrainParamsDisplay::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PulseTrainParamsDisplay::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PulseTrainParamsDisplay::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PulseTrainParamsDisplay))
        return static_cast<void*>(const_cast< PulseTrainParamsDisplay*>(this));
    return QWidget::qt_metacast(_clname);
}

int PulseTrainParamsDisplay::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
