/* Copyright (C) 2008 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#include <QObject>
#include <QSocketNotifier>

//#include "hime-imcontext-qt.h"
#include "hime-common-qt.h"

class HIMEQt: public QObject
{

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
