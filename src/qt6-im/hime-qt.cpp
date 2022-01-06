/*
 * Copyright (C) 2022 The HIME team, Taiwan
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

#include "../util.h"
#include "hime-qt.h"
#include <cstdio>

#define HIMEID "hime"

QStringList QHimePlatformInputContextPlugin::keys () const {
    dbg ("QStringList QHimePlatformInputContextPlugin::keys()\n");
    return QStringList (QStringLiteral (HIMEID));
}

QHimePlatformInputContext *QHimePlatformInputContextPlugin::create (const QString &system, const QStringList &paramList) {
    Q_UNUSED (paramList);
    dbg ("QHimePlatformInputContextPlugin::create()\n");

    if (system.compare (system, QStringLiteral (HIMEID), Qt::CaseInsensitive) == 0)
        return new QHimePlatformInputContext;
    return 0;
}
