/*
 * Copyright (C) 2020 The HIME team, Taiwan
 * Copyright (C) 2008 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#ifndef HIME_QT4_HIME_QT_H
#define HIME_QT4_HIME_QT_H

#include "hime-common-qt.h"
#include <QObject>
#include <QSocketNotifier>

class HIMEQt : public QObject {

    Q_OBJECT

  public slots:
    void handle_message ();

  public:
    /**
     * Constructor.
     */
    HIMEQt ();

    /**
     * Destructor.
     */
    ~HIMEQt ();

    /**
     * A messenger is opened.
     */
    void messenger_opened ();

    /**
     * A messenger is closed.
     */
    void messenger_closed ();

  private:
    /**
     * The notifier for the messenger socket.
     */
    QSocketNotifier *socket_notifier;
};

#endif /* HIME_QT4_HIME_QT_H */
