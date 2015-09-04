include ../../config.mak

SOFILE=libhime-im-client.so
SOFILEVER=libhime-im-client.so.1.2.4

.SUFFIXES:      .c .o .E .cpp
WALL=-Wall
CFLAGS+= -DUNIX=1 $(WALL) $(OPTFLAGS) -I. -I.. -I../IMdkit/include $(GTKINC) \
        -DCLIENT_LIB=1 -DHIME_BIN_DIR=\"$(HIME_BIN_DIR)\" -DUNIX=1 \
        -DHIME_TABLE_DIR=\"$(HIME_TABLE_DIR)\" \
        -DFREEBSD=$(FREEBSD) -fPIC
OBJS = hime-im-client.o hime-send.o im-addr.o hime-conf.o util.o hime-crypt-fpic.o

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
	$(CC) $(SO_FLAGS) -Wl,-soname,libhime-im-client.so.1 $(OBJS) -lX11 -lglib-2.0 -o $@ -L/usr/X11R6/lib
else
	$(CC) $(SO_FLAGS) -Wl,-soname,libhime-im-client.so.1 $(OBJS) -lX11 -lglib-2.0 -o $@ -L/usr/local/lib
endif
	ln -sf $(SOFILEVER) $(SOFILE)
	ln -sf $(SOFILEVER) $(SOFILE).1
	if [ $(GTK_IM) = 'Y' ]; then $(MAKE) -C ../gtk-im; fi

LOCALLIB=/usr/local/$(LIB)

install:
	install -d "$(himelibdir)"
	install -m 755 $(SOFILEVER) "$(himelibdir)"
	cd "$(himelibdir)"; rm -f $(SOFILE); ln -s $(SOFILEVER) $(SOFILE); \
	ln -sf $(SOFILEVER) $(SOFILE).1

hime-conf.o: ../hime-conf.c
	@echo "  $^ -> $@"
	$(CCX) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

util.o: ../util.c
	@echo "  $^ -> $@"
	$(CCX) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

im-addr.o: ../im-addr.c
	@echo "  $^ -> $@"
	$(CCX) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

hime-crypt-fpic.o: ../hime-crypt.c
	@echo "  $^ -> $@"
	$(CCX) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

clean:
	@echo "clean up"
	rm -f *.o *.so *.so.* *~ *.E *.db tags core.* .depend

.depend:
	@echo "building $@ ..."
	$(CCX) $(CPPFLAGS) $(CFLAGS) -MM *.c > $@

include .depend
