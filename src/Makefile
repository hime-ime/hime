
include ../config.mak

CFLAGS= -DUNIX=1 $(WALL) $(OPTFLAGS) $(GTKINC) -I./IMdkit/include -I./im-client \
        -DHIME_TABLE_DIR=\"$(HIME_TABLE_DIR)\" \
        -DHIME_OGG_DIR=\"$(HIME_OGG_DIR)\" \
        -DDOC_DIR=\"$(DOC_DIR)\" \
        -DHIME_ICON_DIR=\"$(HIME_ICON_DIR)\" \
        -DHIME_SCRIPT_DIR=\"$(HIME_SCRIPT_DIR)\" -DHIME_BIN_DIR=\"$(HIME_BIN_DIR)\" \
        -DSYS_ICON_DIR=\"$(SYS_ICON_DIR)\" -DFREEBSD=$(FREEBSD) -DMAC_OS=$(MAC_OS) \
        -DG_DISABLE_SINGLE_INCLUDES -DG_DISABLE_DEPRECATED \
        -DGDK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED \
        -DGTK_DISABLE_SINGLE_INCLUDES -DGTK_DISABLE_DEPRECATED

VERSION_FILE = ../VERSION.hime

OBJ_IMSRV = im-addr.o im-dispatch.o im-srv.o hime-crypt.o

HIME1_OBJ = win-message.pico win-sym.pico win-inmd-switch.pico pinyin.pico \
	win-pho-near.pico win-kbm.pico hime-module.pico pho2pinyin.pico
HIME2_OBJ = t2s-lookup.pico


hime_pho_o  = win-pho.o pho.o pho-util.o pho-sym.o table-update.o pho-dbg.o
hime_gtab_o = gtab.o win-gtab.o gtab-util.o gtab-list.o gtab-buf.o


OBJS      = hime.o eve.o util.o hime-conf.o hime-settings.o locale.o \
	hime-icon.o about.o html-browser.o \
	hime-exec-script.o pho-play.o cache.o gtk_bug_fix.o \
	phrase-save-menu.o \
	$(hime_pho_o) $(hime_gtab_o) \
	hime-common.o phrase.o t2s-lookup.o gtab-use-count.o \
	win-save-phrase.o unix-exec.o pho-kbm-name.o statistic.o tsin-scan.o \
	hime-module.o lang.o \
	hime-module-cb.o gtab-init.o fullchar.o gtab-tsin-fname.o

OBJS_TSLEARN = hime-tslearn.o util.o hime-conf.o pho-util.o tsin-util.o \
	hime-send.o pho-sym.o table-update.o locale.o \
	hime-settings.o hime-common.o hime-icon.o pho-dbg.o \
	pho2pinyin.o pinyin.o lang.o gtab-list.o gtab-init.o fullchar.o \
	gtab-tsin-fname.o unix-exec.o gtab-util.o

OBJS_TS_EDIT = hime-ts-edit.o util.o hime-conf.o pho-util.o tsin-util.o \
	hime-send.o pho-sym.o table-update.o locale.o \
	hime-settings.o hime-common.o hime-icon.o pho-dbg.o \
	pho2pinyin.o pinyin.o lang.o gtab-list.o gtab-init.o fullchar.o \
	gtab-tsin-fname.o unix-exec.o gtab-util.o

OBJS_JUYIN_LEARN = hime-juyin-learn.o locale.o util.o pho-util.o pho-sym.o \
	pho2pinyin.o hime-settings.o hime-conf.o table-update.o pinyin.o \
	hime-icon.o pho-dbg.o

OBJS_hime-sim2trad = hime-sim2trad.o util.o hime2.so locale.o hime-conf.o hime-icon.o

OBJS_hime-phod2a = hime-phod2a.o pho-util.o hime-conf.o pho-sym.o table-update.o \
	pho-dbg.o locale.o hime-settings.o util.o

OBJS_hime-tsa2d32 = hime-tsa2d32.o hime-send.o util.o pho-sym.o hime-conf.o \
	locale.o pho-lookup.o pinyin.o pho2pinyin.o pho-dbg.o

