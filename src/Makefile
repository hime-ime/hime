include ../config.mak

CFLAGS= $(OPTFLAGS) $(GTKINC) \
		-DGDK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED \
		-DGTK_DISABLE_SINGLE_INCLUDES -DGTK_DISABLE_DEPRECATED \
		-DG_DISABLE_SINGLE_INCLUDES \
		-DGSEAL_ENABLE \
		-DHIME_BIN_DIR=\"$(HIME_BIN_DIR)\" \
		-DHIME_DEFAULT_ICON_DIR=\"$(HIME_DEFAULT_ICON_DIR)\" \
		-DHIME_OGG_DIR=\"$(HIME_OGG_DIR)\" \
		-DHIME_SCRIPT_DIR=\"$(HIME_SCRIPT_DIR)\" \
		-DHIME_TABLE_DIR=\"$(HIME_TABLE_DIR)\" \
		-DHIME_VERSION=\"$(HIME_VERSION)\" \
		-DDOC_DIR=\"$(DOC_DIR)\" \
		-DSYS_ICON_DIR=\"$(SYS_ICON_DIR)\" \
		-DFREEBSD=$(FREEBSD) \
		-I./IMdkit/include -I./im-client

OBJ_IMSRV = hime-crypt.o im-addr.o im-dispatch.o im-srv.o

hime_pho_o  = pho-dbg.o pho-sym.o pho-util.o pho.o \
			table-update.o win-pho.o
hime_gtab_o = gtab-buf.o gtab-list.o gtab-util.o gtab.o win-gtab.o


OBJS      = $(hime_pho_o) $(hime_gtab_o) hime.o \
			about.o cache.o eve.o fullchar.o \
			gtab-init.o gtab-tsin-fname.o gtab-use-count.o gtk_bug_fix.o \
			hime-common.o hime-conf.o hime-exec-script.o hime-icon.o \
			hime-module-cb.o hime-module.o hime-settings.o \
			lang.o locale.o \
			pho-kbm-name.o pho-play.o pho2pinyin.o \
			phrase-save-menu.o phrase.o \
			pinyin.o \
			statistic.o t2s-lookup.o tsin-scan.o \
			unix-exec.o util.o \
			win-inmd-switch.o win-kbm.o win-message.o win-pho-near.o \
			win-save-phrase.o win-sym.o

OBJS_TSLEARN = hime-tslearn.o \
			fullchar.o \
			gtab-init.o gtab-list.o gtab-tsin-fname.o gtab-util.o \
			hime-common.o hime-conf.o hime-icon.o hime-settings.o \
			lang.o locale.o \
			pho-dbg.o pho-sym.o pho-util.o pho2pinyin.o \
			pinyin.o \
			table-update.o tsin-util.o unix-exec.o util.o

OBJS_TS_EDIT = hime-ts-edit.o \
			fullchar.o \
			gtab-init.o gtab-list.o gtab-tsin-fname.o gtab-util.o \
			hime-common.o hime-conf.o hime-icon.o hime-settings.o \
			lang.o locale.o \
			pho-dbg.o pho-sym.o pho-util.o pho2pinyin.o \
			pinyin.o \
			table-update.o tsin-util.o unix-exec.o util.o

OBJS_JUYIN_LEARN = hime-juyin-learn.o \
			hime-common.o hime-conf.o hime-icon.o hime-settings.o \
			locale.o \
			pho-dbg.o pho-sym.o pho-util.o pho2pinyin.o \
			pinyin.o \
			table-update.o util.o

OBJS_hime-sim2trad = hime-sim2trad.o \
			hime-common.o hime-conf.o hime-icon.o hime-settings.o \
			locale.o \
			pho2pinyin.o pinyin.o \
			t2s-lookup.o util.o

OBJS_hime-phod2a = hime-phod2a.o \
			hime-common.o hime-conf.o hime-settings.o \
			locale.o \
			pho-dbg.o pho-sym.o pho-util.o pho2pinyin.o \
			pinyin.o \
			table-update.o util.o

OBJS_hime-tsa2d32 = hime-tsa2d32.o \
			hime-common.o hime-conf.o hime-settings.o \
			locale.o \
			pho-dbg.o pho-lookup.o pho-sym.o pho2pinyin.o \
			pinyin.o \
			util.o

OBJS_hime-phoa2d = hime-phoa2d.o \
			hime-common.o hime-conf.o hime-settings.o \
			locale.o \
			pho-dbg.o pho-lookup.o pho-sym.o pho2pinyin.o \
			pinyin.o \
			util.o

