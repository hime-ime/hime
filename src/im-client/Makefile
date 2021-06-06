include ../../config.mak

SOFILE=libhime-im-client.so
SOFILEVER=libhime-im-client.so.1.2.4

.SUFFIXES:      .c .o .E .cpp
CFLAGS+= $(OPTFLAGS) $(GTKINC) \
		-DGDK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED \
		-DGTK_DISABLE_SINGLE_INCLUDES -DGTK_DISABLE_DEPRECATED \
		-DG_DISABLE_SINGLE_INCLUDES \
		-DCLIENT_LIB=1 \
		-DHIME_BIN_DIR=\"$(HIME_BIN_DIR)\" \
		-DHIME_TABLE_DIR=\"$(HIME_TABLE_DIR)\" \
		-DFREEBSD=$(FREEBSD) \
		-I. -I.. -I../IMdkit/include \
		-fPIC

DEPS = hime-common.o hime-conf.o hime-crypt.o hime-settings.o \
		locale.o im-addr.o pho2pinyin.o pinyin.o util.o
OBJS = hime-im-client.o hime-send.o $(DEPS)

.cpp.E:
	@echo "  $< -> $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) -E -o $@ $<

.c.o:
	@echo "  $< -> $@"
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $<

.cpp.o:
	@echo "  $< -> $@"
	$(CC) -x c -c $(CPPFLAGS) $(CFLAGS) $<

$(SOFILEVER):   $(OBJS)
	@echo "linking $@ ..."

# TODO: Drop g_get_tmp_dir() and -lglib-2.0
ifeq ($(FREEBSD), 0)
	$(CC) $(SO_FLAGS) -Wl,-soname,libhime-im-client.so.1 $(OBJS) -lX11 -lglib-2.0 -lgtk-x11-2.0 -o $@ -L/usr/X11R6/lib
else
	$(CC) $(SO_FLAGS) -Wl,-soname,libhime-im-client.so.1 $(OBJS) -lX11 -lglib-2.0 -lgtk-x11-2.0 -o $@ -L/usr/local/lib
endif
	ln -sf $(SOFILEVER) $(SOFILE)
	ln -sf $(SOFILEVER) $(SOFILE).1
	if [ $(GTK_IM) = 'Y' ]; then $(MAKE) -C ../gtk-im; fi
	if [ $(GTK3_IM) = 'Y' ]; then $(MAKE) -C ../gtk3-im; fi
	if [ $(QT5_IM) = 'Y' ]; then $(MAKE) -C ../qt5-im; fi

LOCALLIB=/usr/local/$(LIB)

.PHONY: install
install:
	install -d "$(himelibdir)"
	install -m 755 $(SOFILEVER) "$(himelibdir)"
	cd "$(himelibdir)"; rm -f $(SOFILE); ln -s $(SOFILEVER) $(SOFILE); \
	ln -sf $(SOFILEVER) $(SOFILE).1

.PHONY: uninstall
uninstall:
	cd $(himelibdir) && rm -f $(SOFILEVER) $(SOFILE) $(SOFILE).1
	rmdir --ignore-fail-on-non-empty $(himelibdir)

# rules to rebuild $(DEPS), rebuild with -fPIC
hime-common.o: ../hime-common.c
	@echo "  $^ -> $@"
	$(CCX) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

hime-conf.o: ../hime-conf.c
	@echo "  $^ -> $@"
	$(CCX) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

hime-crypt.o: ../hime-crypt.c
	@echo "  $^ -> $@"
	$(CCX) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

hime-settings.o: ../hime-settings.c
	@echo "  $^ -> $@"
	$(CCX) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

locale.o: ../locale.c
	@echo "  $^ -> $@"
	$(CCX) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

im-addr.o: ../im-addr.c
	@echo "  $^ -> $@"
	$(CCX) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

pho2pinyin.o: ../pho2pinyin.c
	@echo "  $^ -> $@"
	$(CCX) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

pinyin.o: ../pinyin.c
	@echo "  $^ -> $@"
	$(CCX) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

util.o: ../util.c
	@echo "  $^ -> $@"
	$(CCX) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<


.PHONY: clean
clean:
	@echo "clean up"
	rm -f *.o *.so *.so.* *~ *.E *.db tags core.* .depend

.depend:
	@echo "building $@ ..."
	$(CCX) $(CPPFLAGS) $(CFLAGS) -MM *.c > $@

include .depend
