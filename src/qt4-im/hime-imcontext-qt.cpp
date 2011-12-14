/* Copyright (C) 2011 Edward Der-Hua Liu, Hsin-Chu, Taiwan
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

#include "hime-imcontext-qt.h"
#include "hime-common-qt.h"
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <cstdio>
#include "hime-im-client.h"
#include <QColor>
#include <QPalette>
#include <QInputMethodEvent>
#include <QTextCharFormat>

using namespace Qt;
static QWidget *focused_widget;

typedef QInputMethodEvent::Attribute QAttribute;

HIMEIMContext::HIMEIMContext ()
{
  Display *display = QX11Info::display();
  if (!(hime_ch = hime_im_client_open(display))) {
    perror("cannot open hime_ch");
    return;
  }
}

HIMEIMContext::~HIMEIMContext ()
{
  if (hime_ch) {
    hime_im_client_close(hime_ch);
    hime_ch = NULL;
  }
}

QString HIMEIMContext::identifierName()
{
  return HIME_IDENTIFIER_NAME;
}

void HIMEIMContext::mouseHandler (int offset, QMouseEvent *event)
{
}

void HIMEIMContext::widgetDestroyed (QWidget *widget)
{
}

void HIMEIMContext::reset ()
{
  if (hime_ch) {
    hime_im_client_reset(hime_ch);
    update_preedit();
  }
}

void HIMEIMContext::update_cursor(QWidget *fwidget)
{
  hime_im_client_set_window(hime_ch, fwidget->winId());
  QRect rect = fwidget->inputMethodQuery (ImMicroFocus).toRect ();
  QPoint point (rect.x (), rect.y () + rect.height ());
  QPoint gxy = fwidget->mapToGlobal (point);
  if (hime_ch) {
    Display *dpy = QX11Info::display();
    WId ow;
    int wx, wy;
    XTranslateCoordinates(dpy, fwidget->winId(), DefaultRootWindow(dpy),
    0,0,  &wx, &wy, &ow);

    hime_im_client_set_cursor_location(hime_ch, gxy.x()-wx, gxy.y()-wy);
  }
}

void HIMEIMContext::update_preedit()
{
  QList<QAttribute> preedit_attributes;
//  QString preedit_string;
  int preedit_cursor_position=0;
  int sub_comp_len;
  char *str=NULL;
  HIME_PREEDIT_ATTR att[HIME_PREEDIT_ATTR_MAX_N];
  int attN = hime_im_client_get_preedit(hime_ch, &str, att, &preedit_cursor_position, &sub_comp_len);

  if (hime_ch) {
    int ret;
    hime_im_client_set_flags(hime_ch, FLAG_HIME_client_handle_use_preedit, &ret);
  }

  preedit_attributes.push_back (QAttribute (QInputMethodEvent::Cursor, preedit_cursor_position, true, 0));

  const QWidget *focused_widget = qApp->focusWidget ();
  if (!focused_widget || !str) {
free_mem:
    free(str);
    return;
  }
  const QPalette &palette = focused_widget->palette ();
  if (&palette==NULL)
    goto free_mem;
  const QBrush &reversed_foreground = palette.base ();
  const QBrush &reversed_background = palette.text ();

#if DBG || 0
  printf("update_preedit attN:%d '%s'\n", attN, str);
#endif
  int i;
  for(i=0; i < attN; i++) {
    int ofs0 = att[i].ofs0;
    int len = att[i].ofs1 - att[i].ofs0;

    switch (att[i].flag) {
      case HIME_PREEDIT_ATTR_FLAG_REVERSE:
          {
              QTextCharFormat text_format;
              text_format.setForeground (reversed_foreground);
              text_format.setBackground (reversed_background);
              QAttribute qt_attribute (QInputMethodEvent::TextFormat, ofs0, len, text_format);
              preedit_attributes.push_back (qt_attribute);
          }
          break;
      case HIME_PREEDIT_ATTR_FLAG_UNDERLINE:
          {
              QTextCharFormat text_format;
              text_format.setProperty (QTextFormat::FontUnderline, true);
              QAttribute qt_attribute (QInputMethodEvent::TextFormat, ofs0, len, text_format);
              preedit_attributes.push_back (qt_attribute);
          }
    }
  }

  QInputMethodEvent im_event (QString::fromUtf8(str), preedit_attributes);
  sendEvent (im_event);
  free(str);
}

void HIMEIMContext::update()
{
    QWidget *focused_widget = qApp->focusWidget ();
    if (focused_widget != NULL) {
        if (focused_widget == NULL) {
          if (hime_ch)
            hime_im_client_focus_in(hime_ch);
        }

        update_cursor(focused_widget);
    }
}

QString HIMEIMContext::language ()
{
    return "";
}

void HIMEIMContext::setFocusWidget(QWidget *widget)
{
  if (!widget)
    return;

  if (focused_widget != widget) {
#if 0
    if (focused_widget) {
      char *rstr;
      hime_im_client_focus_out2(hime_ch, &rstr);
      if (rstr) {
          QString inputText = QString::fromUtf8(rstr);
          QInputMethodEvent commit_event;
          commit_event.setCommitString (inputText);
          sendEvent (commit_event);

          QList<QAttribute> preedit_attributes;
          QInputMethodEvent im_event (QString::fromUtf8(""), preedit_attributes);
          sendEvent (im_event);
      }
    }
    focused_widget = widget;
#else
    hime_im_client_focus_out(hime_ch);
#endif
  }

  if (hime_ch) {
    hime_im_client_set_window(hime_ch, widget->winId());
  }

  QInputContext::setFocusWidget (widget);
//  puts("setFocusWidget");
  if (hime_ch)
    hime_im_client_focus_in(hime_ch);
}

bool HIMEIMContext::x11FilterEvent (QWidget *widget, XEvent *event)
{
  KeySym keysym;
  char static_buffer[256];
  char *buffer = static_buffer;
  int buffer_size = sizeof(static_buffer) - 1;

  if (event->type != KeyPress && event->type != KeyRelease)
      return TRUE;

  XKeyEvent *keve = (XKeyEvent *) event;

  XLookupString (keve, buffer, buffer_size, &keysym, NULL);
  int result;
  char *rstr = NULL;
  unsigned int state = keve->state;


  if (event->type == KeyPress) {
      result = hime_im_client_forward_key_press(hime_ch,
        keysym, state, &rstr);

      if (rstr) {
          QString inputText = QString::fromUtf8(rstr);
          QInputMethodEvent commit_event;
          commit_event.setCommitString (inputText);
          sendEvent (commit_event);
      }
  } else {
     result = hime_im_client_forward_key_release(hime_ch,
       keysym, state, &rstr);
  }

  if (result)
    update_preedit();

  update_cursor(widget);

  if (rstr)
      free(rstr);

  return result;
}

bool HIMEIMContext::filterEvent (const QEvent *event)
{
  return FALSE;
}

bool HIMEIMContext::isComposing() const
{
  char *str;
  HIME_PREEDIT_ATTR att[HIME_PREEDIT_ATTR_MAX_N];
  int preedit_cursor_position, sub_comp_len;
  hime_im_client_get_preedit(hime_ch, &str, att, &preedit_cursor_position, &sub_comp_len);
  bool is_compose = str[0]>0;
  free(str);

  return is_compose;
}
