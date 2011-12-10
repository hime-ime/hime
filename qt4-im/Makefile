include ../config.mak
QT=qt4
QTIM=$(QT)/plugins/inputmethods
IMMODULES=$(libdir)/$(QTIM)

INCS=-I../im-client -I/usr/include/X11 `pkg-config QtCore QtGui --cflags`
CXXFLAGS=$(OPTFLAGS) $(INCS) -Wall -D_REENTRANT -DUNIX=1 -fPIC  -DQT4 -DQT_SHARED \
-DQT_IMMODULE -DPIC
OBJS= moc_hime-qt.o hime-qt.o im-hime-qt.o hime-imcontext-qt.o
.SUFFIXES:	.c .cpp .a .so .E .h
LDFLAGS=-L../im-client -lhime-im-client `pkg-config QtCore QtGui --libs`
all:    im-hime.so

.cpp.E:
	$(CXX) -E $(CFLAGS) $(INCS) $< > $@

moc_hime-qt.cpp:	hime-qt.h
	/usr/$(LIB)/$(QT)/bin/moc $< -o moc_hime-qt.cpp

im-hime.so: $(OBJS)
	export LD_RUN_PATH=$(himelibdir) ;\
	$(CXX) $(gcc_ld_run_path) -fno-exceptions -shared -o $@ $(OBJS) $(LDFLAGS)
	rm -f core.*
install:
	if [ $(prefix) = /usr/local ]; then \
		install -d $(DESTDIR)/$(QT4_IM_DIR); \
		install -m 755 im-hime.so $(DESTDIR)/$(QT4_IM_DIR); \
	else \
		install -d $(IMMODULES); \
                install -m 755 im-hime.so $(IMMODULES); \
	fi
clean:
	rm -f $(OBJS) im-hime.so *~ core.* *moc*.cpp *.E .depend

.depend:
	$(CXX) $(CXXFLAGS) -MM *.cpp > $@

include .depend
