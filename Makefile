OPTFLAGS=-g

include config.mak
include suffixes-rule

hime_tsin_o = tsin.o tsin-util.o win0.o win1.o tsin-parse.o
hime_pho_o = win-pho.o pho.o pho-util.o pho-sym.o table-update.o pho-dbg.o
hime_gtab_o = gtab.o win-gtab.o gtab-util.o gtab-list.o gtab-buf.o

HIME_SO= hime1.so hime2.so

OBJS=hime.o eve.o util.o hime-conf.o hime-settings.o locale.o hime-icon.o about.o html-browser.o \
     hime-exec-script.o $(HIME_SO) pho-play.o cache.o gtk_bug_fix.o phrase-save-menu.o \
     $(hime_pho_o) $(hime_gtab_o) hime-common.o phrase.o t2s-lookup.o gtab-use-count.o \
     win-save-phrase.o unix-exec.o pho-kbm-name.o statistic.o tsin-scan.o hime-module.o lang.o \
     hime-module-cb.o gtab-init.o fullchar.o gtab-tsin-fname.o

OBJS_TSLEARN=hime-tslearn.o util.o hime-conf.o pho-util.o tsin-util.o hime-send.o pho-sym.o \
             table-update.o locale.o hime-settings.o hime-common.o hime-icon.o pho-dbg.o  \
             pho2pinyin.o pinyin.o lang.o gtab-list.o gtab-init.o fullchar.o \
             gtab-tsin-fname.o unix-exec.o gtab-util.o

OBJS_TS_EDIT=hime-ts-edit.o util.o hime-conf.o pho-util.o tsin-util.o hime-send.o pho-sym.o \
             table-update.o locale.o hime-settings.o hime-common.o hime-icon.o pho-dbg.o  \
             pho2pinyin.o pinyin.o lang.o gtab-list.o gtab-init.o fullchar.o \
             gtab-tsin-fname.o unix-exec.o gtab-util.o

OBJS_JUYIN_LEARN=hime-juyin-learn.o locale.o util.o pho-util.o pho-sym.o pho2pinyin.o \
                 hime-settings.o hime-conf.o table-update.o pinyin.o hime-icon.o pho-dbg.o
OBJS_hime-sim2trad=hime-sim2trad.o util.o hime2.so locale.o hime-conf.o hime-icon.o
OBJS_hime-phod2a=hime-phod2a.o pho-util.o hime-conf.o pho-sym.o table-update.o pho-dbg.o locale.o \
             hime-settings.o util.o
OBJS_hime-tsa2d32=hime-tsa2d32.o hime-send.o util.o pho-sym.o hime-conf.o locale.o pho-lookup.o \
pinyin.o pho2pinyin.o pho-dbg.o
OBJS_hime-phoa2d=hime-phoa2d.o pho-sym.o hime-send.o hime-conf.o locale.o pho-lookup.o util.o pho-dbg.o
OBJS_kbmcv=kbmcv.o pho-sym.o util.o locale.o
OBJS_hime-tsd2a32=hime-tsd2a32.o pho-sym.o pho-dbg.o locale.o util.o gtab-dbg.o pho2pinyin.o \
	hime-conf.o pinyin.o
OBJS_cin2gtab=cin2gtab.o gtab-util.o util.o locale.o
OBJS_gtab2cin=gtab2cin.o gtab-util.o util.o locale.o
OBJS_gtab_merge=hime-gtab-merge.o gtab-util.o util.o locale.o
OBJS_hime_setup=hime-setup.o hime-conf.o util.o hime-send.o hime-settings.o html-browser.o \
	hime-setup-list.o locale.o hime-setup-pho.o about.o lang.o \
	hime-icon.o hime-setup-gtab.o gtab-list.o hime-exec-script.o pho-kbm-name.o hime-module-cb.o

OBJS_hime_gb_toggle = hime-gb-toggle.o hime-conf.o util.o hime-send.o
OBJS_hime_kbm_toggle = hime-kbm-toggle.o hime-conf.o util.o hime-send.o
OBJS_hime_exit = hime-exit.o hime-conf.o util.o hime-send.o
OBJS_hime_message = hime-message.o hime-conf.o util.o hime-send.o
OBJS_pin_juyin = pin-juyin.o util.o pho-lookup.o locale.o pho-sym.o

OBJS_tsin2gtab_phrase = hime-tsin2gtab-phrase.o hime-conf.o util.o locale.o \
	pho-dbg.o pho-sym.o gtab-dbg.o lang.o

#WALL=-Wall

ifeq ($(USE_XIM),Y)
IMdkitLIB = IMdkit/lib/libXimd.a
CFLAGS += -DUSE_XIM=1
OBJS+=IC.o
endif

ifeq ($(USE_TRAY),Y)
CFLAGS += -DTRAY_ENABLED=1
OBJS += tray.o eggtrayicon.o tray-win32.o
endif

ifeq ($(USE_I18N),Y)
CFLAGS += -DHIME_i18n_message=1
endif

