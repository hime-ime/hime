OBJS = imhime.o gtkimcontexthime.o
GTK2IM=gtk-2.0/immodules
GTKINC=$(shell pkg-config --cflags gtk+-2.0)
LDFLAGS=$(shell pkg-config --libs gtk+-2.0)
GTK_LIBDIR=$(shell pkg-config --variable=libdir gtk+-2.0)
GTK_BINARY_VERSION=$(shell pkg-config --variable=gtk_binary_version gtk+-2.0)
MODULEDIR=$(DESTDIR)/$(GTK_LIBDIR)/gtk-2.0/$(GTK_BINARY_VERSION)/immodules
IMMODULES=$(libdir)/$(GTK2IM)
IMMODULES_LOCAL=/usr/$(LIB)/$(GTK2IM)
IMMODULES_QUERY=$(GTK_LIBDIR)/libgtk2.0-0/gtk-query-immodules-2.0

CFLAGS += $(OPTFLAGS) $(GTKINC) \
		-DGDK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED \
		-DGTK_DISABLE_SINGLE_INCLUDES -DGTK_DISABLE_DEPRECATED \
		-DG_DISABLE_SINGLE_INCLUDES \
		-DCLIENT_LIB=1 \
		-DFREEBSD=$(FREEBSD) \
		-I../im-client -I.. \
		-I../IMdkit/include

%.o: %.c
	@echo "  $< -> $@"
	$(CC) -c -fPIC $(CPPFLAGS) $(CFLAGS) -o $@ $<

%.E: %.c
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