OBJS_kbmcv = kbmcv.o \
			locale.o pho-sym.o util.o

OBJS_hime-tsd2a32 = hime-tsd2a32.o \
			gtab-dbg.o \
			hime-common.o hime-conf.o hime-settings.o \
			locale.o \
			pho-dbg.o pho-sym.o pho2pinyin.o \
			pinyin.o \
			util.o

OBJS_cin2gtab     = hime-cin2gtab.o gtab-util.o locale.o util.o
OBJS_gtab2cin     = hime-gtab2cin.o gtab-util.o locale.o util.o
OBJS_gtab_merge   = hime-gtab-merge.o gtab-util.o locale.o util.o
OBJS_hime_setup   = hime-setup.o \
					about.o \
					gtab-list.o \
					hime-common.o hime-conf.o hime-exec-script.o hime-icon.o \
					hime-module-cb.o hime-settings.o hime-setup-appearance.o \
					hime-setup-gtab.o hime-setup-list.o hime-setup-pho.o \
					lang.o locale.o \
					pho-kbm-name.o pho2pinyin.o pinyin.o \
					util.o

OBJS_hime_gb_toggle  = hime-gb-toggle.o \
					hime-common.o hime-conf.o hime-settings.o \
					locale.o \
					pho2pinyin.o pinyin.o\
					util.o

OBJS_hime_message    = hime-message.o \
					hime-common.o hime-conf.o hime-settings.o \
					locale.o \
					pho2pinyin.o pinyin.o \
					util.o

OBJS_pin_juyin       = pin-juyin.o locale.o pho-lookup.o pho-sym.o util.o

OBJS_tsin2gtab_phrase = hime-tsin2gtab-phrase.o \
						gtab-dbg.o \
						hime-common.o hime-conf.o hime-settings.o \
						lang.o locale.o \
						pho-dbg.o pho-sym.o pho2pinyin.o \
						pinyin.o \
						util.o


ifeq ($(USE_XIM),Y)
IMdkitLIB   = IMdkit/lib/libXimd.a
CFLAGS     += -DUSE_XIM=1
OBJS       += IC.o
endif

ifeq ($(USE_SYSTEM_TRAY),Y)
CFLAGS     += -DTRAY_ENABLED=1
OBJS       += tray.o tray-double.o
ifeq ($(GTK),gtk+-2.0)
OBJS       += gdkpixbuf-drawable.o
endif
ifeq ($(USE_UNITY_TRAY),Y)
CFLAGS     += -DTRAY_UNITY=1
OBJS       += tray-appindicator.o
endif
endif

ifeq ($(USE_I18N),Y)
CFLAGS     += -DHIME_I18N_MESSAGE=1
endif

ifeq ($(USE_TSIN),Y)
hime_tsin_o = tsin.o tsin-util.o win0.o win1.o tsin-parse.o
CFLAGS     += -DUSE_TSIN=1
OBJS       += $(hime_tsin_o)
endif

# Temporary workaround to turn off Tsin, will be fixed later
ifeq ($(USE_TSIN),N)
OBJS       += tsin.o tsin-util.o win0.o win1.o tsin-parse.o
endif

ifeq ($(GIT_HAVE),Y)
CFLAGS     += -DGIT_HASH=\"$(GIT_HASH)\" -DGIT_HAVE=1
endif

PROGS     = hime \
	hime-tsd2a32 hime-tsa2d32 hime-phoa2d hime-phod2a \
	hime-tslearn hime-setup hime-cin2gtab hime-gtab2cin \
	hime-juyin-learn hime-sim2trad hime-gb-toggle hime-message \
	hime-gtab-merge hime-tsin2gtab-phrase hime-ts-edit

PROGS_SYM = hime-trad2sim hime-exit hime-sim hime-trad hime-kbm-toggle

PROGS_CV  = kbmcv pin-juyin

.SUFFIXES:  .c .o .E .pico .cpp

.cpp.o:
	@echo "  $< -> $@"
	$(CCX) $(CPPFLAGS) $(CFLAGS) -c $<
.c.o:
	@echo "  $< -> $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<
.c.pico:
	@echo "  $< -> $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -fpic -o $@ $<
.cpp.pico:
	@echo "  $< -> $@"
	$(CCX) $(CPPFLAGS) $(CFLAGS) -c -fpic -o $@ $<
