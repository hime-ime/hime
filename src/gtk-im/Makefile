include ../../config.mak

OBJS = imhime.o gtkimcontexthime.o
GTK2IM=gtk-2.0/immodules
GTKINC=`pkg-config --cflags gtk+-2.0`
LDFLAGS=`pkg-config --libs gtk+-2.0`
IMMODULES=$(libdir)/$(GTK2IM)
IMMODULES_LOCAL=/usr/$(LIB)/$(GTK2IM)


ifeq ($(MAC_OS),1)
EXTRA_INC=-I/sw/include
endif

.SUFFIXES:	.c .o .E
CFLAGS = $(OPTFLAGS) $(EXTRA_INC) $(GTKINC) -I../im-client -I.. -DUNIX=1 \
-I../IMdkit/include -DCLIENT_LIB=1 -DMAC_OS=$(MAC_OS) -DFREEBSD=$(FREEBSD)

.c.o:
	@echo "  $< -> $@"
	@$(CC) -c -fPIC $(CFLAGS) -o $@ $<

.c.E:
	@echo "  $< -> $@"
	@$(CC) -E $(CFLAGS) -o $@ $<

all:	im-hime.so im-hime-cur.so

im-hime.so:     $(OBJS) ../im-client/libhime-im-client.so
	@echo "linking $@ ..."
	@$(CC) $(gcc_ld_run_path) $(SO_FLAGS) $(OBJS) -L../im-client -lhime-im-client -o $@ $(LDFLAGS)
	@rm -f core.*

im-hime-cur.so:     $(OBJS) ../im-client/libhime-im-client.so
	@echo "linking $@ ..."
	@$(CC) $(SO_FLAGS) $(OBJS) -L../im-client -lhime-im-client -o $@ $(LDFLAGS)
	@rm -f core.*

clean:
	@echo "clean up"
	@rm -f *.o *.so *~ core.* .depend *.E

install:
	if [ $(prefix) = /usr/local ]; then \
		install -d $(IMMODULES_LOCAL); \
		install -m 755 im-hime.so $(IMMODULES_LOCAL); \
		cd /etc/gtk-2.0; gtk-query-immodules-2.0 > gtk.immodules.lib; \
	else \
		install -d $(IMMODULES); \
		install -m 755 im-hime.so $(IMMODULES); \
	fi

.depend:
	@echo "building $@ ..."
	@$(CC) $(CFLAGS) -MM *.c > $@

include .depend
