# New ports collection makefile for:	zh-hime
# Date created:				2012/06/20
# Whom:					The HIME project
# Depends:				git gtk20 intltool libXtst gtk30 gmake ja-anthy
#					zh-libchewing qt4-moc qt4-corelib qt4-gui
#
# $FreeBSD$
#

PORTNAME=	hime
DATE!=		/bin/date "+%Y%m%d"
PORTVERSION=	0.9.10+git${DATE}
CATEGORIES=	chinese textproc
MASTER_SITES=	http://hime.luna.com.tw/

MAINTAINER=	The HIME project
COMMENT=	A GTK2+/GTK3+ based universal input method platform

LICENSE=	LGPL21

USE_GMAKE=	yes
HAS_CONFIGURE=	yes

USE_XORG=	xtst
USE_GNOME=	gtk20
USE_LDCONFIG=	yes
WITH_DEBUG=	yes
OPTIONS=	NLS		"Enable Native Language Support"	on \
		GTK3_IM_MODULE	"Build the GTK+3 im-module"		on \
		QT4_IM_MODULE	"Build the QT4 im-module"		on \
		ANTHY		"Support Anthy input method"		on \
		CHEWING		"Support Chewing input method"		on

TMP_PLIST_DIR=	build

do-fetch:

do-extract:
	@${RM} -rf ${WRKDIR}
	@${MKDIR} ${WRKDIR}
	(cd ${WRKDIR} && \
	 git clone --depth=1 git://github.com/hime-ime/hime.git && \
	 ${LN} -s ${PORTNAME} ${PORTNAME}-${PORTVERSION})

pre-patch:
	@${SED} -i -e 's/^\(	install -m 644 \*\.1 \$$(man1dir)\)$$/\1 \&\& find .\/ -name "*.1" -exec gzip $$(man1dir)\/{} \\;/g' ${WRKDIR}/${PORTNAME}/man/Makefile

.include <bsd.port.options.mk>

.if defined(WITH_GTK3_IM_MODULE)
BUILD_DEPENDS+=	${LOCALBASE}/libdata/pkgconfig/gtk+-3.0.pc:${PORTSDIR}/x11-toolkits/gtk30
GTK3_BINARY_VERSION!=	/usr/local/bin/pkg-config --variable=gtk_binary_version gtk+-3.0
.else
CONFIGURE_ARGS+=--disable-gtk3-im-module
.endif

.if defined(WITH_QT4_IM_MODULE)
BUILD_DEPENDS+=	/usr/local/libdata/pkgconfig/QtGui.pc:${PORTSDIR}/x11-toolkits/qt4-gui \
		/usr/local/libdata/pkgconfig/QtCore.pc:${PORTSDIR}/devel/qt4-corelib \
		/usr/local/bin/moc-qt4:${PORTSDIR}/devel/qt4-moc
QT_NONSTANDARD=	yes
.else
CONFIGURE_ARGS+=--disable-qt4-immodule
.endif

.if defined(WITH_ANTHY)
BUILD_DEPENDS+=	${LOCALBASE}/include/anthy/anthy.h:${PORTSDIR}/japanese/anthy
.else
CONFIGURE_ARGS+=--disable-anthy
.endif

.if defined(WITH_CHEWING)
BUILD_DEPENDS+=	zh-libchewing>=0.3.2:${PORTSDIR}/chinese/libchewing
.else
CONFIGURE_ARGS+=--disable-chewing
.endif

.if !defined(WITHOUT_NLS)
USE_GETTEXT=	yes
.else
CONFIGURE_ARGS+=--disable-nls
.endif

post-install:
	@${RM} -rf ${WRKDIR}/${TMP_PLIST_DIR}
	@${MKDIR} ${WRKDIR}/${TMP_PLIST_DIR}
	( cd ${WRKDIR}/${PORTNAME} && \
	  ${GMAKE} DESTDIR=${WRKDIR}/${TMP_PLIST_DIR} install && \
	  cd ${WRKDIR}/${TMP_PLIST_DIR}${PREFIX} && \
	  ${FIND} ./ -type f > ${TMPPLIST} && \
	  ${FIND} ./ -type l >> ${TMPPLIST} && \
	  mtree -U -f ${MTREE_FILE} && \
	  DIRLIST=`mtree -L -d -f ${MTREE_FILE} | ${SED} -e 's/ extra$$//g'` && \
	  for DIR in $$DIRLIST; do \
	  	case "$$DIR" in \
			lib/gtk-2.0|lib/gtk-3.0|share/locale/en_US|lib/qt4) ;; \
			*) ${FIND} $$DIR -type d | ${SORT} -r | ${SED} -e 's/^/@dirrm /g' >> ${TMPPLIST};; \
		esac ; \
	  done; \
	  echo '@exec gtk-query-immodules-2.0 > %D/etc/gtk-2.0/gtk.immodules || /usr/bin/true' >> ${TMPPLIST}; \
	  echo '@unexec gtk-query-immodules-2.0 > %D/etc/gtk-2.0/gtk.immodules || /usr/bin/true' >> ${TMPPLIST} )
.if defined(WITH_GTK3_IM_MODULE)
	  echo '@exec gtk-query-immodules-3.0 > %D/lib/gtk-3.0/${GTK3_BINARY_VERSION}/immodules.cache 2>/dev/null || /usr/bin/true' >> ${TMPPLIST};
	  echo '@unexec gtk-query-immodules-3.0 > %D/lib/gtk-3.0/${GTK3_BINARY_VERSION}/immodules.cache 2>/dev/null || /usr/bin/true' >> ${TMPPLIST};
.endif

.include <bsd.port.mk>
