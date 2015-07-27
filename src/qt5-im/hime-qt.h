#ifndef HIME_QT5_PLUGIN_H
#define HIME_QT5_PLUGIN_H

#include <QtCore/QStringList>
#include <qpa/qplatforminputcontextplugin_p.h>

#include "hime-imcontext-qt.h"


class QHimePlatformInputContextPlugin : public QPlatformInputContextPlugin
{
    Q_OBJECT
public:
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QPlatformInputContextFactoryInterface" FILE "hime.json")
    QStringList keys() const;
    QHimePlatformInputContext *create(const QString& system, const QStringList& paramList);
};

#endif
