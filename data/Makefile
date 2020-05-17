include ../config.mak

SRC_DIR       = ../src
IM_CLIENT_DIR = $(SRC_DIR)/im-client

.SUFFIXES: .kbmsrc .kbm .cin .gtab
CFLAGS+= $(OPTFLAGS) $(GTKINC) \
		-DGDK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED \
		-DGTK_DISABLE_SINGLE_INCLUDES -DGTK_DISABLE_DEPRECATED \
		-DG_DISABLE_SINGLE_INCLUDES \
		-DGSEAL_ENABLE \
		-I$(SRC_DIR) \
		-I../src/IMdkit/include -I../src/im-client

export HIME_NO_RELOAD=

GTAB_DATA = ar30-big.gtab \
	ar30.gtab \
	array40.gtab \
	cj-punc.gtab \
	cj.gtab \
	cj5.gtab \
	cj543.gtab \
	esperanto.gtab \
	greek.gtab \
	hangul-roman.gtab \
	hangul.gtab \
	jyutping.gtab \
	latin-letters.gtab \
	pinyin.gtab \
	russian.gtab \
	simplex-punc.gtab \
	simplex.gtab \
	symbols.gtab \
	vims.gtab

KBM_DATA = colemak.kbm \
	dvorak.kbm \
	et.kbm \
	et26.kbm \
	hsu.kbm \
	ibm.kbm \
	mitac.kbm \
	pinyin-no-tone.kbm \
	pinyin.kbm \
	zo.kbm

ifeq ($(INSTALL_DFSG_INCOMPATIBLE),Y)
GTAB_DATA += dayi3.gtab
endif

DATA = $(GTAB_DATA) $(KBM_DATA) \
	tsin32 \
	tsin32.idx \
	s-tsin32 \
	s-tsin32.idx \
	pho.tab2 \
	s-pho.tab2 \
	pho-huge.tab2 \
	s-pho-huge.tab2 \
	s2t.dat \
	t2s.dat \
	pin-juyin.xlt \
	tsin-1.src \
	gtab.list

DATAKEEP = symbol-table phrase.table phrase-ctrl.table

PHO_A2D    = $(SRC_DIR)/hime-phoa2d
TS_A2D     = $(SRC_DIR)/hime-tsa2d32
CIN2GTAB   = $(SRC_DIR)/hime-cin2gtab
KBMCV      = $(SRC_DIR)/kbmcv
PIN_JUYIN  = $(SRC_DIR)/pin-juyin

ifeq ($(USE_TSIN),Y)
REMOVE_TSIN = keep_tsin
endif

ifeq ($(USE_TSIN),N)
REMOVE_TSIN = remove_tsin
endif


.PHONY: all
all: $(DATA)

$(PHO_A2D):
	$(MAKE) -C $(SRC_DIR) hime-phoa2d

$(TS_A2D):
	$(MAKE) -C $(SRC_DIR) hime-tsa2d32

$(CIN2GTAB):
	$(MAKE) -C $(SRC_DIR) hime-cin2gtab

$(KBMCV):
	$(MAKE) -C $(SRC_DIR) kbmcv

$(PIN_JUYIN):
	$(MAKE) -C $(SRC_DIR) pin-juyin

tsin32: tsin.src  $(TS_A2D)
	export LD_LIBRARY_PATH=$(IM_CLIENT_DIR); $(TS_A2D) $<
tsin32.idx: tsin32


s-tsin32: s-tsin.src  $(TS_A2D)
	export LD_LIBRARY_PATH=$(IM_CLIENT_DIR); $(TS_A2D) $< $@
s-tsin32.idx: s-tsin32


pho.tab2: pho.tab2.src $(PHO_A2D)
	export LD_LIBRARY_PATH=$(IM_CLIENT_DIR); $(PHO_A2D) $<

s-pho.tab2: s-pho.tab2.src $(PHO_A2D)
	export LD_LIBRARY_PATH=$(IM_CLIENT_DIR); $(PHO_A2D) $<

pho-huge.tab2: pho-huge.tab2.src $(PHO_A2D)
	export LD_LIBRARY_PATH=$(IM_CLIENT_DIR); $(PHO_A2D) $<

s-pho-huge.tab2: s-pho-huge.tab2.src $(PHO_A2D)
	export LD_LIBRARY_PATH=$(IM_CLIENT_DIR); $(PHO_A2D) $<

t2s.dat: t2s-file t2s-file.table
	./t2s-file
s2t.dat: t2s-file

pin-juyin.xlt: $(PIN_JUYIN) pin-juyin.src
	$(PIN_JUYIN)

extr1: extr1.c $(SRC_DIR)/util.o $(SRC_DIR)/locale.o
	@echo "building $@ ..."
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

t2s-file: t2s-file.c $(SRC_DIR)/util.o $(SRC_DIR)/locale.o
	@echo "building $@ ..."
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

tsin-1.src: extr1
	./extr1 > $@

.cin.gtab: $(CIN2GTAB)
	$(CIN2GTAB) $<

.kbmsrc.kbm: $(KBMCV)
	$(KBMCV) $<

keep_tsin: gtab.list.in
	cp gtab.list.in gtab.list

remove_tsin: gtab.list.in
	grep -v TSIN gtab.list.in > gtab.list

gtab.list: $(REMOVE_TSIN)
	touch gtab.list

.PHONY: install
install:
	install -d "$(HIME_TABLE_DIR)"
	install -m 644 $(DATA) $(DATAKEEP) "$(HIME_TABLE_DIR)"

.PHONY: uninstall
uninstall:
	cd $(HIME_TABLE_DIR) && rm -f $(DATA) $(DATAKEEP)
	rmdir --ignore-fail-on-non-empty $(HIME_TABLE_DIR)

.PHONY: clean
clean:
	@echo "clean up"
	@rm -f *~ tags core.* $(DATA) *.gtab extr1 t2s-file tsin-1.src tsin-gtabidx* tsin-ch-idx gtab.list
