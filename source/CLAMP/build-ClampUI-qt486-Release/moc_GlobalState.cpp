/****************************************************************************
** Meta object code from reading C++ file 'GlobalState.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CLAMP_UI/GlobalState.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GlobalState.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GlobalState[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      24,   13,   12,   12, 0x05,
      65,   57,   12,   12, 0x05,
      91,   12,   12,   12, 0x05,
     122,  108,   12,   12, 0x05,
     166,  153,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
     193,  108,   12,   12, 0x0a,
     231,   12,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_GlobalState[] = {
    "GlobalState\0\0unit,value\0"
    "pipetteOffsetChanged(int,double)\0"
    "running\0threadStatusChanged(bool)\0"
    "threadFinished()\0title,message\0"
    "error(const char*,const char*)\0"
    "unit,message\0statusMessage(int,QString)\0"
    "errorMessage(const char*,const char*)\0"
    "finishThread()\0"
};

void GlobalState::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        GlobalState *_t = static_cast<GlobalState *>(_o);
        switch (_id) {
        case 0: _t->pipetteOffsetChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 1: _t->threadStatusChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->threadFinished(); break;
        case 3: _t->error((*reinterpret_cast< const char*(*)>(_a[1])),(*reinterpret_cast< const char*(*)>(_a[2]))); break;
        case 4: _t->statusMessage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 5: _t->errorMessage((*reinterpret_cast< const char*(*)>(_a[1])),(*reinterpret_cast< const char*(*)>(_a[2]))); break;
        case 6: _t->finishThread(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData GlobalState::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GlobalState::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_GlobalState,
      qt_meta_data_GlobalState, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GlobalState::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GlobalState::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GlobalState::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GlobalState))
        return static_cast<void*>(const_cast< GlobalState*>(this));
    return QObject::qt_metacast(_clname);
}

int GlobalState::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void GlobalState::pipetteOffsetChanged(int _t1, double _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void GlobalState::threadStatusChanged(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void GlobalState::threadFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void GlobalState::error(const char * _t1, const char * _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void GlobalState::statusMessage(int _t1, QString _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_END_MOC_NAMESPACE
