include ../../config.mak

ifeq ($(FREEBSD), 0)
  INCS=-I../im-client -I/usr/include/X11 `pkg-config QtCore QtGui --cflags`
else
  INCS=-I../im-client -I/usr/local/include `pkg-config QtCore QtGui --cflags`
endif
CXXFLAGS+=$(OPTFLAGS) $(INCS) -Wall -D_REENTRANT -DUNIX=1 -fPIC  -DQT4 -DQT_SHARED \
-DQT_IMMODULE -DPIC
OBJS= moc_hime-qt.o hime-qt.o im-hime-qt.o hime-imcontext-qt.o
.SUFFIXES:	.c .cpp .a .so .E .h
LDFLAGS+=$(LDFLAGS_ENV) -L../im-client -lhime-im-client `pkg-config QtCore QtGui --libs`
all:    im-hime.so

.cpp.E:
	@echo "  $< -> $@"
	$(CXX) -E $(CFLAGS) $(INCS) $< > $@

.cpp.o:
	@echo "  $< -> $@"
	$(CXX) -c -o $@ $(CXXFLAGS) $<

moc_hime-qt.cpp:	hime-qt.h
	@echo "  $< -> $@"
	$(QT4_MOC_PATH) $< -o moc_hime-qt.cpp

im-hime.so: $(OBJS)
	@echo "linking $@ ..."
	@export LD_RUN_PATH=$(himelibdir) ;\
		$(CXX) $(gcc_ld_run_path) -fno-exceptions -shared -o $@ $(OBJS) $(LDFLAGS)
	@rm -f core.*

install:
	install -d "$(QT4_IM_DIR)"; \
	install -m 755 im-hime.so "$(QT4_IM_DIR)"

clean:
	@echo "clean up"
	@rm -f $(OBJS) im-hime.so *~ core.* *moc*.cpp *.E .depend

.depend:
	@echo "building $@ ..."
	$(CXX) $(CXXFLAGS) -MM *.cpp > $@

include .depend
