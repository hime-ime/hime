/****************************************************************************
** Meta object code from reading C++ file 'hime-qt.h'
**
** Created: Mon Dec 19 23:13:09 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "hime-qt.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'hime-qt.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_HIMEQt[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_HIMEQt[] = {
    "HIMEQt\0\0handle_message()\0"
};

const QMetaObject HIMEQt::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_HIMEQt,
      qt_meta_data_HIMEQt, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &HIMEQt::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *HIMEQt::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *HIMEQt::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HIMEQt))
        return static_cast<void*>(const_cast< HIMEQt*>(this));
    return QObject::qt_metacast(_clname);
}

int HIMEQt::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: handle_message(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
