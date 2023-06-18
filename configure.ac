#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([HIME], [0.10.1], [BUG-REPORT-ADDRESS])
AC_PROG_CC()
AC_PROG_CXX()

AM_INIT_AUTOMAKE([foreign])

AC_DEFUN(
  [LIBRARY_CHECK_MESSAGE],
  [
    AS_IF(
      [$1],
      [
        AS_IF(
          [$2],
          [AC_MSG_NOTICE([$3 is used])],
          [AC_MSG_NOTICE([$3 is installed but not used])]
        )
      ],
      [
        AS_IF(
          [$2],
          [AC_MSG_ERROR([$3 should be installed])],
          [AC_MSG_NOTICE([$3 is not installed and not used])]
        )
      ]
    )
  ]
)

AC_SUBST([CCLD], [$CC])
AC_SUBST([CCX], [$CC])
# Set CFLAGS to empty string since AC_PROG_CC() would set it with the default value "-g -O2"
AC_SUBST([CFLAGS], [""])
AS_IF(
  [test -z "$CFLAGS"],
  [
    AC_SUBST([CFLAGS], ["-std=gnu17"])
    AC_SUBST([CXXFLAGS], ["-std=gnu++17"])
    AC_SUBST([OPTFLAGS], ["-Wall"])
  ],
  [AC_SUBST([OPTFLAGS], ["$CFLAGS"])]
)
AC_SUBST([CFLAGS], ["$CFLAGS $CPPFLAGS"])
AC_SUBST([CXXFLAGS], ["$CXXFLAGS $CPPFLAGS"])
AC_SUBST([DOC_DIR], ["\${datadir}/doc/hime"])
AS_IF(
  [test "x$ECHO" = "x"],
  [
    AC_SUBST([ECHO], ["$(whereis -b echo | awk "{print \$2}")"])
    AS_IF(
      [test "x$ECHO" = "x"],
      [AC_SUBST([ECHO], ["echo"])]
    )
  ]
)
AC_SUBST([FREEBSD], ["0"])
AS_IF(
  [uname | grep FreeBSD | grep -v GNU/kFreeBSD >/dev/null 2>/dev/null],
  [AC_SUBST([FREEBSD], ["1"])]
)
AS_IF(
  [uname | grep OpenBSD >/dev/null 2>/dev/null],
  [AC_SUBST([FREEBSD], ["1"])]
)
AM_CONDITIONAL([FREEBSD], [test "x$FREEBSD" = "x1"])
AC_SUBST([HIME_BIN_DIR], ["\${bindir}"])
AC_SUBST([HIME_DEFAULT_ICON_DIR], ["\${datadir}/pixmaps/hime"])
AC_SUBST([HIME_OGG_DIR], ["\${datadir}/hime/ogg"])
AC_SUBST([HIME_SCRIPT_DIR], ["\${datadir}/hime/script"])
AC_SUBST([HIME_TABLE_DIR], ["\${datadir}/hime/table"])
AC_SUBST([HIME_VERSION], ["$(head -n 1 ChangeLog)"])
AC_SUBST([SO_FLAGS], ["$LDFLAGS -shared -Wl,--as-needed"])
AC_SUBST([LDFLAGS_ENV], ["$LDFLAGS"])
AC_SUBST([LDFLAGS], ["$LDFLAGS -Wl,--as-needed -lX11 -lm"])
AC_SUBST([MAKE], ["make"])
AS_IF(
  [command -v gmake > /dev/null 2> /dev/null],
  [AC_SUBST([MAKE], ["gmake"])]
)
AC_SUBST([SYS_ICON_DIR], ["\${datadir}/pixmaps"])
AC_SUBST([bindir], ["\${DESTDIR}$bindir"])
AC_SUBST([datadir], ["\${DESTDIR}$datadir"])
AC_SUBST([filterdir], ["\${datadir}/hime/filter"])
AC_SUBST([includedir], ["\${DESTDIR}$includedir"])
AC_SUBST([mandir], ["\${DESTDIR}$mandir"])
AC_SUBST([man1dir], ["\${mandir}/man1"])

