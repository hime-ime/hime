include ../../config.mak

ifeq ($(FREEBSD), 0)
  INCS=-I../im-client -I/usr/include/X11
else
  INCS=-I../im-client -I/usr/local/include
endif
INCS+=`pkg-config Qt5Core Qt5Gui Qt5PlatformSupport Qt5DBus --cflags`
INCS+=-I `pkg-config --variable=includedir Qt5Gui`/QtGui/`pkg-config --modversion Qt5Gui`/QtGui

CXXFLAGS=$(OPTFLAGS) $(INCS) -Wall -D_REENTRANT -DUNIX=1 -fPIC  -DQT5 -DQT_SHARED \
-DQT_IMMODULE -DPIC
OBJS= moc_hime-qt.o hime-qt.o hime-imcontext-qt.o moc_hime-imcontext-qt.o
.SUFFIXES:	.c .cpp .a .so .E .h
LDFLAGS=$(LDFLAGS_ENV) -L../im-client -lhime-im-client `pkg-config Qt5Core Qt5Gui Qt5PlatformSupport Qt5DBus --libs`


all:    im-hime.so

.cpp.E:
	@echo "  $< -> $@"
	@$(CXX) -E $(CFLAGS) $(INCS) $< > $@

.cpp.o:
	@echo "  $< -> $@"
	@$(CXX) -c -o $@ $(CXXFLAGS) $<

moc_hime-qt.cpp:	hime-qt.h
	@echo "  $< -> $@"
	@$(QT5_MOC_PATH) $< -o moc_hime-qt.cpp

moc_hime-imcontext-qt.cpp:	hime-imcontext-qt.h
	@echo "  $< -> $@"
	@$(QT5_MOC_PATH) $< -o moc_hime-imcontext-qt.cpp

im-hime.so: $(OBJS)
	@echo "linking $@ ..."
	@export LD_RUN_PATH=$(himelibdir) ;\
		$(CXX) $(gcc_ld_run_path) -fno-exceptions -shared -o $@ $(OBJS) $(LDFLAGS)
	@rm -f core.*

install:
	install -d $(QT5_IM_DIR); \
	install -m 755 im-hime.so $(QT5_IM_DIR)

clean:
	@echo "clean up"
	@rm -f $(OBJS) im-hime.so *~ core.* *moc*.cpp *.E .depend

.depend:
	@echo "building $@ ..."
	@$(CXX) $(CXXFLAGS) -MM *.cpp > $@

include .depend
