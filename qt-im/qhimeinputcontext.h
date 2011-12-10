/****************************************************************************
** $Id: qximinputcontext_p.h,v 1.6 2004/06/22 06:47:27 daisuke Exp $
**
** Definition of QHIMEInputContext
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the input method module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QXIMINPUTCONTEXT_H
#define QXIMINPUTCONTEXT_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of internal files.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//
//


#include <qinputcontext.h>
#include <private/qt_x11_p.h>

#include "hime-im-client.h"

class QHIMEInputContext : public QInputContext
{
    Q_OBJECT
public:
    QHIMEInputContext();
    ~QHIMEInputContext();

    QString identifierName();
    QString language();

    bool x11FilterEvent( QWidget *keywidget, XEvent *event );
    void reset();

    void setFocus();
    void unsetFocus();
    void setMicroFocus( int x, int y, int w, int h, QFont *f = 0 );
    void mouseHandler( int x, QEvent::Type type,
		       Qt::ButtonState button, Qt::ButtonState state );
    bool isPreeditRelocationEnabled();

    void setHolderWidget( QWidget *widget );

    bool hasFocus() const;
    void resetClientState();
    void close( const QString &errMsg );

    void sendIMEvent( QEvent::Type type,
		      const QString &text = QString::null,
		      int cursorPosition = -1, int selLength = 0 );

    void create_xim();
    void close_xim();

    HIME_client_handle *hime_ch;

    QString composingText;
    QMemArray<bool> selectedChars;

protected:
    virtual bool isPreeditPreservationEnabled();  // not a QInputContext func

    QCString _language;

private:
    void setComposePosition(int, int);
    void setComposeArea(int, int, int, int);
    void setXFontSet(const QFont &);

    int lookupString(XKeyEvent *, QCString &, KeySym *, Status *) const;
};


#endif // QXIMINPUTCONTEXT_H