# USE_XIM
AC_ARG_ENABLE(
  [xim],
  [AS_HELP_STRING([--enable-xim], [Enable XIM, default is yes])],
  [enable_xim=$enableval],
  [enable_xim=yes]
)
AM_CONDITIONAL([USE_XIM], [test "x$enable_xim" = "xyes"])
# USE_SYSTEM_TRAY
AC_ARG_ENABLE(
  [system-tray],
  [AS_HELP_STRING([--enable-system-tray], [Enable System Tray, default is yes])],
  [enable_system_tray=$enableval],
  [enable_system_tray=yes]
)
AM_CONDITIONAL([USE_SYSTEM_TRAY], [test "x$enable_system_tray" = "xyes"])
# LIB
AC_SUBST([LIB], ["lib"])
AS_IF(
  [test -d /usr/lib64 -a -d /lib64],
  [AC_SUBST([LIB], ["lib64"])]
)
AS_IF(
  [test "x$(uname -s)" = "xLinux"],
  [AS_IF(
    [command -v lsb_release >/dev/null],
    [
      distid=$(lsb_release -is 2>/dev/null)
      AS_IF(
        [test "x$distid" = "xUbuntu" -o "x$distid" = "xDebian" -o "x$distid" = "xArch" -o "x$distid" = "xarchlinux"],
        [AC_SUBST([LIB], ["lib"])]
      )
    ],
    [
      DEBIAN_LD=$(ld -v 2>&1 | grep -i debian)
      UBUNTU_LD=$(ld -v 2>&1 | grep -i ubuntu)
      if [ test "x$DEBIAN_LD" != "x" -o "x$UBUNTU_LD" != "x"]; then
        AC_SUBST([LIB], ["lib"])
      elif [ test -e /etc/arch-release ]; then
        AC_SUBST([LIB], ["lib"])
      fi
    ]
  )]
)
# use_lib
AC_SUBST([use_lib], [""])
AC_ARG_ENABLE(
  [lib64],
  [AS_HELP_STRING([--enable-lib64], [Use lib64 directory, default is auto-detect])],
  [enable_lib64=$enableval],
  [enable_lib64=""]
)
if [ test "x$enable_lib64" = "xyes"]; then
  AC_SUBST([use_lib], ["lib64"])
elif [ test "x$enable_lib64" = "xno"]; then
  AC_SUBST([use_lib], ["lib"])
elif [ test -n "$enable_lib64"]; then
  AC_SUBST([use_lib], ["$enable_lib64"])
fi
AS_IF(
  [test -n "$use_lib"],
  [AC_SUBST([LIB], ["$use_lib"])]
)
AC_SUBST([libdir], ["\${prefix}/$LIB"])
AC_SUBST([libdir], ["\${DESTDIR}$libdir"])
AC_SUBST([himelibdir], ["\${libdir}/hime"])
AC_SUBST([hime_ld_run_path], ["\${himelibdir}"])
AC_SUBST([gcc_ld_run_path], ["-Wl,-rpath,\${hime_ld_run_path}"])
# GIT_HAVE
# GIT_HASH
AS_IF(
  [test -d .git],
  [
    AC_SUBST([git_have], ["yes"])
    AC_SUBST([GIT_HASH], ["$(git log 2>/dev/null | head -n 1 | cut -d ' ' -f 2 | head -c 7)"])
  ]
)
AM_CONDITIONAL([GIT_HAVE], [test "x$git_have" = "xyes"])

# Checks for programs.
# pkg-config
PKG_PROG_PKG_CONFIG()

