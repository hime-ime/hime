include ../../config.mak
QT=qt3
QTIM=$(QT)/plugins/inputmethods
IMMODULES=$(libdir)/$(QTIM)
INCS=-I../im-client -I/usr/include `pkg-config --cflags qt-mt` -I/usr/include/Xft2/X11/Xft  -I/usr/include/fontconfig -I/usr/include/freetype2
CXXFLAGS+=$(OPTFLAGS) $(INCS) -Wall -D_REENTRANT -DUNIX=1 -fPIC  -DQT_THREAD_SUPPORT -DQT_PLUGIN -DQT_SHARED -DQT_NO_DEBUG
OBJS=qhimeinputcontextplugin.o qhimeinputcontextplugin_my.o qhimeinputcontext.o qhimeinputcontext_x11.o
.SUFFIXES:	.c .cpp .a .so .E .h
LDFLAGS+=$(LDFLAGS_ENV) -L../im-client -lhime-im-client -Wl,-rpath,$(himelibdir) `pkg-config qt-mt --libs`

.PHONY: all
all:	im-hime.so

.cpp.E:
	@echo "  $< -> $@"
	$(CXX) -E $(CFLAGS) $(INCS) $< > $@

.cpp.o:
	@echo "  $< -> $@"
	$(CXX) -c -o $@ $(CXXFLAGS) $<

.h.o:
	@echo "  $< -> $@"
	@$(QT3_MOC_PATH) $< -o $<_moc.cpp
	$(CXX) -c -pipe $(CXXFLAGS) $<_moc.cpp -o $@

im-hime.so: $(OBJS)
	@echo "linking $@ ..."
	$(CXX) $(gcc_ld_run_path) -fno-exceptions -shared -o $@ $(OBJS) $(LDFLAGS)
	@rm -f core.*

.PHONY: install
install:
	if [ $(prefix) = /usr/local ]; then \
		install -d $(QT_IM_DIR); \
		install -m 755 im-hime.so $(QT_IM_DIR); \
	else \
		install -d $(IMMODULES); \
                install -m 755 im-hime.so $(IMMODULES); \
	fi

.PHONY: uninstall
uninstall:
	if [ $(prefix) = /usr/local ]; then \
		rm -f $(QT_IM_DIR)/im-hime.so; \
		rmdir --ignore-fail-on-non-empty $(QT_IM_DIR); \
	else \
		rm -f $(IMMODULES)/im-hime.so; \
		rmdir --ignore-fail-on-non-empty $(IMMODULES); \
	fi

.PHONY: clean
clean:
	@echo "clean up"
	@rm -f $(OBJS) im-hime.so *~ core.* *moc*.cpp *.E

