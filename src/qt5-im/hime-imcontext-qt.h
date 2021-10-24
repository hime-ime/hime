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

#ifndef HIME_QT5_IM_H
#define HIME_QT5_IM_H

#include <QtGui/qpa/qplatforminputcontext.h>

class QInputMethodEvent;
struct HIME_client_handle_S;

class QHimePlatformInputContext : public QPlatformInputContext {
    Q_OBJECT
  public:
    QHimePlatformInputContext ();
    virtual ~QHimePlatformInputContext ();

    virtual bool filterEvent (const QEvent *event);
    virtual bool isValid () const;
    virtual void invokeAction (QInputMethod::Action, int cursorPosition);
    virtual void reset ();
    virtual void commit ();
    virtual void update (Qt::InputMethodQueries quries);
    virtual void setFocusObject (QObject *object);

  private:
    HIME_client_handle_S *hime_ch;
    void send_event (QInputMethodEvent &e);
    void update_preedit ();
    void cursorMoved ();
    bool send_key_press (quint32 keysym, quint32 state);
    void commitPreedit ();
    void send_str (char *s);
};

#endif /* HIME_QT5_IM_H */
