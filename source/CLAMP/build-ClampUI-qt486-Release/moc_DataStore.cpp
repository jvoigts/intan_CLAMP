/****************************************************************************
** Meta object code from reading C++ file 'DataStore.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CLAMP_UI/DataStore.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DataStore.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DataProcessor[] = {

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

static const char qt_meta_stringdata_DataProcessor[] = {
    "DataProcessor\0"
};

void DataProcessor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData DataProcessor::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DataProcessor::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_DataProcessor,
      qt_meta_data_DataProcessor, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DataProcessor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DataProcessor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DataProcessor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DataProcessor))
        return static_cast<void*>(const_cast< DataProcessor*>(this));
    return QObject::qt_metacast(_clname);
}

int DataProcessor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_FilterProcessor[] = {

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
      24,   17,   16,   16, 0x0a,
      53,   50,   16,   16, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_FilterProcessor[] = {
    "FilterProcessor\0\0enable\0"
    "enableLowPassFilter(bool)\0fc\0"
    "setLowPassFilterCutoff(double)\0"
};

void FilterProcessor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        FilterProcessor *_t = static_cast<FilterProcessor *>(_o);
        switch (_id) {
        case 0: _t->enableLowPassFilter((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->setLowPassFilterCutoff((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData FilterProcessor::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FilterProcessor::staticMetaObject = {
    { &DataProcessor::staticMetaObject, qt_meta_stringdata_FilterProcessor,
      qt_meta_data_FilterProcessor, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FilterProcessor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FilterProcessor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FilterProcessor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FilterProcessor))
        return static_cast<void*>(const_cast< FilterProcessor*>(this));
    return DataProcessor::qt_metacast(_clname);
}

int FilterProcessor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DataProcessor::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
static const uint qt_meta_data_DataStore[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      18,   11,   10,   10, 0x05,
      55,   52,   10,   10, 0x05,
      99,   90,   10,   10, 0x05,
     148,   10,   10,   10, 0x05,
     167,   10,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
     188,  182,   10,   10, 0x0a,
     205,   90,   10,   10, 0x0a,
     243,  240,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_DataStore[] = {
    "DataStore\0\0enable\0lowPassFilterEnabledChanged(bool)\0"
    "fc\0lowPassFilterCutoffChanged(double)\0"
    "Ra,Rm,Cm\0wholeCellParametersChanged(double,double,double)\0"
    "timescaleChanged()\0waveformDone()\0"
    "value\0setOverlay(bool)\0"
    "setWholeCell(double,double,double)\0"
    "Ra\0adjustRa(double)\0"
};

void DataStore::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DataStore *_t = static_cast<DataStore *>(_o);
        switch (_id) {
        case 0: _t->lowPassFilterEnabledChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->lowPassFilterCutoffChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 2: _t->wholeCellParametersChanged((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 3: _t->timescaleChanged(); break;
        case 4: _t->waveformDone(); break;
        case 5: _t->setOverlay((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->setWholeCell((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 7: _t->adjustRa((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DataStore::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DataStore::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_DataStore,
      qt_meta_data_DataStore, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DataStore::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DataStore::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DataStore::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DataStore))
        return static_cast<void*>(const_cast< DataStore*>(this));
    return QObject::qt_metacast(_clname);
}

int DataStore::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void DataStore::lowPassFilterEnabledChanged(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DataStore::lowPassFilterCutoffChanged(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DataStore::wholeCellParametersChanged(double _t1, double _t2, double _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DataStore::timescaleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}

// SIGNAL 4
void DataStore::waveformDone()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}
QT_END_MOC_NAMESPACE
