#!/bin/sh
[ ! -e debian/hime/`pkg-config --variable=libdir QtCore`/qt4/plugins/inputmethods/im-hime.so ] && sed -i 's/, hime-qt4-immodule//' debian/hime/DEBIAN/control
[ ! -e debian/hime/usr/lib/qt3/plugins/inputmethods/im-hime.so ] && sed -i 's/, hime-qt3-immodule//' debian/hime/DEBIAN/control
[ ! -e debian/hime/`pkg-config --variable=libdir gtk+-3.0`/gtk-3.0/`pkg-config --variable=gtk_binary_version gtk+-3.0`/immodules/im-hime.so ] && sed -i 's/, hime-gtk3-immodule//' debian/hime/DEBIAN/control
[ ! -e debian/hime/usr/lib/hime/chewing-module.so ] && sed -i 's/, libchewing3//' debian/hime/DEBIAN/control

if [ ! -e debian/hime/usr/lib/hime/anthy-module.so ]; then
  grep -v 'Suggests: kasumi' debian/hime/DEBIAN/control | sed 's/, anthy//' > debian/hime/DEBIAN/control.in
  mv debian/hime/DEBIAN/control.in debian/hime/DEBIAN/control
fi

if [ -x /usr/lib/libgtk2.0-0/gtk-query-immodules-2.0 ]; then
  /usr/lib/libgtk2.0-0/gtk-query-immodules-2.0 debian/hime/usr/lib/gtk-2.0/2.10.0/immodules/im-hime.so | grep hime | tail -n 1 >> debian/hime/usr/lib/gtk-2.0/2.10.0/immodule-files.d/hime.immodules
else
  rm -fr debian/hime/usr/lib/gtk-2.0/2.10.0/immodule-files.d
fi

true
