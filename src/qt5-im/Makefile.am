if FREEBSD
  INCS=-I../im-client -I/usr/local/include
else
  INCS=-I../im-client -I/usr/include/X11
endif

QTINC=$(shell pkg-config --variable=includedir Qt5Core)
QTVER=$(shell pkg-config --modversion Qt5Core)

INCS+=$(shell pkg-config Qt5Core Qt5Gui Qt5DBus --cflags)
INCS+=-I$(QTINC)/QtGui/$(QTVER)
INCS+=-I$(QTINC)/QtCore/$(QTVER)

CXXFLAGS+=$(OPTFLAGS) $(INCS) \
		-D_REENTRANT \
		-DQT5 \
		-DQT_SHARED \
		-DQT_IMMODULE \
		-DPIC -fPIC

OBJS= moc_hime-qt.o hime-qt.o hime-imcontext-qt.o moc_hime-imcontext-qt.o

QTLIBS=$(shell pkg-config Qt5Core Qt5Gui Qt5DBus --libs)
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
	$(QT5_MOC_PATH) $(INCS) $< -o moc_hime-qt.cpp

moc_hime-imcontext-qt.cpp:	hime-imcontext-qt.h
	@echo "  $< -> $@"
	$(QT5_MOC_PATH) $(INCS) $< -o moc_hime-imcontext-qt.cpp

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
	install -d "$(QT5_IM_DIR)"; \
	install -m 755 im-hime.so "$(QT5_IM_DIR)"

.PHONY: uninstall
uninstall:
	rm -f $(QT5_IM_DIR)/im-hime.so
	rmdir --ignore-fail-on-non-empty $(QT5_IM_DIR)

.PHONY: clean
clean:
	@echo "clean up"
	@rm -f $(OBJS) im-hime.so *~ core.* *moc*.cpp *.E .depend
	@rm -f moc_hime-qt.cpp moc_hime-imcontext-qt.cpp

.depend:
	@echo "building $@ ..."
	@$(CXX) $(CXXFLAGS) -MM *.cpp > $@