ifeq ($(USE_TSIN),Y)
CFLAGS += -DUSE_TSIN=1
OBJS += $(hime_tsin_o)
endif

OBJ_IMSRV=im-addr.o im-dispatch.o im-srv.o hime-crypt.o

PROGS=hime hime-tsd2a32 hime-tsa2d32 hime-phoa2d hime-phod2a hime-tslearn hime-setup cin2gtab gtab2cin \
	hime-juyin-learn hime-sim2trad hime-gb-toggle hime-message hime-gtab-merge \
	hime-kbm-toggle hime-tsin2gtab-phrase hime-exit hime-ts-edit
PROGS_SYM=hime-trad2sim
PROGS_CV=kbmcv pin-juyin


all:	$(PROGS) hime-trad2sim $(HIME_SO) $(DATA) $(PROGS_CV) hime-fedora.spec
	$(MAKE) -C data
	if [ $(BUILD_MODULE) = 'Y' ]; then $(MAKE) -C modules; fi
	if [ $(USE_I18N) = 'Y' ]; then $(MAKE) -C po; fi
	if [ $(GTK_IM) = 'Y' ]; then $(MAKE) -C gtk-im; fi
	if [ $(GTK3_IM) = 'Y' ]; then $(MAKE) -C gtk3-im; fi
	if [ $(QT_IM) = 'Y' ]; then $(MAKE) -C qt-im; fi
	if [ $(QT4_IM) = 'Y' ]; then $(MAKE) -C qt4-im; fi

#gcc_ld_run_path=-Wl,-rpath,$(hime_ld_run_path)

hime:   $(OBJS) $(IMdkitLIB) $(OBJ_IMSRV)
	$(CCLD) $(EXTRA_LDFLAGS) $(gcc_ld_run_path) -o $@ $(OBJS) $(IMdkitLIB) $(OBJ_IMSRV) -lXtst $(LDFLAGS) -L/usr/X11R6/$(LIB)
	rm -f core.* vgcore.*

hime-nocur:   $(OBJS) $(IMdkitLIB) $(OBJ_IMSRV)
	$(CCLD) -Wl,-rpath,$(himelibdir) $(EXTRA_LDFLAGS) -o $@ $(OBJS) $(IMdkitLIB) $(OBJ_IMSRV) -lXtst $(LDFLAGS) -L/usr/X11R6/$(LIB)
	rm -f core.*

hime-tslearn:        $(OBJS_TSLEARN)
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_TSLEARN) -L./im-client -lhime-im-client $(LDFLAGS)

hime-ts-edit:        $(OBJS_TS_EDIT)
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_TS_EDIT) -L./im-client -lhime-im-client $(LDFLAGS)

hime-juyin-learn:        $(OBJS_JUYIN_LEARN)
	$(CCLD) -o $@ $(OBJS_JUYIN_LEARN) $(LDFLAGS)
	rm -f core.*
hime-sim2trad:        $(OBJS_hime-sim2trad)
	$(CC) $(gcc_ld_run_path) -o $@ $(OBJS_hime-sim2trad) $(LDFLAGS)
	rm -f core.*
hime-trad2sim:	hime-sim2trad
	ln -sf hime-sim2trad hime-trad2sim

hime-setup:     $(OBJS_hime_setup) im-client/libhime-im-client.so
	rm -f core.*
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_hime_setup) -L./im-client -lhime-im-client $(LDFLAGS)

hime-phoa2d: $(OBJS_hime-phoa2d) im-client/libhime-im-client.so
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_hime-phoa2d) -L./im-client -lhime-im-client $(LDFLAGS)

hime-phod2a: $(OBJS_hime-phod2a)
	$(CCLD) -lX11 -o $@ $(OBJS_hime-phod2a) $(LDFLAGS)

hime-tsa2d32:  $(OBJS_hime-tsa2d32) im-client/libhime-im-client.so
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_hime-tsa2d32) -L./im-client -lhime-im-client $(LDFLAGS)

hime-tsd2a:
	$(CCLD) -o $@ $(LDFLAGS)

hime-tsd2a32:  $(OBJS_hime-tsd2a32)
	$(CCLD) -o $@ $(OBJS_hime-tsd2a32) $(LDFLAGS)

