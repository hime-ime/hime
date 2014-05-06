include ../../config.mak

OBJS = imhime.o gtkimcontexthime.o
GTK2IM=gtk-2.0/immodules
GTKINC=`pkg-config --cflags gtk+-2.0`
LDFLAGS=`pkg-config --libs gtk+-2.0`
GTK_LIBDIR=`pkg-config --variable=libdir gtk+-2.0`
GTK_BINARY_VERSION=`pkg-config --variable=gtk_binary_version gtk+-2.0`
MODULEDIR=$(DESTDIR)/$(GTK_LIBDIR)/gtk-2.0/$(GTK_BINARY_VERSION)/immodules
IMMODULES=$(libdir)/$(GTK2IM)
IMMODULES_LOCAL=/usr/$(LIB)/$(GTK2IM)
IMMODULES_QUERY=`whereis gtk-query-immodules-2.0 | cut -d ' ' -f 2`


ifeq ($(MAC_OS),1)
EXTRA_INC=-I/sw/include
endif

.SUFFIXES:	.c .o .E
CFLAGS = $(OPTFLAGS) $(EXTRA_INC) $(GTKINC) -I../im-client -I.. -DUNIX=1 \
-I../IMdkit/include -DCLIENT_LIB=1 -DMAC_OS=$(MAC_OS) -DFREEBSD=$(FREEBSD)

.c.o:
	@echo "  $< -> $@"
	$(CC) -c -fPIC $(CPPFLAGS) $(CFLAGS) -o $@ $<

.c.E:
	@echo "  $< -> $@"
	$(CC) -E $(CPPFLAGS) $(CFLAGS) -o $@ $<

all:	im-hime.so

im-hime.so:     $(OBJS) ../im-client/libhime-im-client.so
	@echo "linking $@ ..."
	$(CC) $(gcc_ld_run_path) $(SO_FLAGS) $(OBJS) -L../im-client -lhime-im-client -o $@ $(LDFLAGS)
	@rm -f core.*

clean:
	@echo "clean up"
	@rm -f *.o *.so *~ core.* .depend *.E

install:
	if [ "$(prefix)" = "/usr/local" -o "$(prefix)" = "/usr" ]; then \
		install -d $(MODULEDIR); \
		install -m 755 im-hime.so $(MODULEDIR); \
		if [ "$(prefix)" = "/usr/local" ]; then \
			if [ -d $(DESTDIR)/etc/gtk-2.0 ]; then \
				cd $(DESTDIR)/etc/gtk-2.0; $(IMMODULES_QUERY) > gtk.immodules; \
			fi \
		fi \
	else \
		install -d $(IMMODULES); \
		install -m 755 im-hime.so $(IMMODULES); \
	fi

.depend:
	@echo "building $@ ..."
	$(CC) $(CPPFLAGS) $(CFLAGS) -MM *.c > $@

include .depend
