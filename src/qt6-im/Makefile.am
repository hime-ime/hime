if FREEBSD
  INCS=-I../im-client -I/usr/local/include
else
  INCS=-I../im-client -I/usr/include/X11
endif

# TODO(xatier): hardcode for now
# QTINC=$(shell pkg-config --variable=includedir Qt6Core)
# QTVER=$(shell pkg-config --modversion Qt6Core)
QTINC=/usr/include/qt6
QTVER=$(shell ls $(QTINC)/QtCore/ | grep ^6)

# TODO(xatier): hardcode for now
#INCS+=$(shell pkg-config Qt6Core Qt6Gui Qt6DBus --cflags)
INCS+=-I/usr/include/qt6/QtCore -I/usr/include/qt6 -DQT_GUI_LIB -I/usr/include/qt6/QtGui -DQT_DBUS_LIB -I/usr/include/qt6/QtDBus -DQT_CORE_LIB
INCS+=-I$(QTINC)/QtGui/$(QTVER)
INCS+=-I$(QTINC)/QtCore/$(QTVER)

CXXFLAGS+=$(OPTFLAGS) $(INCS) \
		-D_REENTRANT \
		-DQT6 \
		-DQT_SHARED \
		-DQT_IMMODULE \
		-DPIC -fPIC

OBJS= moc_hime-qt.o hime-qt.o hime-imcontext-qt.o moc_hime-imcontext-qt.o

#QTLIBS=$(shell pkg-config Qt6Core Qt6Gui Qt6DBus --libs)
QTLIBS=-lQt6Gui -lQt6DBus -lQt6Core
LDFLAGS+=$(LDFLAGS_ENV) -L../im-client -lhime-im-client $(QTLIBS)

.PHONY: all
all:    qt_info im-hime.so

%.E: %.cpp
	@echo "  $< -> $@"
	@$(CXX) -E $(CFLAGS) $(INCS) $< > $@

%.o: %.cpp
	@echo "  $< -> $@"
	@$(CXX) -c -o $@ $(CXXFLAGS) $<

moc_hime-qt.cpp:	hime-qt.h
	@echo "  $< -> $@"
	$(QT6_MOC_PATH) $(INCS) $< -o moc_hime-qt.cpp

moc_hime-imcontext-qt.cpp:	hime-imcontext-qt.h
	@echo "  $< -> $@"
	$(QT6_MOC_PATH) $(INCS) $< -o moc_hime-imcontext-qt.cpp

im-hime.so: $(OBJS)
	@echo "linking $@ ..."
	@export LD_RUN_PATH=$(himelibdir) ;\
		$(CXX) $(gcc_ld_run_path) -fno-exceptions -shared -o $@ $(OBJS) $(LDFLAGS)
	@rm -f core.*

.PHONY: qt_info
qt_info:
	@echo "Qt toolchain info:"
	@echo ""
	@echo "Qt version: $(QTVER)"
	@echo "include path: $(QTINC)"
	@echo "full include path: $(INCS)"
	@echo ""

.PHONY: install
install:
	install -d "$(QT6_IM_DIR)"; \
	install -m 755 im-hime.so "$(QT6_IM_DIR)"

.PHONY: uninstall
uninstall:
	rm -f $(QT6_IM_DIR)/im-hime.so
	rmdir --ignore-fail-on-non-empty $(QT6_IM_DIR)

.PHONY: clean
clean:
	@echo "clean up"
	@rm -f $(OBJS) im-hime.so *~ core.* *moc*.cpp *.E .depend
	@rm -f moc_hime-qt.cpp moc_hime-imcontext-qt.cpp

.depend:
	@echo "building $@ ..."
	@$(CXX) $(CXXFLAGS) -MM *.cpp > $@

