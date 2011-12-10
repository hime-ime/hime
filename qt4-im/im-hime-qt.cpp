#include <cassert>

#include <Qt>
#include <QInputContextPlugin>

using namespace Qt;

#include "hime-qt.h"
#include "hime-imcontext-qt.h"

/* Static Variables */
// static HIMEClientQt *client = NULL;

/* The class Definition */
class HIMEInputContextPlugin: public QInputContextPlugin
{

    private:

        static QStringList hime_languages;

    public:

        HIMEInputContextPlugin ();

        ~HIMEInputContextPlugin ();

        QStringList keys () const;

        QStringList languages (const QString &key);

        QString description (const QString &key);

        QInputContext *create (const QString &key);

        QString displayName (const QString &key);

};


/* Implementations */
QStringList HIMEInputContextPlugin::hime_languages;


HIMEInputContextPlugin::HIMEInputContextPlugin ()
{
}


HIMEInputContextPlugin::~HIMEInputContextPlugin ()
{
#if 0
    delete client;
    client = NULL;
#endif
}

QStringList HIMEInputContextPlugin::keys () const {
    QStringList identifiers;
    identifiers.push_back (HIME_IDENTIFIER_NAME);
    return identifiers;
}


QStringList HIMEInputContextPlugin::languages (const QString &key)
{
    if (hime_languages.empty ()) {
        hime_languages.push_back ("zh_TW");
        hime_languages.push_back ("zh_HK");
        hime_languages.push_back ("zh_CN");
        hime_languages.push_back ("ja");
    }
    return hime_languages;
}


QString HIMEInputContextPlugin::description (const QString &key)
{
    return QString::fromUtf8 ("Qt immodule plugin for hime");
}


QInputContext *HIMEInputContextPlugin::create (const QString &key)
{
    if (key.toLower () != HIME_IDENTIFIER_NAME) {
        return NULL;
    } else {
        return new HIMEIMContext;
    }
}


QString HIMEInputContextPlugin::displayName (const QString &key)
{
    return key;
}

Q_EXPORT_PLUGIN2 (HIMEInputContextPlugin, HIMEInputContextPlugin)