OBJS_hime-phoa2d = hime-phoa2d.o pho-sym.o hime-send.o hime-conf.o locale.o \
	pho-lookup.o util.o pho-dbg.o

OBJS_kbmcv = kbmcv.o pho-sym.o util.o locale.o

OBJS_hime-tsd2a32 = hime-tsd2a32.o pho-sym.o pho-dbg.o locale.o util.o \
	gtab-dbg.o pho2pinyin.o hime-conf.o pinyin.o

OBJS_cin2gtab     = cin2gtab.o gtab-util.o util.o locale.o
OBJS_gtab2cin     = gtab2cin.o gtab-util.o util.o locale.o
OBJS_gtab_merge   = hime-gtab-merge.o gtab-util.o util.o locale.o
OBJS_hime_setup   = hime-setup.o hime-conf.o util.o hime-send.o hime-settings.o \
	html-browser.o hime-setup-list.o locale.o hime-setup-pho.o about.o \
	lang.o hime-icon.o hime-setup-gtab.o gtab-list.o hime-exec-script.o \
	pho-kbm-name.o hime-module-cb.o

OBJS_hime_gb_toggle  = hime-gb-toggle.o  hime-conf.o util.o hime-send.o
OBJS_hime_kbm_toggle = hime-kbm-toggle.o hime-conf.o util.o hime-send.o
OBJS_hime_exit       = hime-exit.o       hime-conf.o util.o hime-send.o
OBJS_hime_message    = hime-message.o    hime-conf.o util.o hime-send.o

OBJS_pin_juyin       = pin-juyin.o util.o pho-lookup.o locale.o pho-sym.o

OBJS_tsin2gtab_phrase = hime-tsin2gtab-phrase.o hime-conf.o util.o locale.o \
	pho-dbg.o pho-sym.o gtab-dbg.o lang.o


#WALL=-Wall

ifeq ($(USE_XIM),Y)
IMdkitLIB   = IMdkit/lib/libXimd.a
CFLAGS     += -DUSE_XIM=1
OBJS       += IC.o
endif

ifeq ($(USE_TRAY),Y)
CFLAGS     += -DTRAY_ENABLED=1
OBJS       += tray.o eggtrayicon.o tray-win32.o
endif

ifeq ($(USE_I18N),Y)
CFLAGS     += -DHIME_i18n_message=1
endif

ifeq ($(USE_TSIN),Y)
hime_tsin_o = tsin.o tsin-util.o win0.o win1.o tsin-parse.o
CFLAGS     += -DUSE_TSIN=1
OBJS       += $(hime_tsin_o)
endif

ifeq ($(MAC_OS),1)
EXTRA_LDFLAGS=-bind_at_load 
endif


HIME_SO   = hime1.so hime2.so

PROGS     = hime \
	hime-tsd2a32 hime-tsa2d32 hime-phoa2d hime-phod2a \
	hime-tslearn hime-setup cin2gtab gtab2cin \
	hime-juyin-learn hime-sim2trad hime-gb-toggle hime-message hime-gtab-merge \
	hime-kbm-toggle hime-tsin2gtab-phrase hime-exit hime-ts-edit

PROGS_SYM = hime-trad2sim

PROGS_CV  = kbmcv pin-juyin




.SUFFIXES:  .c .o .E .pico .cpp

.cpp.o:
	@echo "  $< -> $@"
	@$(CCX) $(CFLAGS) -c $<
.c.o:
	@echo "  $< -> $@"
	@$(CC) $(CFLAGS) -c $<
.c.pico:
	@echo "  $< -> $@"
	@$(CC) $(CFLAGS) -c -fpic -o $@ $<
.cpp.pico:
	@echo "  $< -> $@"
	@$(CCX) $(CFLAGS) -c -fpic -o $@ $<
.c.E:
	@echo "  $< -> $@"
	@$(CC) $(CFLAGS) -E -o $@ $<
.cpp.E:
	@echo "  $< -> $@"
	@$(CCX) $(CFLAGS) -E -o $@ $<


