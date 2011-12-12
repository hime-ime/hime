/* Copyright (C) 2009 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#include <QApplication>
#include <QEvent>
#include <QFont>
#include <QInputContext>
#include <QInputMethodEvent>
#include <QObject>
#include <QPoint>
#include <QWidget>
#include <QX11Info>

struct HIME_client_handle_S;

class HIMEIMContext: public QInputContext {
    public:
        HIMEIMContext ();
        ~HIMEIMContext ();
        bool x11FilterEvent (QWidget *widget, XEvent *event);
        bool filterEvent (const QEvent *event);
        void update();
        QString identifierName();
        QString language();
        void mouseHandler (int offset, QMouseEvent *event);
        void setFocusWidget (QWidget *widget);
        void widgetDestroyed (QWidget *widget);
        void reset ();
        HIME_client_handle_S *hime_ch;
        bool isComposing() const;
        void update_cursor(QWidget *);
        void update_preedit();
};
