/****************************************************************************
** Meta object code from reading C++ file 'Plot.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CLAMP_UI/Display/Plot.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Plot.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Axis[] = {

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
       6,    5,    5,    5, 0x05,

 // slots: signature, parameters, type, tag, flags
      20,    5,    5,    5, 0x0a,
      29,    5,    5,    5, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Axis[] = {
    "Axis\0\0axisChanged()\0zoomIn()\0zoomOut()\0"
};

void Axis::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Axis *_t = static_cast<Axis *>(_o);
        switch (_id) {
        case 0: _t->axisChanged(); break;
        case 1: _t->zoomIn(); break;
        case 2: _t->zoomOut(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Axis::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Axis::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Axis,
      qt_meta_data_Axis, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Axis::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Axis::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Axis::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Axis))
        return static_cast<void*>(const_cast< Axis*>(this));
    return QObject::qt_metacast(_clname);
}

int Axis::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void Axis::axisChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_Lines[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
       7,    6,    6,    6, 0x05,
      34,   24,    6,    6, 0x05,

 // slots: signature, parameters, type, tag, flags
      67,    6,    6,    6, 0x0a,
      80,    6,    6,    6, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Lines[] = {
    "Lines\0\0needFullRedraw()\0tMin,tMax\0"
    "needPartialRedraw(double,double)\0"
    "clearLines()\0cycleLines()\0"
};

void Lines::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Lines *_t = static_cast<Lines *>(_o);
        switch (_id) {
        case 0: _t->needFullRedraw(); break;
        case 1: _t->needPartialRedraw((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 2: _t->clearLines(); break;
        case 3: _t->cycleLines(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Lines::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Lines::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Lines,
      qt_meta_data_Lines, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Lines::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Lines::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Lines::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Lines))
        return static_cast<void*>(const_cast< Lines*>(this));
    return QObject::qt_metacast(_clname);
}

int Lines::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void Lines::needFullRedraw()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void Lines::needPartialRedraw(double _t1, double _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_Plot[] = {

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
      12,    6,    5,    5, 0x0a,
      33,    5,    5,    5, 0x0a,
      50,    5,    5,    5, 0x08,
      66,    5,    5,    5, 0x08,
      98,   88,    5,    5, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Plot[] = {
    "Plot\0\0value\0setAutoScaling(bool)\0"
    "autoScaleForce()\0refreshPixmap()\0"
    "scaleAndFullRefresh()\0tMin,tMax\0"
    "scaleAndPartialRefresh(double,double)\0"
};

void Plot::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Plot *_t = static_cast<Plot *>(_o);
        switch (_id) {
        case 0: _t->setAutoScaling((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->autoScaleForce(); break;
        case 2: _t->refreshPixmap(); break;
        case 3: _t->scaleAndFullRefresh(); break;
        case 4: _t->scaleAndPartialRefresh((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Plot::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Plot::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Plot,
      qt_meta_data_Plot, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Plot::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Plot::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Plot::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Plot))
        return static_cast<void*>(const_cast< Plot*>(this));
    return QWidget::qt_metacast(_clname);
}

int Plot::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
QT_END_MOC_NAMESPACE
