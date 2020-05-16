/*
 * Copyright (C) 2020 The HIME team, Taiwan
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef HIME_QT5_PLUGIN_H
#define HIME_QT5_PLUGIN_H

#include <QtGui/qpa/qplatforminputcontextplugin_p.h>

#include "hime-imcontext-qt.h"
#include <QtCore/QStringList>

class QHimePlatformInputContextPlugin : public QPlatformInputContextPlugin {
    Q_OBJECT
  public:
    Q_PLUGIN_METADATA (IID QPlatformInputContextFactoryInterface_iid FILE "hime.json")
    QStringList keys () const;
    QHimePlatformInputContext *create (const QString &system, const QStringList &paramList);
};

#endif /* HIME_QT5_PLUGIN_H */