.c.E:
	@echo "  $< -> $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) -E -o $@ $<
.cpp.E:
	@echo "  $< -> $@"
	$(CCX) $(CPPFLAGS) $(CFLAGS) -E -o $@ $<

.PHONY: all
all: $(PROGS) $(PROGS_SYM) $(DATA) $(PROGS_CV)
	$(MAKE) -C modules
	if [ $(GTK_IM) = 'Y' ]; then $(MAKE) -C gtk-im; fi
	if [ $(GTK3_IM) = 'Y' ]; then $(MAKE) -C gtk3-im; fi
	if [ $(QT5_IM) = 'Y' ]; then $(MAKE) -C qt5-im; fi

$(IMdkitLIB):
	@echo "building $@ ..."
	$(MAKE) -C IMdkit/lib

im-client/libhime-im-client.so:
	@echo "building $@ ..."
	$(MAKE) -C im-client

hime: $(OBJS) $(IMdkitLIB) $(OBJ_IMSRV)
	@echo "linking $@ ..."
ifeq ($(FREEBSD), 0)
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS) $(IMdkitLIB) $(OBJ_IMSRV) \
		-lXtst $(LDFLAGS) -L/usr/X11R6/$(LIB)
else
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS) $(IMdkitLIB) $(OBJ_IMSRV) \
		-lXtst $(LDFLAGS) -L/usr/local/$(LIB)
endif
	@rm -f core.* vgcore.*

hime-tslearn: $(OBJS_TSLEARN) im-client/libhime-im-client.so
	@echo "linking $@ ..."
	$(CCLD) $(gcc_ld_run_path) -o $@ $^ -L./im-client -lhime-im-client \
		$(LDFLAGS)

hime-ts-edit: $(OBJS_TS_EDIT) im-client/libhime-im-client.so
	@echo "linking $@ ..."
	$(CCLD) $(gcc_ld_run_path) -o $@ $^ -L./im-client -lhime-im-client \
		$(LDFLAGS)

hime-juyin-learn: $(OBJS_JUYIN_LEARN)
	@echo "linking $@ ..."
	$(CCLD) -o $@ $^ $(LDFLAGS)
	@rm -f core.*

hime-sim2trad: $(OBJS_hime-sim2trad)
	@echo "linking $@ ..."
	$(CC) $(gcc_ld_run_path) -o $@ $^ $(LDFLAGS)
	@rm -f core.*

hime-trad2sim:  hime-sim2trad
	ln -sf hime-sim2trad $@

hime-setup: $(OBJS_hime_setup) im-client/libhime-im-client.so
	@echo "linking $@ ..."
	@rm -f core.*
	$(CCLD) $(gcc_ld_run_path) -o $@ $^ -L./im-client -lhime-im-client \
		$(LDFLAGS)

hime-phoa2d: $(OBJS_hime-phoa2d) im-client/libhime-im-client.so
	@echo "linking $@ ..."
	$(CCLD) $(gcc_ld_run_path) -o $@ $^ -L./im-client -lhime-im-client \
		$(LDFLAGS)

hime-phod2a: $(OBJS_hime-phod2a)
	@echo "linking $@ ..."
	$(CCLD) -lX11 -o $@ $^ $(LDFLAGS)

hime-tsa2d32:  $(OBJS_hime-tsa2d32) im-client/libhime-im-client.so
	@echo "linking $@ ..."
	$(CCLD) $(gcc_ld_run_path) -o $@ $^ -L./im-client -lhime-im-client \
		$(LDFLAGS)

hime-tsd2a32: $(OBJS_hime-tsd2a32)
	@echo "linking $@ ..."
	$(CCLD) -o $@ $^ $(LDFLAGS)