all: hime-version.h $(PROGS) $(PROGS_SYM) $(DATA) $(PROGS_CV)
	if [ $(BUILD_MODULE) = 'Y' ]; then $(MAKE) -C modules; fi
	if [ $(GTK_IM) = 'Y' ]; then $(MAKE) -C gtk-im; fi
	if [ $(GTK3_IM) = 'Y' ]; then $(MAKE) -C gtk3-im; fi
	if [ $(QT_IM) = 'Y' ]; then $(MAKE) -C qt-im; fi
	if [ $(QT4_IM) = 'Y' ]; then $(MAKE) -C qt4-im; fi

hime1.so: $(HIME1_OBJ)
	@echo "linking $@ ..."
	@$(CCLD) $(SO_FLAGS) -o $@ $^ $(LDFLAGS)

hime2.so: $(HIME2_OBJ)
	@echo "linking $@ ..."
	@$(CCLD) $(SO_FLAGS) -o $@ $^ $(LDFLAGS)

$(IMdkitLIB):
	@echo "building $@ ..."
	$(MAKE) -C IMdkit/lib

im-client/libhime-im-client.so:
	@echo "building $@ ..."
	$(MAKE) -C im-client

hime: $(HIME_SO) $(OBJS) $(IMdkitLIB) $(OBJ_IMSRV)
	@echo "linking $@ ..."
	@$(CCLD) $(EXTRA_LDFLAGS) $(gcc_ld_run_path) -o $@ $^ -lXtst $(LDFLAGS) -L/usr/X11R6/$(LIB)
	@rm -f core.* vgcore.*

hime-nocur: $(HIME_SO) $(OBJS) $(IMdkitLIB) $(OBJ_IMSRV)
	@echo "linking $@ ..."
	@$(CCLD) -Wl,-rpath,$(himelibdir) $(EXTRA_LDFLAGS) -o $@ $^ -lXtst $(LDFLAGS) -L/usr/X11R6/$(LIB)
	@rm -f core.*

hime-tslearn: $(OBJS_TSLEARN)
	@echo "linking $@ ..."
	@$(CCLD) $(gcc_ld_run_path) -o $@ $^ -L./im-client -lhime-im-client $(LDFLAGS)

hime-ts-edit: $(OBJS_TS_EDIT)
	@echo "linking $@ ..."
	@$(CCLD) $(gcc_ld_run_path) -o $@ $^ -L./im-client -lhime-im-client $(LDFLAGS)

hime-juyin-learn: $(OBJS_JUYIN_LEARN)
	@echo "linking $@ ..."
	@$(CCLD) -o $@ $^ $(LDFLAGS)
	@rm -f core.*

hime-sim2trad: $(OBJS_hime-sim2trad)
	@echo "linking $@ ..."
	@$(CC) $(gcc_ld_run_path) -o $@ $^ $(LDFLAGS)
	@rm -f core.*

hime-trad2sim:  hime-sim2trad
	ln -sf hime-sim2trad hime-trad2sim

hime-setup: $(OBJS_hime_setup) im-client/libhime-im-client.so
	@echo "linking $@ ..."
	@rm -f core.*
	@$(CCLD) $(gcc_ld_run_path) -o $@ $^ -L./im-client -lhime-im-client $(LDFLAGS)

hime-phoa2d: $(OBJS_hime-phoa2d) im-client/libhime-im-client.so
	@echo "linking $@ ..."
	@$(CCLD) $(gcc_ld_run_path) -o $@ $^ -L./im-client -lhime-im-client $(LDFLAGS)

hime-phod2a: $(OBJS_hime-phod2a)
	@echo "linking $@ ..."
	@$(CCLD) -lX11 -o $@ $^ $(LDFLAGS)

hime-tsa2d32:  $(OBJS_hime-tsa2d32) im-client/libhime-im-client.so
	@echo "linking $@ ..."
	@$(CCLD) $(gcc_ld_run_path) -o $@ $^ -L./im-client -lhime-im-client $(LDFLAGS)

#hime-tsd2a:
#	$(CCLD) -o $@ $(LDFLAGS)

hime-tsd2a32: $(OBJS_hime-tsd2a32)
	@echo "linking $@ ..."
	@$(CCLD) -o $@ $^ $(LDFLAGS)

