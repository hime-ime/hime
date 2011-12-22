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

#include "qhimeinputcontext.h"
#include "qwidget.h"

#include <stdlib.h>
#include <X11/keysymdef.h>
#include "hime-im-client.h"


QHIMEInputContext::QHIMEInputContext()
    : QInputContext(), hime_ch(NULL)
{
//    printf("create_xim\n");
    Display *appDpy = QPaintDevice::x11AppDisplay();

    if (!hime_ch) {
      if (!(hime_ch = hime_im_client_open(appDpy)))
        perror("cannot open hime_ch");
        return;
    }
}


void QHIMEInputContext::setHolderWidget( QWidget *widget )
{

    if ( ! widget )
	return;

    QInputContext::setHolderWidget( widget );

//    printf("setHolderWidget %x\n",  widget->winId());

    if (! widget->isTopLevel()) {
	qWarning("QInputContext: cannot create input context for non-toplevel widgets");
	return;
    }

    if (hime_ch) {
        hime_im_client_set_window(hime_ch, widget->winId());
    }
}


QHIMEInputContext::~QHIMEInputContext()
{
//    printf("QHIMEInputContext::~QHIMEInputContext()\n");
    hime_im_client_close(hime_ch);
    hime_ch = NULL;
}


bool QHIMEInputContext::x11FilterEvent( QWidget *keywidget, XEvent *event )
{
#ifndef QT_NO_HIME
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

            sendIMEvent( QEvent::IMStart );
            sendIMEvent( QEvent::IMEnd, inputText );
        }
    } else {
       result = hime_im_client_forward_key_release(hime_ch,
         keysym, state, &rstr);
    }


    if (rstr)
        free(rstr);

    return result;
}


void QHIMEInputContext::sendIMEvent( QEvent::Type type, const QString &text,
				    int cursorPosition, int selLength )
{
    QInputContext::sendIMEvent( type, text, cursorPosition, selLength );
    if ( type == QEvent::IMCompose )
	composingText = text;
}


void QHIMEInputContext::reset()
{
//    printf("reset %x %d %d\n", focusWidget(), isComposing(), composingText.isNull());

    if ( focusWidget() && isComposing() && ! composingText.isNull() ) {
	QInputContext::reset();

	resetClientState();
    }
}


void QHIMEInputContext::resetClientState()
{
//    printf("resetClientState\n");
}


void QHIMEInputContext::close( const QString &errMsg )
{
//    printf("close\n");
    qDebug( "%s", (const char*) errMsg );
    emit deletionRequested();
}


bool QHIMEInputContext::hasFocus() const
{
//    printf("hasFocus\n");
    return ( focusWidget() != 0 );
}


void QHIMEInputContext::setMicroFocus(int x, int y, int, int h, QFont *f)
{
    QWidget *widget = focusWidget();

    if (widget ) {
	QPoint p( x, y );
	QPoint p2 = widget->mapTo( widget->topLevelWidget(), QPoint( 0, 0 ) );
	p = widget->topLevelWidget()->mapFromGlobal( p );
	setComposePosition(p.x(), p.y() + h);
   }
}

void QHIMEInputContext::mouseHandler( int , QEvent::Type type,
				     Qt::ButtonState button,
				     Qt::ButtonState)
{
}

void QHIMEInputContext::setComposePosition(int x, int y)
{
//    printf("setComposePosition %d %d\n", x, y);
    if (hime_ch) {
      hime_im_client_set_cursor_location(hime_ch, x, y);
    }
}


void QHIMEInputContext::setComposeArea(int x, int y, int w, int h)
{
//    printf("setComposeArea %d %d %d %d\n", x, y, w, h);
}


void QHIMEInputContext::setFocus()
{
//    printf("setFocus\n", hime_ch);

    if (hime_ch) {
      hime_im_client_focus_in(hime_ch);
    }
}

void QHIMEInputContext::unsetFocus()
{
//    printf("unsetFocus\n");
    if (hime_ch) {
      hime_im_client_focus_out(hime_ch);
    }
}


bool QHIMEInputContext::isPreeditRelocationEnabled()
{
    return ( language() == "ja" );
}


bool QHIMEInputContext::isPreeditPreservationEnabled()
{
    return ( language() == "ja" );
}


QString QHIMEInputContext::identifierName()
{
    return "hime";
}


QString QHIMEInputContext::language()
{
    QString locale("zh_TW");

    _language = locale;

    return _language;
}

#endif //QT_NO_IM