cin2gtab:  $(OBJS_cin2gtab)
	$(CCLD) -o $@ $(OBJS_cin2gtab) $(LDFLAGS)
	rm -f data/*.gtab

gtab2cin:  $(OBJS_gtab2cin)
	$(CCLD) -o $@ $(OBJS_gtab2cin) $(LDFLAGS)

hime-gtab-merge:  $(OBJS_gtab_merge)
	$(CCLD) -o $@ $(OBJS_gtab_merge) $(LDFLAGS)

kbmcv:  $(OBJS_kbmcv)
	$(CCLD) -o $@ $(OBJS_kbmcv) $(LDFLAGS)

hime-gb-toggle:	$(OBJS_hime_gb_toggle)
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_hime_gb_toggle) -L./im-client -lhime-im-client $(LDFLAGS)

hime-kbm-toggle:	$(OBJS_hime_kbm_toggle)
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_hime_kbm_toggle) -L./im-client -lhime-im-client $(LDFLAGS)

hime-exit:	$(OBJS_hime_exit)
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_hime_exit) -L./im-client -lhime-im-client $(LDFLAGS)

hime-message:	$(OBJS_hime_message)
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_hime_message) -L./im-client -lhime-im-client $(LDFLAGS)

pin-juyin:	$(OBJS_pin_juyin)
	$(CCLD) -o $@ $(OBJS_pin_juyin) $(LDFLAGS)

hime-tsin2gtab-phrase:       $(OBJS_tsin2gtab_phrase)
	$(CCLD) -o $@ $(OBJS_tsin2gtab_phrase) $(LDFLAGS)

im-client/libhime-im-client.so:
	$(MAKE) -C im-client

hime-version.h:	VERSION.hime
	echo '#define HIME_VERSION "'`cat VERSION.hime`'"' > hime-version.h

hime1_so += win-message.pico win-sym.pico \
win-inmd-switch.pico pinyin.pico win-pho-near.pico win-kbm.pico hime-module.pico pho2pinyin.pico

hime1.so: $(hime1_so) pho.o tsin.o eve.o gtab.o win-sym.o
	$(CCLD) $(SO_FLAGS) -o $@ $(hime1_so) $(LDFLAGS)

hime2_so= t2s-lookup.pico
hime2.so: $(hime2_so) hime-conf.o
	$(CCLD) $(SO_FLAGS) -o $@ $(hime2_so) $(LDFLAGS)

$(IMdkitLIB):
	$(MAKE) -C IMdkit/lib

ibin:	hime-nocur
	install $(PROGS) $(bindir); \
	cp hime-nocur $(bindir)/hime; \
	rm -f $(bindir)/hime-trad2sim; ln -sf hime-sim2trad $(bindir)/hime-trad2sim
	install $(HIME_SO) $(himelibdir)

install:
	install -d $(datadir)/pixmaps
	install -m 644 hime.png $(datadir)/pixmaps
	$(MAKE) -C icons install
	install -d $(himelibdir)
	install $(HIME_SO) $(himelibdir)
	install -d $(bindir)
	$(MAKE) -C data install
	$(MAKE) -C im-client install
	if [ $(BUILD_MODULE) = 'Y' ]; then $(MAKE) -C modules install; fi
	if [ $(GTK_IM) = 'Y' ]; then $(MAKE) -C gtk-im install; fi
	if [ $(GTK3_IM) = 'Y' ]; then $(MAKE) -C gtk3-im install; fi
	if [ $(QT_IM) = 'Y' ]; then $(MAKE) -C qt-im install; fi
	if [ $(QT4_IM) = 'Y' ]; then $(MAKE) -C qt4-im install; fi
	if [ $(prefix) = /usr/local ]; then \
	   install -m 644 hime.png /usr/share/pixmaps; \
	   install -d $(DOC_DIR); \
	   install -m 644 README.html Changelog.html $(DOC_DIR); \
	   install $(PROGS) $(bindir); \
	   rm -f $(bindir)/hime-trad2sim; ln -sf hime-sim2trad $(bindir)/hime-trad2sim; \
	else \
	   install -d $(DOC_DIR_i); \
	   install -m 644 README.html Changelog.html $(DOC_DIR_i); \
	   install $(PROGS) $(bindir); \
	   rm -f $(bindir)/hime-trad2sim; ln -sf hime-sim2trad $(bindir)/hime-trad2sim; \
	fi
	$(MAKE) -C scripts install
	$(MAKE) -C menu install
	$(MAKE) -C man install
	if [ $(USE_I18N) = 'Y' ]; then $(MAKE) -C po install; fi

clean:
	$(MAKE) -C IMdkit clean
	$(MAKE) -C data clean
	$(MAKE) -C scripts clean
	$(MAKE) -C im-client clean
	$(MAKE) -C modules clean
	if [ $(GTK_IM) = 'Y' ]; then $(MAKE) -C gtk-im clean; fi
	if [ $(GTK3_IM) = 'Y' ]; then $(MAKE) -C gtk3-im clean; fi
	$(MAKE) -C qt-im clean
	$(MAKE) -C qt4-im clean
	$(MAKE) -C man clean
	$(MAKE) -C menu clean
	$(MAKE) -C po clean
	rm -f *.o *.E *.db *.pico *.so config.mak tags $(PROGS) hime-nocur $(PROGS_CV) \
	$(DATA) .depend hime-trad2sim hime.log hime-fedora.spec
	find . '(' -name '.ted*' -o -name '*~' -o -name 'core.*' -o -name 'vgcore.*' ')' -exec rm {} \;

.depend:
	$(CCX) $(CFLAGS) -MM *.cpp > $@

config.mak: VERSION.hime configure
	./configure

hime-fedora.spec:	hime-fedora.spec.in
	rm -f $@
	sed -e "s/__hime_version__/$(HIME_VERSION)/" < $< > $@

include .depend