cin2gtab: $(OBJS_cin2gtab)
	@echo "linking $@ ..."
	@$(CCLD) -o $@ $^ $(LDFLAGS)
	@rm -f data/*.gtab

gtab2cin: $(OBJS_gtab2cin)
	@echo "linking $@ ..."
	@$(CCLD) -o $@ $^ $(LDFLAGS)

hime-gtab-merge: $(OBJS_gtab_merge)
	@echo "linking $@ ..."
	@$(CCLD) -o $@ $^ $(LDFLAGS)

kbmcv: $(OBJS_kbmcv)
	@echo "linking $@ ..."
	@$(CCLD) -o $@ $^ $(LDFLAGS)

hime-gb-toggle: $(OBJS_hime_gb_toggle)
	@echo "linking $@ ..."
	@$(CCLD) $(gcc_ld_run_path) -o $@ $^ -L./im-client -lhime-im-client $(LDFLAGS)

hime-kbm-toggle: $(OBJS_hime_kbm_toggle)
	@echo "linking $@ ..."
	@$(CCLD) $(gcc_ld_run_path) -o $@ $^ -L./im-client -lhime-im-client $(LDFLAGS)

hime-exit: $(OBJS_hime_exit)
	@echo "linking $@ ..."
	@$(CCLD) $(gcc_ld_run_path) -o $@ $^ -L./im-client -lhime-im-client $(LDFLAGS)

hime-message: $(OBJS_hime_message)
	@echo "linking $@ ..."
	@$(CCLD) $(gcc_ld_run_path) -o $@ $^ -L./im-client -lhime-im-client $(LDFLAGS)

pin-juyin: $(OBJS_pin_juyin)
	@echo "linking $@ ..."
	@$(CCLD) -o $@ $^ $(LDFLAGS)

hime-tsin2gtab-phrase: $(OBJS_tsin2gtab_phrase)
	@echo "linking $@ ..."
	@$(CCLD) -o $@ $^ $(LDFLAGS)


hime-version.h: $(VERSION_FILE)
	@echo "building $@ ... current version is '"`cat $<`"'"
	@echo '#define HIME_VERSION "'`cat $<`'"' > $@

#ibin: hime-nocur
#	install $(PROGS) $(bindir); \
#	cp hime-nocur $(bindir)/hime; \
#	rm -f $(bindir)/hime-trad2sim; ln -sf hime-sim2trad $(bindir)/hime-trad2sim
#	install $(HIME_SO) $(himelibdir)

install:
	install -d $(bindir)
	install $(PROGS) $(bindir)
	rm -f $(bindir)/hime-trad2sim; ln -sf hime-sim2trad $(bindir)/hime-trad2sim
	install -d $(himelibdir)
	install $(HIME_SO) $(himelibdir)
	$(MAKE) -C im-client install
	if [ $(BUILD_MODULE) = 'Y' ]; then $(MAKE) -C modules install; fi
	if [ $(GTK_IM) = 'Y' ]; then $(MAKE) -C gtk-im install; fi
	if [ $(GTK3_IM) = 'Y' ]; then $(MAKE) -C gtk3-im install; fi
	if [ $(QT_IM) = 'Y' ]; then $(MAKE) -C qt-im install; fi
	if [ $(QT4_IM) = 'Y' ]; then $(MAKE) -C qt4-im install; fi

clean:
	@echo "clean up"
	$(MAKE) -C IMdkit clean
	$(MAKE) -C im-client clean
	$(MAKE) -C modules clean
	if [ $(GTK_IM) = 'Y' ]; then $(MAKE) -C gtk-im clean; fi
	if [ $(GTK3_IM) = 'Y' ]; then $(MAKE) -C gtk3-im clean; fi
	$(MAKE) -C qt-im clean
	$(MAKE) -C qt4-im clean
	@rm -f *.o *.E *.db *.pico *.so tags $(PROGS) hime-nocur $(PROGS_CV) \
		$(DATA) .depend hime-trad2sim hime.log hime-fedora.spec
	@find . '(' -name '.ted*' -o -name '*~' -o -name 'core.*' -o -name 'vgcore.*' ')' -exec rm {} \;

.depend:
	@echo "building $@ ..."
	@$(CCX) $(CFLAGS) -MM *.c > $@


include .depend

