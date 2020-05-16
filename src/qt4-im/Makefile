include ../../config.mak

ifeq ($(FREEBSD), 0)
  INCS=-I../im-client -I/usr/include/X11
else
  INCS=-I../im-client -I/usr/local/include
endif
INCS+=$(shell pkg-config QtCore QtGui --cflags)

CXXFLAGS+=$(OPTFLAGS) $(INCS) \
		-D_REENTRANT \
		-DQT4 \
		-DQT_SHARED \
		-DQT_IMMODULE \
		-DPIC -fPIC

OBJS= moc_hime-qt.o hime-qt.o im-hime-qt.o hime-imcontext-qt.o moc_hime-imcontext-qt.o

QTLIBS=$(shell pkg-config QtCore QtGui --libs)
LDFLAGS+=$(LDFLAGS_ENV) -L../im-client -lhime-im-client $(QTLIBS)

.SUFFIXES:	.c .cpp .a .so .E .h

.PHONY: all
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

moc_hime-imcontext-qt.cpp:	hime-imcontext-qt.h
	@echo "  $< -> $@"
	@$(QT5_MOC_PATH) $(INCS) $< -o moc_hime-imcontext-qt.cpp

im-hime.so: $(OBJS)
	@echo "linking $@ ..."
	@export LD_RUN_PATH=$(himelibdir) ;\
		$(CXX) $(gcc_ld_run_path) -fno-exceptions -shared -o $@ $(OBJS) $(LDFLAGS)
	@rm -f core.*

.PHONY: install
install:
	install -d "$(QT4_IM_DIR)"; \
	install -m 755 im-hime.so "$(QT4_IM_DIR)"

.PHONY: uninstall
uninstall:
	rm -f $(QT4_IM_DIR)/im-hime.so
	rmdir --ignore-fail-on-non-empty $(QT4_IM_DIR)

.PHONY: clean
clean:
	@echo "clean up"
	@rm -f $(OBJS) im-hime.so *~ core.* *moc*.cpp *.E .depend
	@rm -f moc_hime-qt.cpp moc_hime-imcontext-qt.cpp

.depend:
	@echo "building $@ ..."
	$(CXX) $(CXXFLAGS) -MM *.cpp > $@

include .depend