# Checks for libraries.
# Xtst
PKG_CHECK_MODULES(
  [XTST],
  [xtst],
  [],
  [AC_MSG_ERROR([Please install Xtst])]
)
# GTK
PKG_CHECK_MODULES(
  [GTK2],
  [gtk+-2.0 >= 2.24.31],
  [gtk2_is_installed=yes],
  [gtk2_is_installed=no]
)
PKG_CHECK_MODULES(
  [GTK3],
  [gtk+-3.0 >= 3.22.30],
  [gtk3_is_installed=yes],
  [gtk3_is_installed=no]
)
AS_IF(
  [test "x$gtk2_is_installed" = "xno" -a "x$gtk3_is_installed" = "xno"],
  [AC_MSG_ERROR([Please install GTK 2 or GTK 3])]
)
AC_ARG_WITH(
  [gtk],
  [AS_HELP_STRING(
    [--with-gtk],
    [Select GTK version, default is GTK 2. [ARG: 2.0|3.0]]
  )],
  [
    with_gtk=$withval
    AS_IF(
      [test "x$with_gtk" != "x3.0"],
      [with_gtk=2.0]
    )
  ],
  [with_gtk=2.0]
)
AS_IF(
  [test "x$with_gtk" = "x2.0"],
  [
    AC_SUBST([GTK], ["gtk+-2.0"])
    AC_SUBST([GTK_IM], ["Y"])
    AC_SUBST([GTK3_IM], ["N"])
    AC_SUBST([GTKINC], ["$GTK2_CFLAGS"])
    AC_SUBST([GTKLDFLAGS], ["$GTK2_LIBS"])
    AC_SUBST([APPINDICATOR], ["appindicator-0.1"])
  ],
  [
    AC_SUBST([GTK], ["gtk+-3.0"])
    AC_SUBST([GTK_IM], ["N"])
    AC_SUBST([GTK3_IM], ["Y"])
    AC_SUBST([GTKINC], ["$GTK3_CFLAGS"])
    AC_SUBST([GTKLDFLAGS], ["$GTK3_LIBS"])
    AC_SUBST([APPINDICATOR], ["appindicator3-0.1"])
  ]
)
AM_CONDITIONAL([GTK2], [test "x$with_gtk" = "x2.0"])
# Qt
# QT5_IM
# QT6_IM
AC_SUBST([QT5_IM], ["N"])
AC_SUBST([QT6_IM], ["N"])
QT5_IM_DIR_CUSTOM=
QT6_IM_DIR_CUSTOM=
#                  debian                 fedora               fedora           freebsd                slackware          arch
qt5_moc_path_list="/usr/share/qt5/bin/moc /usr/lib/qt5/bin/moc /usr/bin/moc-qt5 /usr/local/bin/moc-qt5 /usr/local/bin/moc /usr/bin/moc"
qt6_moc_path_list="/usr/share/qt6/bin/moc /usr/lib/qt6/bin/moc /usr/bin/moc-qt6 /usr/local/bin/moc-qt6 /usr/local/bin/moc /usr/lib/qt6/moc"
# QT5
PKG_CHECK_MODULES(
  [QT5],
  [Qt5Core Qt5Gui],
  [
    qt5_is_installed=yes
    AC_SUBST([QT5_IM], ["Y"])
  ],
  [qt5_is_installed=no]
)
AC_ARG_ENABLE(
  [qt5_immodule],
  [AS_HELP_STRING([--enable-qt5-immodule], [Enable QT5 im-module, default is auto-detect])],
  [enable_qt5_immodule=$enableval],
  [enable_qt5_immodule=yes]
)
AC_ARG_WITH(
  [qt5_moc_path],
  [AS_HELP_STRING([--with-qt5-moc-path=PATH], [Specify the path of 'moc' command for QT5])],
  [QT5_MOC_PATH=$withval],
  [QT5_MOC_PATH=]
)
AC_ARG_WITH(
  [qt5_im_module_path],
  [AS_HELP_STRING([--with-qt5-im-module-path=PATH], [Specify the install path for QT5 im-module])],
  [QT5_IM_DIR_CUSTOM=$withval],
  [QT5_IM_DIR_CUSTOM=]
)
AS_IF(
  [test "x$qt5_is_installed" = "xno" -a "x$enable_qt5_immodule" = "xyes"],
  [AC_MSG_ERROR([Require QT5 installed])]
)
AS_IF(
  [test "x$enable_qt5_immodule" = "xno"],
  [AC_SUBST([QT5_IM], ["N"])]
)
AS_IF(
  [test "x$QT5_IM" = "xY"],
  [
    if test ! -x "$QT5_MOC_PATH"; then
      mocs="$(pkg-config --variable=host_bins Qt5Core)/moc $qt5_moc_path_list"
      for moc in ${mocs}; do
        if test -x "$moc"; then
          AC_SUBST([QT5_MOC_PATH], ["$moc"])
          break
        fi
      done
    fi
    AS_IF(
      [test ! -x "$QT5_MOC_PATH"],
      [AC_MSG_ERROR(["Can not find moc for QT5"])]
    )
  ]
)
AS_IF(
  [test -z "$QT5_IM_DIR"],
  [AC_SUBST([QT5_IM_DIR], ["/qt5/plugins/platforminputcontexts"])]
)
AS_IF(
  [test -z "$QT5_IM_DIR_CUSTOM"],
  [AC_SUBST([QT5_IM_DIR], ["$(pkg-config --define-variable=prefix=\$\(DESTDIR\)/\$\(prefix\) --variable=libdir Qt5Core)$QT5_IM_DIR"])],
  [AC_SUBST([QT5_IM_DIR], ["\$(DESTDIR)$QT5_IM_DIR_CUSTOM"])]
)
AC_SUBST([QT5_MOC_PATH], ["$QT5_MOC_PATH"])
AC_SUBST([QT5_QMAKE_PATH], ["$(echo $QT5_MOC_PATH | sed 's/moc/qmake/')"])
# QT6
# TODO(xatier): Qt6 doesn't provide pkg-config, we need to hardcode things manually.
AC_SUBST([QT6_IM], ["Y"])
AC_ARG_ENABLE(
  [qt6_immodule],
  [AS_HELP_STRING([--enable-qt6-immodule], [Enable QT6 im-module, default is auto-detect])],
  [enable_qt6_immodule=$enableval],
  [enable_qt6_immodule=yes]
)
AC_ARG_WITH(
  [qt6_moc_path],
  [AS_HELP_STRING([--with-qt6-moc-path=PATH], [Specify the path of 'moc' command for QT6])],
  [QT6_MOC_PATH=$withval],
  [QT6_MOC_PATH=]
)
AC_ARG_WITH(
  [qt6_im_module_path],
  [AS_HELP_STRING([--with-qt6-im-module-path=PATH], [Specify the install path for QT6 im-module])],
  [QT6_IM_DIR_CUSTOM=$withval],
  [QT6_IM_DIR_CUSTOM=]
)
AS_IF(
  [test "x$enable_qt6_immodule" = "xno"],
  [AC_SUBST([QT6_IM], ["N"])]
)
AS_IF(
  [test "x$QT6_IM" = "xY"],
  [
    if test ! -x "$QT6_MOC_PATH"; then
      # TODO(xatier): hardcode moc path for now.
      mocs="/usr/lib/qt6/moc $qt6_moc_path_list"
      for moc in ${mocs}; do
        if test -x "$moc"; then
          AC_SUBST([QT6_MOC_PATH], ["$moc"])
          break
        fi
      done
    fi
    AS_IF(
      [test ! -x "$QT6_MOC_PATH"],
      [AC_MSG_ERROR(["Can not find moc for QT6"])]
    )
  ]
)
AS_IF(
  [test -z "$QT6_IM_DIR"],
  [AC_SUBST([QT6_IM_DIR], ["/qt6/plugins/platforminputcontexts"])]
)
AS_IF(
  [test -z "$QT6_IM_DIR_CUSTOM"],
  [AC_SUBST([QT6_IM_DIR], ["\$(DESTDIR)/\$(prefix)/lib$QT6_IM_DIR"])],
  [AC_SUBST([QT6_IM_DIR], ["\$(DESTDIR)$QT6_IM_DIR_CUSTOM"])]
)
AC_SUBST([QT6_MOC_PATH], ["$QT6_MOC_PATH"])
AC_SUBST([QT6_QMAKE_PATH], ["$(echo $QT6_MOC_PATH | sed 's/moc/qmake/')"])
# USE_ANTHY
PKG_CHECK_MODULES(
  [ANTHY],
  [anthy],
  [anthy_is_installed=yes],
  [anthy_is_installed=no]
)
AC_ARG_ENABLE(
  [anthy],
  [AS_HELP_STRING([--enable-anthy], [Enable Anthy, default is auto-detect])],
  [enable_anthy=$enableval],
  [
    AS_IF(
      [test "x$anthy_is_installed" = "yes"],
      [enable_anthy=yes],
      [enable_anthy=no]
    )
  ]
)
AS_IF(
  [test "x$anthy_is_installed" = "xyes" -a "x$enable_anthy" == "xyes"],
  [AC_SUBST([ANTHY_INC], ["-I$(pkg-config --variable=includedir anthy) $(pkg-config --libs anthy)"])]
)
AM_CONDITIONAL([USE_ANTHY], [test "x$anthy_is_installed" = "xyes" -a "x$enable_anthy" == "xyes"])
# USE_CHEWING
PKG_CHECK_MODULES(
  [CHEWING],
  [chewing >= 0.3.4],
  [chewing_is_installed=yes],
  [chewing_is_installed=no]
)
AC_ARG_ENABLE(
  [chewing],
  [AS_HELP_STRING([--enable-chewing], [Enable Chewing, default is auto-detect])],
  [enable_chewing=$enableval],
  [
    AS_IF(
      [test "x$chewing_is_installed" = "yes"],
      [enable_chewing=yes],
      [enable_chewing=no]
    )
  ]
)
AM_CONDITIONAL([USE_CHEWING], [test "x$chewing_is_installed" = "xyes" -a "x$enable_chewing" == "xyes"])
# USE_UNITY_TRAY
PKG_CHECK_MODULES(
  [APPINDICATOR],
  [$APPINDICATOR >= 0.4.0],
  [appindicator_is_installed=yes],
  [appindicator_is_installed=no]
)
AC_ARG_ENABLE(
  [appindicator],
  [AS_HELP_STRING([--enable-appindicator], [Enable Ubuntu Unity Tray, default is auto-detect])],
  [enable_appindicator=$enableval],
  [
    AS_IF(
      [test "x$appindicator_is_installed" = "yes"],
      [enable_appindicator=yes],
      [enable_appindicator=no]
    )
  ]
)
AS_IF(
  [test "x$appindicator_is_installed" = "xyes" -a "x$enable_appindicator" == "xyes"],
  [
    AC_SUBST([GTKINC], ["$GTKINC $APPINDICATORINC"])
    AC_SUBST([GTKLDFLAGS], ["$GTKLDFLAGS $APPINDICATORLDFLAGS"])
  ]
)
AM_CONDITIONAL([USE_UNITY_TRAY], [test "x$appindicator_is_installed" = "xyes" -a "x$enable_appindicator" == "xyes"])
LDFLAGS+=" $GTKLDFLAGS"
AS_IF(
  [test "x$FREEBSD" = "x0"],
  [AC_SUBST([LDFLAGS], ["$LDFLAGS -ldl"])]
)

LIBRARY_CHECK_MESSAGE(
  [test "x$gtk2_is_installed" = "xyes"],
  [test "x$with_gtk" = "x2.0"],
  [GTK 2]
)
LIBRARY_CHECK_MESSAGE(
  [test "x$gtk3_is_installed" = "xyes"],
  [test "x$with_gtk" = "x3.0"],
  [GTK 3]
)
LIBRARY_CHECK_MESSAGE(
  [test "x$anthy_is_installed" = "xyes"],
  [test "x$enable_anthy" = "xyes"],
  [Anthy]
)
LIBRARY_CHECK_MESSAGE(
  [test "x$chewing_is_installed" = "xyes"],
  [test "x$enable_chewing" = "xyes"],
  [Chewing]
)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([
  Makefile
  data/Makefile
  filter/Makefile
  icons/Makefile
  man/Makefile
  menu/Makefile
  po/Makefile
  scripts/Makefile
  src/Makefile
  src/gtk-im/Makefile
  src/gtk3-im/Makefile
  src/im-client/Makefile
  src/IMdkit/Makefile
  src/IMdkit/lib/Makefile
  src/modules/Makefile
  src/qt5-im/Makefile
  src/qt6-im/Makefile
])
AC_OUTPUT
