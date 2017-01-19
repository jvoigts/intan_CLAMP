/****************************************************************************
** Meta object code from reading C++ file 'ControlWindow.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../CLAMP_UI/Control/ControlWindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ControlWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ControlWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      24,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x05,
      41,   35,   14,   14, 0x05,
      73,   67,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      95,   14,   14,   14, 0x0a,
     115,   35,   14,   14, 0x08,
     145,   14,   14,   14, 0x08,
     167,  159,   14,   14, 0x08,
     189,   67,   14,   14, 0x08,
     207,   67,   14,   14, 0x08,
     230,   67,   14,   14, 0x08,
     253,   14,   14,   14, 0x08,
     289,  281,   14,   14, 0x08,
     324,   14,   14,   14, 0x08,
     341,   14,   14,   14, 0x08,
     358,   14,   14,   14, 0x08,
     373,   14,   14,   14, 0x08,
     392,   14,   14,   14, 0x08,
     406,   14,   14,   14, 0x08,
     434,  427,   14,   14, 0x08,
     458,  451,   14,   14, 0x08,
     476,   14,   14,   14, 0x08,
     495,   14,   14,   14, 0x08,
     519,   14,   14,   14, 0x08,
     540,  527,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ControlWindow[] = {
    "ControlWindow\0\0updateStatsSignal()\0"
    "value\0resistanceChanged(double)\0index\0"
    "clampModeChanged(int)\0setDisplayOptions()\0"
    "setResistanceInternal(double)\0"
    "updateStats()\0running\0setThreadStatus(bool)\0"
    "setClampMode(int)\0tryToSetClampMode(int)\0"
    "setHeadstageFocus(int)\0"
    "setCapacitiveCompensation()\0checked\0"
    "enableCapacitiveCompensation(bool)\0"
    "selectFilename()\0startRecording()\0"
    "startRunning()\0startRunningOnce()\0"
    "stopRunning()\0measureTemperature()\0"
    "enable\0setSaveAux(bool)\0x2Mode\0"
    "setVClampX2(bool)\0openIntanWebsite()\0"
    "keyboardShortcutsHelp()\0about()\0"
    "unit,message\0setStatusMessage(int,QString)\0"
};

void ControlWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        ControlWindow *_t = static_cast<ControlWindow *>(_o);
        switch (_id) {
        case 0: _t->updateStatsSignal(); break;
        case 1: _t->resistanceChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 2: _t->clampModeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->setDisplayOptions(); break;
        case 4: _t->setResistanceInternal((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 5: _t->updateStats(); break;
        case 6: _t->setThreadStatus((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->setClampMode((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->tryToSetClampMode((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->setHeadstageFocus((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: _t->setCapacitiveCompensation(); break;
        case 11: _t->enableCapacitiveCompensation((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: _t->selectFilename(); break;
        case 13: _t->startRecording(); break;
        case 14: _t->startRunning(); break;
        case 15: _t->startRunningOnce(); break;
        case 16: _t->stopRunning(); break;
        case 17: _t->measureTemperature(); break;
        case 18: _t->setSaveAux((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 19: _t->setVClampX2((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 20: _t->openIntanWebsite(); break;
        case 21: _t->keyboardShortcutsHelp(); break;
        case 22: _t->about(); break;
        case 23: _t->setStatusMessage((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData ControlWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ControlWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_ControlWindow,
      qt_meta_data_ControlWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ControlWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ControlWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ControlWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ControlWindow))
        return static_cast<void*>(const_cast< ControlWindow*>(this));
    if (!strcmp(_clname, "CapacitiveCompensationController"))
        return static_cast< CapacitiveCompensationController*>(const_cast< ControlWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int ControlWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 24)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 24;
    }
    return _id;
}

// SIGNAL 0
void ControlWindow::updateStatsSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void ControlWindow::resistanceChanged(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ControlWindow::clampModeChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
static const uint qt_meta_data_KeyboardShortcutDialog[] = {

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

static const char qt_meta_stringdata_KeyboardShortcutDialog[] = {
    "KeyboardShortcutDialog\0"
};

void KeyboardShortcutDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData KeyboardShortcutDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject KeyboardShortcutDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_KeyboardShortcutDialog,
      qt_meta_data_KeyboardShortcutDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &KeyboardShortcutDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *KeyboardShortcutDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *KeyboardShortcutDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_KeyboardShortcutDialog))
        return static_cast<void*>(const_cast< KeyboardShortcutDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int KeyboardShortcutDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
