include ../../config.mak

OBJS = imhime.o gtkimcontexthime.o
GTK3IM=gtk-3.0/immodules
GTKINC=`pkg-config --cflags gtk+-3.0`
LDFLAGS+=`pkg-config --libs gtk+-3.0`
GTK3_LIBDIR=`pkg-config --variable=libdir gtk+-3.0`
GTK3_BINARY_VERSION=`pkg-config --variable=gtk_binary_version gtk+-3.0`
MODULEDIR=$(DESTDIR)/$(GTK3_LIBDIR)/gtk-3.0/$(GTK3_BINARY_VERSION)/immodules
IMMODULES=$(libdir)/$(GTK3IM)
IMMODULES_LOCAL=/usr/$(LIB)/$(GTK3IM)
IMMODULES_QUERY=$(shell whereis gtk-query-immodules-3.0 | awk -F ' = ' '{print $2}')
IMMODULES_CACHE=$(DESTDIR)/$(GTK3_LIBDIR)/gtk-3.0/$(GTK3_BINARY_VERSION)/immodules.cache

ifeq ($(MAC_OS),1)
EXTRA_INC=-I/sw/include
endif

.SUFFIXES:	.c .o .E
CFLAGS += $(OPTFLAGS) $(EXTRA_INC) $(GTKINC) -I../im-client -I.. -DUNIX=1 \
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
			if [ -f "$(IMMODULES_QUERY)" -a -x "$(IMMODULES_QUERY)" ]; then \
				GTK_IM_MODULE_FILE=$(IMMODULES_CACHE) $(IMMODULES_QUERY) --update-cache; \
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
