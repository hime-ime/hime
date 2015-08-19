#include <cstdio>

#include "hime-qt.h"
#include "../util.h"

#define HIMEID "hime"


QStringList QHimePlatformInputContextPlugin::keys() const
{
    dbg("QStringList QHimePlatformInputContextPlugin::keys()\n");
    return QStringList(QStringLiteral(HIMEID));

}

QHimePlatformInputContext *QHimePlatformInputContextPlugin::create(const QString& system, const QStringList& paramList)
{
    Q_UNUSED(paramList);
    dbg("QHimePlatformInputContextPlugin::create()\n");

    if (system.compare(system, QStringLiteral(HIMEID), Qt::CaseInsensitive) == 0)
        return new QHimePlatformInputContext;
    return 0;
}