hime-cin2gtab: $(OBJS_cin2gtab)
	@echo "linking $@ ..."
	$(CCLD) -o $@ $^ $(LDFLAGS)
	@rm -f data/*.gtab

hime-gtab2cin: $(OBJS_gtab2cin)
	@echo "linking $@ ..."
	$(CCLD) -o $@ $^ $(LDFLAGS)

hime-gtab-merge: $(OBJS_gtab_merge)
	@echo "linking $@ ..."
	$(CCLD) -o $@ $^ $(LDFLAGS)

kbmcv: $(OBJS_kbmcv)
	@echo "linking $@ ..."
	$(CCLD) -o $@ $^ $(LDFLAGS)

hime-gb-toggle: $(OBJS_hime_gb_toggle) im-client/libhime-im-client.so
	@echo "linking $@ ..."
	$(CCLD) $(gcc_ld_run_path) -o $@ $^ -L./im-client -lhime-im-client \
		$(LDFLAGS)

hime-kbm-toggle: hime-gb-toggle
	ln -sf hime-gb-toggle $@

hime-exit: hime-gb-toggle
	ln -sf hime-gb-toggle $@

hime-trad: hime-gb-toggle
	ln -sf hime-gb-toggle $@

hime-sim: hime-gb-toggle
	ln -sf hime-gb-toggle $@

hime-message: $(OBJS_hime_message) im-client/libhime-im-client.so
	@echo "linking $@ ..."
	$(CCLD) $(gcc_ld_run_path) -o $@ $^ -L./im-client -lhime-im-client \
		$(LDFLAGS)

pin-juyin: $(OBJS_pin_juyin)
	@echo "linking $@ ..."
	$(CCLD) -o $@ $^ $(LDFLAGS)

hime-tsin2gtab-phrase: $(OBJS_tsin2gtab_phrase)
	@echo "linking $@ ..."
	$(CCLD) -o $@ $^ $(LDFLAGS)

.PHONY: install
install:
	install -d "$(bindir)"
	install $(PROGS) "$(bindir)"
	rm -f "$(bindir)/hime-trad2sim"; ln -sf hime-sim2trad "$(bindir)/hime-trad2sim"
	rm -f "$(bindir)/hime-exit"; ln -sf hime-gb-toggle "$(bindir)/hime-exit"
	rm -f "$(bindir)/hime-trad"; ln -sf hime-gb-toggle "$(bindir)/hime-trad"
	rm -f "$(bindir)/hime-sim"; ln -sf hime-gb-toggle "$(bindir)/hime-sim"
	rm -f "$(bindir)/hime-kbm-toggle"; ln -sf hime-gb-toggle "$(bindir)/hime-kbm-toggle"
	install -d "$(himelibdir)"
	install -d "$(includedir)"
	install -m 644 im-client/hime-im-client.h im-client/hime-im-client-attr.h "$(includedir)"
	$(MAKE) -C im-client install
	$(MAKE) -C modules install
	if [ $(GTK_IM) = 'Y' ]; then $(MAKE) -C gtk-im install; fi
	if [ $(GTK3_IM) = 'Y' ]; then $(MAKE) -C gtk3-im install; fi
	if [ $(QT5_IM) = 'Y' ]; then $(MAKE) -C qt5-im install; fi

.PHONY: uninstall
uninstall:
	cd $(bindir) && rm -f $(PROGS) $(PROGS_SYM)
	cd $(includedir) && rm -f hime-im-client.h hime-im-client-attr.h
	$(MAKE) -C im-client uninstall
	$(MAKE) -C modules uninstall
	if [ $(GTK_IM) = 'Y' ]; then $(MAKE) -C gtk-im uninstall; fi
	if [ $(GTK3_IM) = 'Y' ]; then $(MAKE) -C gtk3-im uninstall; fi
	if [ $(QT5_IM) = 'Y' ]; then $(MAKE) -C qt5-im uninstall; fi
	rmdir --ignore-fail-on-non-empty $(bindir)
	rmdir --ignore-fail-on-non-empty $(himelibdir)
	rmdir --ignore-fail-on-non-empty $(includedir)

.PHONY: clean
clean:
	@echo "clean up"
	$(MAKE) -C IMdkit clean
	$(MAKE) -C im-client clean
	$(MAKE) -C modules clean
	if [ $(GTK_IM) = 'Y' ]; then $(MAKE) -C gtk-im clean; fi
	if [ $(GTK3_IM) = 'Y' ]; then $(MAKE) -C gtk3-im clean; fi
	if [ $(QT5_IM) = 'Y' ]; then $(MAKE) -C qt5-im clean; fi
	@rm -f *.o *.E *.db *.pico *.so tags $(PROGS) $(PROGS_CV) \
		$(DATA) .depend $(PROGS_SYM) hime.log hime-fedora.spec
	@find . '(' -name '.ted*' -o -name '*~' -o -name 'core.*' -o -name 'vgcore.*' ')' -exec rm {} \;

.depend:
	@echo "building $@ ..."
	$(CCX) $(CPPFLAGS) $(CFLAGS) -MM *.c > $@


include .depend
