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
IMMODULES_QUERY=$(GTK_LIBDIR)/libgtk2.0-0/gtk-query-immodules-2.0


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

.PHONY: all
all:	im-hime.so

im-hime.so:     $(OBJS) ../im-client/libhime-im-client.so
	@echo "linking $@ ..."
	$(CC) $(gcc_ld_run_path) $(SO_FLAGS) $(OBJS) -L../im-client -lhime-im-client -o $@ $(LDFLAGS)
	@rm -f core.*

.PHONY: clean
clean:
	@echo "clean up"
	@rm -f *.o *.so *~ core.* .depend *.E

.PHONY: install
install:
	if [ "$(prefix)" = "/usr/local" -o "$(prefix)" = "/usr" ]; then \
		install -d "$(MODULEDIR)"; \
		install -m 755 im-hime.so "$(MODULEDIR)"; \
		if [ "$(prefix)" = "/usr/local" ]; then \
			if [ -d "$(DESTDIR)/etc/gtk-2.0" ]; then \
				cd "$(DESTDIR)/etc/gtk-2.0"; $(IMMODULES_QUERY) > gtk.immodules; \
			fi \
		fi \
	else \
		install -d "$(IMMODULES)"; \
		install -m 755 im-hime.so "$(IMMODULES)"; \
	fi

.PHONY: uninstall
uninstall:
	if [ "$(prefix)" = "/usr/local" -o "$(prefix)" = "/usr" ]; then \
		rm -f $(MODULEDIR)/im-hime.so; \
		rmdir --ignore-fail-on-non-empty $(MODULEDIR); \
	else \
		rm -f $(IMMODULES)/im-hime.so; \
		rmdir --ignore-fail-on-non-empty $(IMMODULES); \
	fi

.depend:
	@echo "building $@ ..."
	$(CC) $(CPPFLAGS) $(CFLAGS) -MM *.c > $@

include .depend
