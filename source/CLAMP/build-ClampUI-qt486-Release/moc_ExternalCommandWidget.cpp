/****************************************************************************
** Meta object code from reading C++ file 'ExternalCommandWidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CLAMP_UI/Control/ExternalCommandWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ExternalCommandWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ExternalCommandWidget[] = {

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
      29,   23,   22,   22, 0x08,
      60,   56,   22,   22, 0x08,
      88,   82,   22,   22, 0x08,
     115,   82,   22,   22, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ExternalCommandWidget[] = {
    "ExternalCommandWidget\0\0state\0"
    "enableCommandChanged(bool)\0adc\0"
    "setCommandSource(int)\0index\0"
    "setVoltageSensitivity(int)\0"
    "setCurrentSensitivity(int)\0"
};

void ExternalCommandWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ExternalCommandWidget *_t = static_cast<ExternalCommandWidget *>(_o);
        switch (_id) {
        case 0: _t->enableCommandChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->setCommandSource((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->setVoltageSensitivity((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->setCurrentSensitivity((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ExternalCommandWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ExternalCommandWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ExternalCommandWidget,
      qt_meta_data_ExternalCommandWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ExternalCommandWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ExternalCommandWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ExternalCommandWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ExternalCommandWidget))
        return static_cast<void*>(const_cast< ExternalCommandWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int ExternalCommandWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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