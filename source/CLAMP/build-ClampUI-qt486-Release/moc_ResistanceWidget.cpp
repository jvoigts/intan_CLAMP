/****************************************************************************
** Meta object code from reading C++ file 'ResistanceWidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CLAMP_UI/Control/ResistanceWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ResistanceWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ResistanceParams[] = {

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

static const char qt_meta_stringdata_ResistanceParams[] = {
    "ResistanceParams\0\0valuesChanged()\0"
    "notifyObserver()\0"
};

void ResistanceParams::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ResistanceParams *_t = static_cast<ResistanceParams *>(_o);
        switch (_id) {
        case 0: _t->valuesChanged(); break;
        case 1: _t->notifyObserver(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData ResistanceParams::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ResistanceParams::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ResistanceParams,
      qt_meta_data_ResistanceParams, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ResistanceParams::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ResistanceParams::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ResistanceParams::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ResistanceParams))
        return static_cast<void*>(const_cast< ResistanceParams*>(this));
    return QObject::qt_metacast(_clname);
}

int ResistanceParams::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void ResistanceParams::valuesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_ResistanceParamsDisplay[] = {

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

static const char qt_meta_stringdata_ResistanceParamsDisplay[] = {
    "ResistanceParamsDisplay\0\0redoLayout()\0"
};

void ResistanceParamsDisplay::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ResistanceParamsDisplay *_t = static_cast<ResistanceParamsDisplay *>(_o);
        switch (_id) {
        case 0: _t->redoLayout(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData ResistanceParamsDisplay::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ResistanceParamsDisplay::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ResistanceParamsDisplay,
      qt_meta_data_ResistanceParamsDisplay, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ResistanceParamsDisplay::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ResistanceParamsDisplay::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ResistanceParamsDisplay::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ResistanceParamsDisplay))
        return static_cast<void*>(const_cast< ResistanceParamsDisplay*>(this));
    return QWidget::qt_metacast(_clname);
}

int ResistanceParamsDisplay::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
static const uint qt_meta_data_ResistanceDialog[] = {

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

static const char qt_meta_stringdata_ResistanceDialog[] = {
    "ResistanceDialog\0"
};

void ResistanceDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData ResistanceDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ResistanceDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_ResistanceDialog,
      qt_meta_data_ResistanceDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ResistanceDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ResistanceDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ResistanceDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ResistanceDialog))
        return static_cast<void*>(const_cast< ResistanceDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int ResistanceDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
