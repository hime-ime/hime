/****************************************************************************
** $Id$
**
** Implementation of QHIMEInputContextPlugin class
**
** Copyright (C) 2004 immodule for Qt Project.  All rights reserved.
**
** This file is written to contribute to Trolltech AS under their own
** licence. You may use this file under your Qt license. Following
** description is copied from their original file headers. Contact
** immodule-qt@freedesktop.org if any conditions of this licensing are
** not clear to you.
**
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

#include "qhimeinputcontext.h"
#include "qhimeinputcontextplugin.h"
#include <qinputcontextplugin.h>


QHIMEInputContextPlugin::QHIMEInputContextPlugin()
{
//    printf("QHIMEInputContextPlugin\n");
}

QHIMEInputContextPlugin::~QHIMEInputContextPlugin()
{
}

QStringList QHIMEInputContextPlugin::keys() const
{
    return QStringList( "hime" );
}

QInputContext *QHIMEInputContextPlugin::create( const QString & )
{
    return new QHIMEInputContext;
}

QStringList QHIMEInputContextPlugin::languages( const QString & )
{
    return QStringList( "" );
}

QString QHIMEInputContextPlugin::displayName( const QString & )
{
    return tr( "hime" );
}

QString QHIMEInputContextPlugin::description( const QString & )
{
    return tr( "hime input method" );
}


Q_EXPORT_PLUGIN( QHIMEInputContextPlugin )
